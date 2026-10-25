#include "VibeAILabFirmwarePluginFactory.h"

#include "QGCLoggingCategory.h"
#include "VibeAILabFirmwarePlugin.h"

QGC_LOGGING_CATEGORY(VibeAILabFirmwarePluginFactoryLog, "VibeAILab.FirmwarePluginFactory")

static VibeAILabFirmwarePluginFactory _vibeAILabFirmwarePluginFactory;

QList<QGCMAVLink::FirmwareClass_t> VibeAILabFirmwarePluginFactory::supportedFirmwareClasses() const
{
    return {QGCMAVLink::FirmwareClassGeneric};
}

QList<QGCMAVLink::VehicleClass_t> VibeAILabFirmwarePluginFactory::supportedVehicleClasses() const
{
    return {QGCMAVLink::VehicleClassMultiRotor};
}

FirmwarePlugin* VibeAILabFirmwarePluginFactory::firmwarePluginForAutopilot(MAV_AUTOPILOT autopilotType,
                                                                           MAV_TYPE vehicleType)
{
    if ((autopilotType != MAV_AUTOPILOT_GENERIC) || (vehicleType != MAV_TYPE_QUADROTOR)) {
        return nullptr;
    }

    if (!_pluginInstance) {
        _pluginInstance = new VibeAILabFirmwarePlugin();
    }
    qCDebug(VibeAILabFirmwarePluginFactoryLog) << "VibeAILab firmware plugin chosen";

    return _pluginInstance;
}
