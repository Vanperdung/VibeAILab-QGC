#pragma once

#include "VehicleConfigUITestBase.h"

class MockLink;
class QQuickItem;

/// UI test of the Sensors page: boots the full QML UI with a generic mock vehicle, opens the page and runs the
/// calibrations with the real buttons. The test plays the FC: it sends the status texts of a run.
///
/// One test function only: the Actuators page gives the QML engine an image provider once per process, so a second
/// UI boot in the same process would use the provider the first engine deleted.
class VibeAILabSensorsUITest : public VehicleConfigUITestBase
{
    Q_OBJECT

private slots:
    void _calibrationsFromPage();

private:
    void _openSensorsPage();
    void _cancelGyroCalibration();
    void _runAccelCalibration(MockLink* mockLink);
    QQuickItem* _sideTile(const char* tileObjectName);
    void _verifySideTileState(const char* tileObjectName, int expectedCalState, const char* context);
    void _verifyAllSideTilesState(int expectedCalState, const char* context);
};
