#include "VibeAILabSensorsUITest.h"

#include <QtCore/QRegularExpression>
#include <QtQuick/QQuickItem>
#include <QtTest/QTest>

#include "MockLink.h"
#include "Vehicle.h"

UT_REGISTER_TEST(VibeAILabSensorsUITest, TestLabel::Integration)

namespace {

// VehicleRotationCal.CalState values
constexpr int sensorsTileIdle = 0;
constexpr int sensorsTileIncomplete = 1;
constexpr int sensorsTileInProgress = 2;
constexpr int sensorsTileCompleted = 3;

constexpr const char* sensorsSideTiles[] = {
    "vibeailabSensors_rightSideUp", "vibeailabSensors_upsideDown",   "vibeailabSensors_noseDown",
    "vibeailabSensors_tailDown",    "vibeailabSensors_leftSideDown", "vibeailabSensors_rightSideDown",
};

constexpr int sensorsUiTimeoutMs = 5000;

}  // namespace

void VibeAILabSensorsUITest::_openSensorsPage()
{
    navigateToConfigureView();
    if (QTest::currentTestFailed()) {
        return;
    }

    // A click on Sensors selects its first section, Gyroscope
    clickSidebarButton(QStringLiteral("vehicleConfig_comp_Sensors"));
    if (QTest::currentTestFailed()) {
        return;
    }

    QVERIFY2(findVisibleItem(_rootItem, QStringLiteral("vibeailabSensors_start"), sensorsUiTimeoutMs),
             "Start button not found after opening the Sensors page");
}

QQuickItem* VibeAILabSensorsUITest::_sideTile(const char* tileObjectName)
{
    return findVisibleItem(_rootItem, QLatin1String(tileObjectName), sensorsUiTimeoutMs);
}

void VibeAILabSensorsUITest::_verifySideTileState(const char* tileObjectName, int expectedCalState, const char* context)
{
    QQuickItem* const tile = _sideTile(tileObjectName);
    QVERIFY2(tile, qPrintable(QStringLiteral("Side tile not visible (%1): %2")
                                  .arg(QLatin1String(context), QLatin1String(tileObjectName))));

    (void) QTest::qWaitFor([&] { return tile->property("calState").toInt() == expectedCalState; }, sensorsUiTimeoutMs);
    QVERIFY2(tile->property("calState").toInt() == expectedCalState,
             qPrintable(QStringLiteral("Side tile in state %1, expected %2 (%3): %4")
                            .arg(tile->property("calState").toInt())
                            .arg(expectedCalState)
                            .arg(QLatin1String(context), QLatin1String(tileObjectName))));
}

void VibeAILabSensorsUITest::_verifyAllSideTilesState(int expectedCalState, const char* context)
{
    for (const char* tileObjectName : sensorsSideTiles) {
        _verifySideTileState(tileObjectName, expectedCalState, context);
        if (QTest::currentTestFailed()) {
            return;
        }
    }
}

void VibeAILabSensorsUITest::_calibrationsFromPage()
{
    // The generic mock vehicle serves no general metadata
    ignoreLogMessage("ComponentInformation.RequestMetaDataTypeStateMachine", QtWarningMsg,
                     QRegularExpression(QStringLiteral("failed to load metadata")));

    runWithMockLink([] { return MockLink::startGenericMockLink(); },
                    [&](QPointer<MockLink> mockLink, Vehicle*) {
                        _openSensorsPage();
                        if (QTest::currentTestFailed()) {
                            return;
                        }

                        _cancelGyroCalibration();
                        if (QTest::currentTestFailed()) {
                            return;
                        }

                        clickSidebarButton(QStringLiteral("vehicleConfig_section_Accelerometer"));
                        if (QTest::currentTestFailed()) {
                            return;
                        }

                        _runAccelCalibration(mockLink);
                    });
}

void VibeAILabSensorsUITest::_cancelGyroCalibration()
{
    // The gyro shows one pose only
    _verifySideTileState("vibeailabSensors_rightSideUp", sensorsTileIdle, "before gyro start");
    if (QTest::currentTestFailed()) {
        return;
    }
    QVERIFY2(!findVisibleItem(_rootItem, QStringLiteral("vibeailabSensors_noseDown"), 0),
             "Nose down tile shown for the gyro");

    // The mock vehicle accepts the start command
    QVERIFY2(clickButton(QStringLiteral("vibeailabSensors_start")), "Failed to click Start");
    _verifySideTileState("vibeailabSensors_rightSideUp", sensorsTileInProgress, "after gyro start");
    if (QTest::currentTestFailed()) {
        return;
    }
    QVERIFY(verifyVisibility(QStringLiteral("vibeailabSensors_restart"), false, QStringLiteral("gyro run")));

    // The mock vehicle accepts the cancel command and sends no gyro text
    QVERIFY(verifyEnabled(QStringLiteral("vibeailabSensors_cancel"), true, QStringLiteral("start reply arrived")));
    QVERIFY2(clickButton(QStringLiteral("vibeailabSensors_cancel")), "Failed to click Cancel");
    QVERIFY(verifyVisibility(QStringLiteral("vibeailabSensors_start"), true, QStringLiteral("after cancel")));
    QVERIFY(verifyText(QStringLiteral("vibeailabSensors_status"), QStringLiteral("Calibration cancelled"),
                       QStringLiteral("after cancel")));
    _verifySideTileState("vibeailabSensors_rightSideUp", sensorsTileIdle, "after cancel");
}

void VibeAILabSensorsUITest::_runAccelCalibration(MockLink* mockLink)
{
    _verifyAllSideTilesState(sensorsTileIdle, "before accel start");
    if (QTest::currentTestFailed()) {
        return;
    }

    QVERIFY2(clickButton(QStringLiteral("vibeailabSensors_start")), "Failed to click Start");
    _verifyAllSideTilesState(sensorsTileIncomplete, "after accel start");
    if (QTest::currentTestFailed()) {
        return;
    }
    QVERIFY(verifyVisibility(QStringLiteral("vibeailabSensors_start"), false, QStringLiteral("accel run")));

    mockLink->sendStatusTextMessage(MAV_SEVERITY_INFO, QStringLiteral("[cal] calibration started"));
    mockLink->sendStatusTextMessage(MAV_SEVERITY_INFO, QStringLiteral("[cal] new side: nose down, hold still"));
    _verifySideTileState("vibeailabSensors_noseDown", sensorsTileInProgress, "side in progress");
    if (QTest::currentTestFailed()) {
        return;
    }

    mockLink->sendStatusTextMessage(MAV_SEVERITY_INFO, QStringLiteral("[cal] side done: nose down, rotate"));
    _verifySideTileState("vibeailabSensors_noseDown", sensorsTileCompleted, "side done");
    if (QTest::currentTestFailed()) {
        return;
    }
    _verifySideTileState("vibeailabSensors_tailDown", sensorsTileIncomplete, "other side");
    if (QTest::currentTestFailed()) {
        return;
    }

    QQuickItem* const progressBar = findVisibleItem(_rootItem, QStringLiteral("vibeailabSensors_progressBar"));
    QVERIFY2(progressBar, "Progress bar not visible during the run");
    QVERIFY2(
        qFuzzyCompare(progressBar->property("value").toDouble(), 1.0 / 6.0),
        qPrintable(
            QStringLiteral("Progress bar at %1 with one side done").arg(progressBar->property("value").toDouble())));

    // Start again: all sides are not done again
    QVERIFY(verifyEnabled(QStringLiteral("vibeailabSensors_restart"), true, QStringLiteral("start reply arrived")));
    QVERIFY2(clickButton(QStringLiteral("vibeailabSensors_restart")), "Failed to click Restart");
    _verifyAllSideTilesState(sensorsTileIncomplete, "after restart");
    if (QTest::currentTestFailed()) {
        return;
    }

    mockLink->sendStatusTextMessage(MAV_SEVERITY_INFO, QStringLiteral("[cal] calibration done"));
    _verifyAllSideTilesState(sensorsTileCompleted, "after calibration done");
    if (QTest::currentTestFailed()) {
        return;
    }
    QVERIFY(verifyVisibility(QStringLiteral("vibeailabSensors_start"), true, QStringLiteral("accel run ended")));
    QVERIFY(verifyText(QStringLiteral("vibeailabSensors_status"), QStringLiteral("Calibration complete"),
                       QStringLiteral("accel run ended")));

    // The result is not shown again after a visit to another section
    clickSidebarButton(QStringLiteral("vehicleConfig_section_Gyroscope"));
    if (QTest::currentTestFailed()) {
        return;
    }
    clickSidebarButton(QStringLiteral("vehicleConfig_section_Accelerometer"));
    if (QTest::currentTestFailed()) {
        return;
    }
    _verifyAllSideTilesState(sensorsTileIdle, "after section switch");
}
