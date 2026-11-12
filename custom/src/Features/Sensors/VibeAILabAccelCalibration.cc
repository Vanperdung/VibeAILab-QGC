#include "VibeAILabAccelCalibration.h"

#include <QtCore/QLatin1StringView>
#include <QtCore/QStringView>
#include <algorithm>
#include <optional>

namespace {

constexpr QLatin1StringView accelCalTextPrefix("[cal] ");
constexpr QLatin1StringView accelCalStartedText("calibration started");
constexpr QLatin1StringView accelCalNewSidePrefix("new side: ");
constexpr QLatin1StringView accelCalSideLostText("side lost, hold still ...");
constexpr QLatin1StringView accelCalSideDonePrefix("side done: ");
constexpr QLatin1StringView accelCalDoneText("calibration done");
constexpr QLatin1StringView accelCalFailedText("calibration failed");
constexpr QLatin1StringView accelCalCancelledText("calibration cancelled");

struct AccelCalPose
{
    QLatin1StringView text;
    VibeAILabAccelCalibration::Side side;
};

constexpr AccelCalPose accelCalPoses[] = {
    {.text = QLatin1StringView("right side up"), .side = VibeAILabAccelCalibration::Side::RightSideUp},
    {.text = QLatin1StringView("upside down"), .side = VibeAILabAccelCalibration::Side::UpsideDown},
    {.text = QLatin1StringView("left side down"), .side = VibeAILabAccelCalibration::Side::LeftSideDown},
    {.text = QLatin1StringView("right side down"), .side = VibeAILabAccelCalibration::Side::RightSideDown},
    {.text = QLatin1StringView("nose down"), .side = VibeAILabAccelCalibration::Side::NoseDown},
    {.text = QLatin1StringView("tail down"), .side = VibeAILabAccelCalibration::Side::TailDown},
};

/// The pose ends at the first comma. Poses have spaces, so the text is not split on them.
std::optional<VibeAILabAccelCalibration::Side> accelCalSideFromPose(QStringView textFromPose)
{
    const qsizetype commaIndex = textFromPose.indexOf(u',');
    const QStringView poseText = (commaIndex < 0) ? textFromPose : textFromPose.first(commaIndex);

    for (const AccelCalPose& pose : accelCalPoses) {
        if (poseText == pose.text) {
            return pose.side;
        }
    }

    return std::nullopt;
}

}  // namespace

VibeAILabAccelCalibration::VibeAILabAccelCalibration(QObject* parent) : VibeAILabCalibration(parent)
{
    _runSideStates.fill(SideState::NotDone);

    // The shown side states also depend on the run state and on how the run ended
    (void) connect(this, &VibeAILabCalibration::stateChanged, this, &VibeAILabAccelCalibration::sideStatesChanged);
    (void) connect(this, &VibeAILabCalibration::resultChanged, this, &VibeAILabAccelCalibration::sideStatesChanged);
}

VibeAILabAccelCalibration::SideState VibeAILabAccelCalibration::sideState(Side side) const
{
    if (runGoingOn()) {
        return _runSideStates[static_cast<size_t>(side)];
    }

    // A finished run has all sides done, even when a "side done" text was lost
    return (result() == Result::Done) ? SideState::Done : SideState::Idle;
}

QList<int> VibeAILabAccelCalibration::sideStates() const
{
    QList<int> states;
    states.reserve(SideCount);
    for (int side = 0; side < SideCount; side++) {
        states.append(static_cast<int>(sideState(static_cast<Side>(side))));
    }

    return states;
}

void VibeAILabAccelCalibration::handleStatusText(const QString& text)
{
    if (!text.startsWith(accelCalTextPrefix)) {
        return;
    }
    const QStringView event = QStringView(text).sliced(accelCalTextPrefix.size());

    if (event == accelCalStartedText) {
        // The FC sends it again when it starts the run again
        _handleStartedText();
        return;
    }

    if (!runGoingOn()) {
        return;
    }

    if (event.startsWith(accelCalNewSidePrefix)) {
        if (const auto side = accelCalSideFromPose(event.sliced(accelCalNewSidePrefix.size()))) {
            _collectSide(*side);
        }
    } else if (event.startsWith(accelCalSideDonePrefix)) {
        if (const auto side = accelCalSideFromPose(event.sliced(accelCalSideDonePrefix.size()))) {
            _completeSide(*side);
        }
    } else if (event == accelCalSideLostText) {
        _loseSideInProgress();
    } else if (event == accelCalDoneText) {
        _finish(Result::Done);
    } else if (event == accelCalFailedText) {
        _finish(Result::Failed);
    } else if (event == accelCalCancelledText) {
        _finish(Result::Cancelled);
    }
}

void VibeAILabAccelCalibration::_resetRun()
{
    std::array<SideState, SideCount> runSideStates{};
    runSideStates.fill(SideState::NotDone);
    _setRunSideStates(runSideStates);
}

QString VibeAILabAccelCalibration::_runningHelpText() const
{
    if (std::ranges::find(_runSideStates, SideState::InProgress) != _runSideStates.end()) {
        return tr("Hold the vehicle still");
    }

    return tr("Place the vehicle on one of the Incomplete sides and hold it still");
}

void VibeAILabAccelCalibration::_collectSide(Side side)
{
    // Only one side is in progress at a time
    std::array<SideState, SideCount> runSideStates = _runSideStates;
    std::ranges::replace(runSideStates, SideState::InProgress, SideState::NotDone);
    runSideStates[static_cast<size_t>(side)] = SideState::InProgress;
    _setRunSideStates(runSideStates);
}

void VibeAILabAccelCalibration::_loseSideInProgress()
{
    std::array<SideState, SideCount> runSideStates = _runSideStates;
    std::ranges::replace(runSideStates, SideState::InProgress, SideState::NotDone);
    _setRunSideStates(runSideStates);
}

void VibeAILabAccelCalibration::_completeSide(Side side)
{
    std::array<SideState, SideCount> runSideStates = _runSideStates;
    runSideStates[static_cast<size_t>(side)] = SideState::Done;
    _setRunSideStates(runSideStates);
}

void VibeAILabAccelCalibration::_setRunSideStates(const std::array<SideState, SideCount>& runSideStates)
{
    if (_runSideStates == runSideStates) {
        return;
    }

    _runSideStates = runSideStates;
    emit sideStatesChanged();
    emit statusTextChanged();
}
