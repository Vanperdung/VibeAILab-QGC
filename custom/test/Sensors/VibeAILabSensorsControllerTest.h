#pragma once

#include "BaseClasses/VehicleTest.h"

/// Tests the Sensors page controller against a mock vehicle: the commands it sends, and how the vehicle's command
/// replies and status texts reach the calibrations.
class VibeAILabSensorsControllerTest : public VehicleTest
{
    Q_OBJECT

private slots:
    void _startCommand_data();
    void _startCommand();
    void _accelRunFollowsStatusTexts();
    void _cancelEndsRunOnReply();
    void _startWithoutReply();
    void _oneCommandAtATime();
    void _failedStartAgainKeepsRun();
};
