#include "VibeAILabGyroCalibrationTest.h"

#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "VibeAILabGyroCalibration.h"

UT_REGISTER_TEST_LIGHTWEIGHT(VibeAILabGyroCalibrationTest, TestLabel::Unit)

using CalReply = VibeAILabCalibration::CommandReply;
using CalResult = VibeAILabCalibration::Result;
using CalState = VibeAILabCalibration::State;

namespace {

void startGyroRun(VibeAILabGyroCalibration& calibration)
{
    QVERIFY(calibration.requestStart());
    calibration.handleStartReply(CalReply::Accepted);
    QCOMPARE(calibration.state(), CalState::Running);
}

}  // namespace

void VibeAILabGyroCalibrationTest::_startReply_data()
{
    QTest::addColumn<CalReply>("reply");
    QTest::addColumn<CalState>("expectedState");
    QTest::addColumn<CalResult>("expectedResult");

    QTest::newRow("accepted") << CalReply::Accepted << CalState::Running << CalResult::None;
    QTest::newRow("vehicle armed") << CalReply::VehicleArmed << CalState::Idle << CalResult::VehicleArmed;
    QTest::newRow("unsupported") << CalReply::Unsupported << CalState::Idle << CalResult::Unsupported;
    QTest::newRow("rejected") << CalReply::Rejected << CalState::Idle << CalResult::Rejected;
    QTest::newRow("no response") << CalReply::NoResponse << CalState::Idle << CalResult::NoResponse;
    QTest::newRow("not sent") << CalReply::NotSent << CalState::Idle << CalResult::NotSent;
}

void VibeAILabGyroCalibrationTest::_startReply()
{
    QFETCH(CalReply, reply);
    QFETCH(CalState, expectedState);
    QFETCH(CalResult, expectedResult);

    VibeAILabGyroCalibration calibration;
    QVERIFY(calibration.requestStart());
    QCOMPARE(calibration.state(), CalState::Starting);

    calibration.handleStartReply(reply);

    QCOMPARE(calibration.state(), expectedState);
    QCOMPARE(calibration.result(), expectedResult);
    QCOMPARE(calibration.active(), expectedState != CalState::Idle);
    QCOMPARE(calibration.runGoingOn(), expectedState == CalState::Running);
    QVERIFY(!calibration.statusText().isEmpty());
}

void VibeAILabGyroCalibrationTest::_startedTextStartsRunBeforeReply()
{
    VibeAILabGyroCalibration calibration;
    QVERIFY(calibration.requestStart());

    calibration.handleStatusText(QStringLiteral("Gyro calibration started"));
    QCOMPARE(calibration.state(), CalState::Running);

    // A late or lost reply to the start command does not end the run
    calibration.handleStartReply(CalReply::NoResponse);
    QCOMPARE(calibration.state(), CalState::Running);
}

void VibeAILabGyroCalibrationTest::_startedTextAfterRefusedStart_data()
{
    QTest::addColumn<CalReply>("reply");
    QTest::addColumn<bool>("resultCleared");
    QTest::addColumn<CalState>("expectedState");

    // No reply in time does not mean the FC did not get the command
    QTest::newRow("no response") << CalReply::NoResponse << false << CalState::Running;
    QTest::newRow("no response, result cleared") << CalReply::NoResponse << true << CalState::Idle;
    QTest::newRow("vehicle armed") << CalReply::VehicleArmed << false << CalState::Idle;
    QTest::newRow("not sent") << CalReply::NotSent << false << CalState::Idle;
}

void VibeAILabGyroCalibrationTest::_startedTextAfterRefusedStart()
{
    QFETCH(CalReply, reply);
    QFETCH(bool, resultCleared);
    QFETCH(CalState, expectedState);

    VibeAILabGyroCalibration calibration;
    QVERIFY(calibration.requestStart());
    calibration.handleStartReply(reply);
    QCOMPARE(calibration.state(), CalState::Idle);
    if (resultCleared) {
        calibration.clearResult();
    }

    calibration.handleStatusText(QStringLiteral("Gyro calibration started"));

    QCOMPARE(calibration.state(), expectedState);
}

void VibeAILabGyroCalibrationTest::_endText_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<CalState>("expectedState");
    QTest::addColumn<CalResult>("expectedResult");

    QTest::newRow("done") << QStringLiteral("Gyro calibration done") << CalState::Idle << CalResult::Done;
    QTest::newRow("cancelled") << QStringLiteral("Gyro calibration cancelled") << CalState::Idle
                               << CalResult::Cancelled;
    QTest::newRow("arm reject text") << QStringLiteral("Arm rejected: gyro calibrating") << CalState::Running
                                     << CalResult::None;
    QTest::newRow("accel text") << QStringLiteral("[cal] calibration done") << CalState::Running << CalResult::None;
}

void VibeAILabGyroCalibrationTest::_endText()
{
    QFETCH(QString, text);
    QFETCH(CalState, expectedState);
    QFETCH(CalResult, expectedResult);

    VibeAILabGyroCalibration calibration;
    startGyroRun(calibration);

    calibration.handleStatusText(text);

    QCOMPARE(calibration.state(), expectedState);
    QCOMPARE(calibration.result(), expectedResult);
}

void VibeAILabGyroCalibrationTest::_cancelReply_data()
{
    QTest::addColumn<CalReply>("reply");
    QTest::addColumn<CalState>("expectedState");
    QTest::addColumn<CalResult>("expectedResult");

    QTest::newRow("accepted") << CalReply::Accepted << CalState::Idle << CalResult::Cancelled;
    QTest::newRow("rejected") << CalReply::Rejected << CalState::Running << CalResult::None;
    QTest::newRow("no response") << CalReply::NoResponse << CalState::Running << CalResult::None;
    QTest::newRow("not sent") << CalReply::NotSent << CalState::Running << CalResult::None;
}

void VibeAILabGyroCalibrationTest::_cancelReply()
{
    QFETCH(CalReply, reply);
    QFETCH(CalState, expectedState);
    QFETCH(CalResult, expectedResult);

    VibeAILabGyroCalibration calibration;
    startGyroRun(calibration);
    QVERIFY(calibration.requestCancel());
    QCOMPARE(calibration.state(), CalState::Cancelling);

    calibration.handleCancelReply(reply);

    QCOMPARE(calibration.state(), expectedState);
    QCOMPARE(calibration.result(), expectedResult);
}

void VibeAILabGyroCalibrationTest::_requestsRefusedInWrongState()
{
    VibeAILabGyroCalibration calibration;
    QVERIFY(!calibration.requestCancel());

    QVERIFY(calibration.requestStart());
    QVERIFY(!calibration.requestStart());
    QVERIFY(!calibration.requestCancel());

    calibration.handleStartReply(CalReply::Accepted);
    QVERIFY(calibration.requestCancel());
    QVERIFY(!calibration.requestStart());
    QVERIFY(!calibration.requestCancel());
}

void VibeAILabGyroCalibrationTest::_replyAfterRunEnded()
{
    VibeAILabGyroCalibration calibration;
    startGyroRun(calibration);
    QVERIFY(calibration.requestCancel());

    // The run ended by itself before the FC got the cancel command
    calibration.handleStatusText(QStringLiteral("Gyro calibration done"));
    QCOMPARE(calibration.result(), CalResult::Done);

    calibration.handleCancelReply(CalReply::Accepted);
    QCOMPARE(calibration.state(), CalState::Idle);
    QCOMPARE(calibration.result(), CalResult::Done);
}

void VibeAILabGyroCalibrationTest::_replyToOtherRequestIgnored()
{
    VibeAILabGyroCalibration calibration;

    // A cancel reply does not start a run
    QVERIFY(calibration.requestStart());
    calibration.handleCancelReply(CalReply::Accepted);
    QCOMPARE(calibration.state(), CalState::Starting);

    // A start reply does not end a cancel
    calibration.handleStartReply(CalReply::Accepted);
    QVERIFY(calibration.requestCancel());
    calibration.handleStartReply(CalReply::Rejected);
    QCOMPARE(calibration.state(), CalState::Cancelling);
    calibration.handleStartReply(CalReply::Accepted);
    QCOMPARE(calibration.state(), CalState::Cancelling);
}

void VibeAILabGyroCalibrationTest::_textsIgnoredWhenIdle()
{
    VibeAILabGyroCalibration calibration;
    QSignalSpy stateSpy(&calibration, &VibeAILabCalibration::stateChanged);
    QSignalSpy resultSpy(&calibration, &VibeAILabCalibration::resultChanged);

    // The texts of the FC's own run at boot
    calibration.handleStatusText(QStringLiteral("Gyro calibration started"));
    calibration.handleStatusText(QStringLiteral("Gyro calibration done"));

    QCOMPARE(stateSpy.count(), 0);
    QCOMPARE(resultSpy.count(), 0);
}

void VibeAILabGyroCalibrationTest::_clearResult()
{
    VibeAILabGyroCalibration calibration;
    startGyroRun(calibration);

    calibration.clearResult();
    QCOMPARE(calibration.state(), CalState::Running);

    calibration.handleStatusText(QStringLiteral("Gyro calibration done"));
    calibration.clearResult();
    QCOMPARE(calibration.result(), CalResult::None);
    QVERIFY(calibration.statusText().isEmpty());
}
