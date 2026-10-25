#pragma once

#include "FirmwarePlugin.h"

/// Firmware plugin for VibeAI Lab flight controller.
class VibeAILabFirmwarePlugin : public FirmwarePlugin
{
    Q_OBJECT

public:
    explicit VibeAILabFirmwarePlugin(QObject* parent = nullptr);

    AutoPilotPlugin* autopilotPlugin(Vehicle* vehicle) const final;
};
