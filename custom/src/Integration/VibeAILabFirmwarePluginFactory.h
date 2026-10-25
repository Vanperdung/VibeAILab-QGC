#pragma once

#include <QtCore/QLoggingCategory>

#include "FirmwarePluginFactory.h"

class VibeAILabFirmwarePlugin;

Q_DECLARE_LOGGING_CATEGORY(VibeAILabFirmwarePluginFactoryLog)

/// Firmware plugin factory for VibeAI Lab flight controller.
/// This factory is responsible for creating instances of the
/// VibeAILabFirmwarePlugin class.
class VibeAILabFirmwarePluginFactory : public FirmwarePluginFactory
{
    Q_OBJECT

public:
    QList<QGCMAVLink::FirmwareClass_t> supportedFirmwareClasses() const final;
    QList<QGCMAVLink::VehicleClass_t> supportedVehicleClasses() const final;
    FirmwarePlugin* firmwarePluginForAutopilot(MAV_AUTOPILOT autopilotType, MAV_TYPE vehicleType) final;

private:
    VibeAILabFirmwarePlugin* _pluginInstance = nullptr;
};
