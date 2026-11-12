#include "VibeAILabGyroCalibration.h"

#include <QtCore/QLatin1StringView>

namespace {

constexpr QLatin1StringView gyroCalStartedText("Gyro calibration started");
constexpr QLatin1StringView gyroCalDoneText("Gyro calibration done");
constexpr QLatin1StringView gyroCalCancelledText("Gyro calibration cancelled");

}  // namespace

VibeAILabGyroCalibration::VibeAILabGyroCalibration(QObject* parent) : VibeAILabCalibration(parent) {}

void VibeAILabGyroCalibration::handleStatusText(const QString& text)
{
    if (text == gyroCalStartedText) {
        // Covers a lost or late reply to the start command. A start that joins a run going on gets no such text.
        _handleStartedText();
        return;
    }

    if (!runGoingOn()) {
        return;
    }

    if (text == gyroCalDoneText) {
        _finish(Result::Done);
    } else if (text == gyroCalCancelledText) {
        _finish(Result::Cancelled);
    }
}

QString VibeAILabGyroCalibration::_runningHelpText() const
{
    return tr("Calibrating. Keep the vehicle still.");
}
