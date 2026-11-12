#include "VibeAILabAccelCalibrationTest.h"

#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "VibeAILabAccelCalibration.h"

UT_REGISTER_TEST_LIGHTWEIGHT(VibeAILabAccelCalibrationTest, TestLabel::Unit)

using CalReply = VibeAILabCalibration::CommandReply;
using CalResult = VibeAILabCalibration::Result;
using AccelSide = VibeAILabAccelCalibration::Side;
using AccelSideState = VibeAILabAccelCalibration::SideState;
using CalState = VibeAILabCalibration::State;

namespace {

constexpr AccelSide accelAllSides[] = {AccelSide::RightSideUp,   AccelSide::UpsideDown, AccelSide::LeftSideDown,
                                       AccelSide::RightSideDown, AccelSide::NoseDown,   AccelSide::TailDown};

void startAccelRun(VibeAILabAccelCalibration& calibration)
{
    QVERIFY(calibration.requestStart());
    calibration.handleStartReply(CalReply::Accepted);
    calibration.handleStatusText(QStringLiteral("[cal] calibration started"));
    QCOMPARE(calibration.state(), CalState::Running);
}

int accelSideCount(const VibeAILabAccelCalibration& calibration, AccelSideState sideState)
{
    int count = 0;
    for (const AccelSide side : accelAllSides) {
        if (calibration.sideState(side) == sideState) {
            count++;
        }
    }

    return count;
}

}  // namespace

void VibeAILabAccelCalibrationTest::_sidesFollowTexts()
{
    VibeAILabAccelCalibration calibration;
    QCOMPARE(accelSideCount(calibration, AccelSideState::Idle), 6);

    startAccelRun(calibration);
    QCOMPARE(accelSideCount(calibration, AccelSideState::NotDone), 6);

    calibration.handleStatusText(QStringLiteral("[cal] new side: nose down, hold still"));
    QCOMPARE(calibration.sideState(AccelSide::NoseDown), AccelSideState::InProgress);
    const QString holdStillText = calibration.statusText();

    // The FC moved on to another side: only one side is in progress
    calibration.handleStatusText(QStringLiteral("[cal] new side: tail down, hold still"));
    QCOMPARE(calibration.sideState(AccelSide::NoseDown), AccelSideState::NotDone);
    QCOMPARE(calibration.sideState(AccelSide::TailDown), AccelSideState::InProgress);

    calibration.handleStatusText(QStringLiteral("[cal] side lost, hold still ..."));
    QCOMPARE(accelSideCount(calibration, AccelSideState::NotDone), 6);
    QVERIFY(calibration.statusText() != holdStillText);

    calibration.handleStatusText(QStringLiteral("[cal] new side: tail down, hold still"));
    calibration.handleStatusText(QStringLiteral("[cal] side done: tail down, rotate"));
    QCOMPARE(calibration.sideState(AccelSide::TailDown), AccelSideState::Done);
    QCOMPARE(accelSideCount(calibration, AccelSideState::NotDone), 5);

    // A lost side does not undo a done side
    calibration.handleStatusText(QStringLiteral("[cal] new side: upside down, hold still"));
    calibration.handleStatusText(QStringLiteral("[cal] side lost, hold still ..."));
    QCOMPARE(calibration.sideState(AccelSide::TailDown), AccelSideState::Done);
    QCOMPARE(calibration.sideState(AccelSide::UpsideDown), AccelSideState::NotDone);

    // The end shows all sides done, also the ones whose "side done" text was lost
    calibration.handleStatusText(QStringLiteral("[cal] calibration done"));
    QCOMPARE(calibration.state(), CalState::Idle);
    QCOMPARE(calibration.result(), CalResult::Done);
    QCOMPARE(accelSideCount(calibration, AccelSideState::Done), 6);
}

void VibeAILabAccelCalibrationTest::_poseText_data()
{
    QTest::addColumn<QString>("poseText");
    QTest::addColumn<AccelSide>("expectedSide");

    QTest::newRow("right side up") << QStringLiteral("right side up") << AccelSide::RightSideUp;
    QTest::newRow("upside down") << QStringLiteral("upside down") << AccelSide::UpsideDown;
    QTest::newRow("left side down") << QStringLiteral("left side down") << AccelSide::LeftSideDown;
    QTest::newRow("right side down") << QStringLiteral("right side down") << AccelSide::RightSideDown;
    QTest::newRow("nose down") << QStringLiteral("nose down") << AccelSide::NoseDown;
    QTest::newRow("tail down") << QStringLiteral("tail down") << AccelSide::TailDown;
}

void VibeAILabAccelCalibrationTest::_poseText()
{
    QFETCH(QString, poseText);
    QFETCH(AccelSide, expectedSide);

    VibeAILabAccelCalibration calibration;
    startAccelRun(calibration);
    QSignalSpy sideStatesSpy(&calibration, &VibeAILabAccelCalibration::sideStatesChanged);

    calibration.handleStatusText(QStringLiteral("[cal] new side: %1, hold still").arg(poseText));
    QCOMPARE(calibration.sideState(expectedSide), AccelSideState::InProgress);
    QCOMPARE(accelSideCount(calibration, AccelSideState::NotDone), 5);

    calibration.handleStatusText(QStringLiteral("[cal] side done: %1, rotate").arg(poseText));
    QCOMPARE(calibration.sideState(expectedSide), AccelSideState::Done);
    QCOMPARE(accelSideCount(calibration, AccelSideState::NotDone), 5);

    QCOMPARE(sideStatesSpy.count(), 2);
    QCOMPARE(calibration.sideStates().at(static_cast<int>(expectedSide)), static_cast<int>(AccelSideState::Done));
}

void VibeAILabAccelCalibrationTest::_endText_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<CalResult>("expectedResult");
    QTest::addColumn<AccelSideState>("expectedSideState");

    QTest::newRow("done") << QStringLiteral("[cal] calibration done") << CalResult::Done << AccelSideState::Done;
    QTest::newRow("failed") << QStringLiteral("[cal] calibration failed") << CalResult::Failed << AccelSideState::Idle;
    QTest::newRow("cancelled") << QStringLiteral("[cal] calibration cancelled") << CalResult::Cancelled
                               << AccelSideState::Idle;
}

void VibeAILabAccelCalibrationTest::_endText()
{
    QFETCH(QString, text);
    QFETCH(CalResult, expectedResult);
    QFETCH(AccelSideState, expectedSideState);

    VibeAILabAccelCalibration calibration;
    startAccelRun(calibration);
    calibration.handleStatusText(QStringLiteral("[cal] side done: nose down, rotate"));

    calibration.handleStatusText(text);

    QCOMPARE(calibration.state(), CalState::Idle);
    QCOMPARE(calibration.result(), expectedResult);
    QCOMPARE(accelSideCount(calibration, expectedSideState), 6);
}

void VibeAILabAccelCalibrationTest::_ignoredText_data()
{
    QTest::addColumn<QString>("text");

    QTest::newRow("no prefix") << QStringLiteral("new side: nose down, hold still");
    QTest::newRow("unknown pose") << QStringLiteral("[cal] new side: on its head, hold still");
    QTest::newRow("pose split on spaces") << QStringLiteral("[cal] side done: nose, rotate");
    QTest::newRow("unknown text") << QStringLiteral("[cal] progress <50>");
    QTest::newRow("PX4 done text") << QStringLiteral("[cal] calibration done: accel");
    QTest::newRow("PX4 started text") << QStringLiteral("[cal] calibration started: 2 accel");
    QTest::newRow("gyro text") << QStringLiteral("Gyro calibration done");
}

void VibeAILabAccelCalibrationTest::_ignoredText()
{
    QFETCH(QString, text);

    VibeAILabAccelCalibration calibration;
    startAccelRun(calibration);
    calibration.handleStatusText(QStringLiteral("[cal] new side: tail down, hold still"));
    QSignalSpy sideStatesSpy(&calibration, &VibeAILabAccelCalibration::sideStatesChanged);

    calibration.handleStatusText(text);

    QCOMPARE(calibration.state(), CalState::Running);
    QCOMPARE(calibration.sideState(AccelSide::TailDown), AccelSideState::InProgress);
    QCOMPARE(sideStatesSpy.count(), 0);
}

void VibeAILabAccelCalibrationTest::_startedTextStartsRunBeforeReply()
{
    VibeAILabAccelCalibration calibration;
    QVERIFY(calibration.requestStart());

    calibration.handleStatusText(QStringLiteral("[cal] new side: nose down, hold still"));
    QCOMPARE(calibration.state(), CalState::Starting);

    calibration.handleStatusText(QStringLiteral("[cal] calibration started"));
    QCOMPARE(calibration.state(), CalState::Running);
    QCOMPARE(accelSideCount(calibration, AccelSideState::NotDone), 6);

    // The reply to the start command comes late: the run and its sides stay
    calibration.handleStatusText(QStringLiteral("[cal] side done: nose down, rotate"));
    calibration.handleStartReply(CalReply::Accepted);
    QCOMPARE(calibration.state(), CalState::Running);
    QCOMPARE(calibration.sideState(AccelSide::NoseDown), AccelSideState::Done);
}

void VibeAILabAccelCalibrationTest::_startedTextAfterNoResponse()
{
    VibeAILabAccelCalibration calibration;
    QVERIFY(calibration.requestStart());
    calibration.handleStartReply(CalReply::NoResponse);
    QCOMPARE(calibration.result(), CalResult::NoResponse);

    // The reply was lost, but the FC got the command and runs
    calibration.handleStatusText(QStringLiteral("[cal] calibration started"));
    QCOMPARE(calibration.state(), CalState::Running);
    QCOMPARE(accelSideCount(calibration, AccelSideState::NotDone), 6);

    calibration.handleStatusText(QStringLiteral("[cal] side done: nose down, rotate"));
    QCOMPARE(calibration.sideState(AccelSide::NoseDown), AccelSideState::Done);
}

void VibeAILabAccelCalibrationTest::_startAgainResetsSides()
{
    VibeAILabAccelCalibration calibration;
    startAccelRun(calibration);
    calibration.handleStatusText(QStringLiteral("[cal] side done: nose down, rotate"));

    // The user starts the run again
    QVERIFY(calibration.requestStart());
    QCOMPARE(calibration.state(), CalState::Restarting);
    calibration.handleStartReply(CalReply::Accepted);
    QCOMPARE(calibration.state(), CalState::Running);
    QCOMPARE(accelSideCount(calibration, AccelSideState::NotDone), 6);

    // The FC starts the run again without a click on this page
    calibration.handleStatusText(QStringLiteral("[cal] side done: tail down, rotate"));
    calibration.handleStatusText(QStringLiteral("[cal] calibration started"));
    QCOMPARE(calibration.state(), CalState::Running);
    QCOMPARE(accelSideCount(calibration, AccelSideState::NotDone), 6);
}

void VibeAILabAccelCalibrationTest::_failedStartAgainKeepsRun_data()
{
    QTest::addColumn<CalReply>("reply");

    QTest::newRow("rejected") << CalReply::Rejected;
    QTest::newRow("no response") << CalReply::NoResponse;
    QTest::newRow("not sent") << CalReply::NotSent;
}

void VibeAILabAccelCalibrationTest::_failedStartAgainKeepsRun()
{
    QFETCH(CalReply, reply);

    VibeAILabAccelCalibration calibration;
    startAccelRun(calibration);
    calibration.handleStatusText(QStringLiteral("[cal] side done: nose down, rotate"));

    // The FC run goes on while the start command is on its way
    QVERIFY(calibration.requestStart());
    QVERIFY(calibration.active());
    QVERIFY(calibration.runGoingOn());
    calibration.handleStatusText(QStringLiteral("[cal] side done: tail down, rotate"));
    QCOMPARE(calibration.sideState(AccelSide::TailDown), AccelSideState::Done);

    calibration.handleStartReply(reply);

    QCOMPARE(calibration.state(), CalState::Running);
    QCOMPARE(calibration.result(), CalResult::None);
    QCOMPARE(accelSideCount(calibration, AccelSideState::Done), 2);

    calibration.handleStatusText(QStringLiteral("[cal] calibration done"));
    QCOMPARE(calibration.result(), CalResult::Done);
}

void VibeAILabAccelCalibrationTest::_runEndsWhileStartAgainOnItsWay()
{
    VibeAILabAccelCalibration calibration;
    startAccelRun(calibration);
    QVERIFY(calibration.requestStart());

    calibration.handleStatusText(QStringLiteral("[cal] calibration done"));
    QCOMPARE(calibration.result(), CalResult::Done);

    // The FC then got the start command and started a new run
    calibration.handleStartReply(CalReply::Accepted);
    QCOMPARE(calibration.state(), CalState::Running);
    QCOMPARE(accelSideCount(calibration, AccelSideState::NotDone), 6);
}

void VibeAILabAccelCalibrationTest::_textsIgnoredWhenIdle()
{
    VibeAILabAccelCalibration calibration;
    QSignalSpy stateSpy(&calibration, &VibeAILabCalibration::stateChanged);
    QSignalSpy resultSpy(&calibration, &VibeAILabCalibration::resultChanged);

    calibration.handleStatusText(QStringLiteral("[cal] calibration started"));
    calibration.handleStatusText(QStringLiteral("[cal] side done: nose down, rotate"));
    calibration.handleStatusText(QStringLiteral("[cal] calibration done"));

    QCOMPARE(stateSpy.count(), 0);
    QCOMPARE(resultSpy.count(), 0);
    QCOMPARE(accelSideCount(calibration, AccelSideState::Idle), 6);
}
