#include "VibeAILabSensorsControllerTest.h"

#include <QtCore/QRegularExpression>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "ParameterManager.h"
#include "Vehicle.h"
#include "VibeAILabAccelCalibration.h"
#include "VibeAILabGyroCalibration.h"
#include "VibeAILabSensorsController.h"

UT_REGISTER_TEST(VibeAILabSensorsControllerTest, TestLabel::Integration, TestLabel::Vehicle)

using CalResult = VibeAILabCalibration::Result;
using CalState = VibeAILabCalibration::State;

void VibeAILabSensorsControllerTest::_startCommand_data()
{
    QTest::addColumn<bool>("gyro");
    QTest::addColumn<QString>("mockStartedText");

    // The mock vehicle answers a start with the PX4 text that names the sensor it got in the command
    QTest::newRow("gyro") << true << QStringLiteral("[cal] calibration started: 2 gyro");
    QTest::newRow("accel") << false << QStringLiteral("[cal] calibration started: 2 accel");
}

void VibeAILabSensorsControllerTest::_startCommand()
{
    QFETCH(bool, gyro);
    QFETCH(QString, mockStartedText);

    VibeAILabSensorsController controller;
    const VibeAILabCalibration* const calibration =
        gyro ? static_cast<VibeAILabCalibration*>(controller.gyro()) : controller.accel();
    QSignalSpy textSpy(vehicle(), &Vehicle::textMessageReceived);

    if (gyro) {
        controller.startGyroCalibration();
    } else {
        controller.startAccelCalibration();
    }
    QCOMPARE(calibration->state(), CalState::Starting);
    QVERIFY(controller.calibrationActive());
    QVERIFY(controller.commandPending());

    QTRY_COMPARE_WITH_TIMEOUT(calibration->state(), CalState::Running, TestTimeout::mediumMs());
    QVERIFY(!controller.commandPending());
    QCOMPARE(mockLink()->receivedMavCommandCount(MAV_CMD_PREFLIGHT_CALIBRATION), 1);

    const auto startedTextReceived = [&textSpy, &mockStartedText]() {
        for (const QList<QVariant>& arguments : textSpy) {
            if (arguments.at(3).toString() == mockStartedText) {
                return true;
            }
        }
        return false;
    };
    QTRY_VERIFY_WITH_TIMEOUT(startedTextReceived(), TestTimeout::mediumMs());
}

void VibeAILabSensorsControllerTest::_accelRunFollowsStatusTexts()
{
    VibeAILabSensorsController controller;
    const VibeAILabAccelCalibration* const accel = controller.accel();

    controller.startAccelCalibration();
    QTRY_COMPARE_WITH_TIMEOUT(accel->state(), CalState::Running, TestTimeout::mediumMs());

    // Only one calibration is active at a time
    expectLogMessage("VibeAILab.SensorsController", QtWarningMsg,
                     QRegularExpression(QStringLiteral("Another calibration is active")));
    controller.startGyroCalibration();
    verifyExpectedLogMessage();
    QCOMPARE(controller.gyro()->state(), CalState::Idle);

    mockLink()->sendStatusTextMessage(MAV_SEVERITY_INFO, QStringLiteral("[cal] side done: nose down, rotate"));
    QTRY_COMPARE_WITH_TIMEOUT(accel->sideState(VibeAILabAccelCalibration::Side::NoseDown),
                              VibeAILabAccelCalibration::SideState::Done, TestTimeout::mediumMs());

    QSignalSpy paramReadSpy(vehicle()->parameterManager(), &ParameterManager::_paramRequestReadSuccess);
    mockLink()->sendStatusTextMessage(MAV_SEVERITY_INFO, QStringLiteral("[cal] calibration done"));
    QTRY_COMPARE_WITH_TIMEOUT(accel->result(), CalResult::Done, TestTimeout::mediumMs());
    QVERIFY(!controller.calibrationActive());

    // The FC set new accel params: they are read again. The mock vehicle has one param named IMU_ACCEL_*.
    QTRY_COMPARE_WITH_TIMEOUT(paramReadSpy.count(), 1, TestTimeout::mediumMs());
    QVERIFY(paramReadSpy.first().at(1).toString().startsWith(QStringLiteral("IMU_ACCEL_")));
}

void VibeAILabSensorsControllerTest::_cancelEndsRunOnReply()
{
    VibeAILabSensorsController controller;
    const VibeAILabGyroCalibration* const gyro = controller.gyro();

    controller.startGyroCalibration();
    QTRY_COMPARE_WITH_TIMEOUT(gyro->state(), CalState::Running, TestTimeout::mediumMs());

    // The mock vehicle sends no gyro "cancelled" text: the accepted reply alone ends the run
    controller.cancelCalibration();
    QCOMPARE(gyro->state(), CalState::Cancelling);

    QTRY_COMPARE_WITH_TIMEOUT(gyro->state(), CalState::Idle, TestTimeout::mediumMs());
    QCOMPARE(gyro->result(), CalResult::Cancelled);
    QCOMPARE(mockLink()->receivedMavCommandCount(MAV_CMD_PREFLIGHT_CALIBRATION), 2);
}

void VibeAILabSensorsControllerTest::_startWithoutReply()
{
    VibeAILabSensorsController controller;
    mockLink()->setCommLost(true);
    expectLogMessage("Vehicle.MavCommandQueue", QtWarningMsg,
                     QRegularExpression(QStringLiteral("Giving up sending command")));

    controller.startGyroCalibration();

    QTRY_COMPARE_WITH_TIMEOUT(controller.gyro()->result(), CalResult::NoResponse, TestTimeout::mediumMs());
    verifyExpectedLogMessage();
    QCOMPARE(controller.gyro()->state(), CalState::Idle);

    // The start command was sent once, without retry
    QCOMPARE(mockLink()->receivedMavCommandCount(MAV_CMD_PREFLIGHT_CALIBRATION), 1);

    mockLink()->setCommLost(false);
}

void VibeAILabSensorsControllerTest::_oneCommandAtATime()
{
    VibeAILabSensorsController controller;
    const VibeAILabAccelCalibration* const accel = controller.accel();

    controller.startAccelCalibration();
    QTRY_COMPARE_WITH_TIMEOUT(accel->state(), CalState::Running, TestTimeout::mediumMs());
    mockLink()->sendStatusTextMessage(MAV_SEVERITY_INFO, QStringLiteral("[cal] side done: nose down, rotate"));
    QTRY_COMPARE_WITH_TIMEOUT(accel->sideState(VibeAILabAccelCalibration::Side::NoseDown),
                              VibeAILabAccelCalibration::SideState::Done, TestTimeout::mediumMs());

    // The cancel command gets no reply for a while
    mockLink()->setCommLost(true);
    controller.cancelCalibration();
    QCOMPARE(accel->state(), CalState::Cancelling);
    QVERIFY(controller.commandPending());

    // Start and cancel are the same MAVLink command: no second one is sent while the first waits for its reply
    expectLogMessage("VibeAILab.SensorsController", QtWarningMsg,
                     QRegularExpression(QStringLiteral("still waits for its reply")));
    controller.startAccelCalibration();
    verifyExpectedLogMessage();
    QCOMPARE(accel->state(), CalState::Cancelling);

    // No reply to the cancel: the FC goes on with its run, so the page stays with it and the user is told
    expectLogMessage("Vehicle.MavCommandQueue", QtWarningMsg,
                     QRegularExpression(QStringLiteral("Giving up sending command")));
    expectAppMessage(QRegularExpression(QStringLiteral("did not respond")));
    QTRY_VERIFY_WITH_TIMEOUT(!controller.commandPending(), TestTimeout::mediumMs());
    verifyExpectedLogMessage();
    verifyExpectedLogMessage();
    QCOMPARE(accel->state(), CalState::Running);
    QCOMPARE(accel->sideState(VibeAILabAccelCalibration::Side::NoseDown), VibeAILabAccelCalibration::SideState::Done);
    QCOMPARE(mockLink()->receivedMavCommandCount(MAV_CMD_PREFLIGHT_CALIBRATION), 2);

    mockLink()->setCommLost(false);
}

void VibeAILabSensorsControllerTest::_failedStartAgainKeepsRun()
{
    VibeAILabSensorsController controller;
    const VibeAILabAccelCalibration* const accel = controller.accel();

    controller.startAccelCalibration();
    QTRY_COMPARE_WITH_TIMEOUT(accel->state(), CalState::Running, TestTimeout::mediumMs());
    mockLink()->sendStatusTextMessage(MAV_SEVERITY_INFO, QStringLiteral("[cal] side done: nose down, rotate"));
    QTRY_COMPARE_WITH_TIMEOUT(accel->sideState(VibeAILabAccelCalibration::Side::NoseDown),
                              VibeAILabAccelCalibration::SideState::Done, TestTimeout::mediumMs());

    // The command to start again gets no reply: the FC goes on with its run, so the page stays with it and the
    // user is told
    mockLink()->setCommLost(true);
    expectLogMessage("Vehicle.MavCommandQueue", QtWarningMsg,
                     QRegularExpression(QStringLiteral("Giving up sending command")));
    expectAppMessage(QRegularExpression(QStringLiteral("did not respond")));
    controller.startAccelCalibration();
    QCOMPARE(accel->state(), CalState::Restarting);

    QTRY_VERIFY_WITH_TIMEOUT(!controller.commandPending(), TestTimeout::mediumMs());
    verifyExpectedLogMessage();
    verifyExpectedLogMessage();
    QCOMPARE(accel->state(), CalState::Running);
    QVERIFY(controller.calibrationActive());
    QCOMPARE(accel->sideState(VibeAILabAccelCalibration::Side::NoseDown), VibeAILabAccelCalibration::SideState::Done);

    mockLink()->setCommLost(false);
}
