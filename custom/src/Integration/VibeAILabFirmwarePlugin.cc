#include "VibeAILabFirmwarePlugin.h"

#include "Vehicle.h"
#include "VibeAILabAutoPilotPlugin.h"

VibeAILabFirmwarePlugin::VibeAILabFirmwarePlugin(QObject* parent) : FirmwarePlugin(parent) {}

AutoPilotPlugin* VibeAILabFirmwarePlugin::autopilotPlugin(Vehicle* vehicle) const
{
    return new VibeAILabAutoPilotPlugin(vehicle, vehicle);
}
