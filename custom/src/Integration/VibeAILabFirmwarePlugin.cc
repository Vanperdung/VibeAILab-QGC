#include "VibeAILabFirmwarePlugin.h"

#include "Vehicle.h"
#include "VibeAILabAutoPilotPlugin.h"
#include "VibeAILabParameterMetaData.h"

VibeAILabFirmwarePlugin::VibeAILabFirmwarePlugin(QObject* parent) : FirmwarePlugin(parent) {}

AutoPilotPlugin* VibeAILabFirmwarePlugin::autopilotPlugin(Vehicle* vehicle) const
{
    return new VibeAILabAutoPilotPlugin(vehicle, vehicle);
}

QString VibeAILabFirmwarePlugin::_internalParameterMetaDataFile(const Vehicle* /*vehicle*/) const
{
    return QStringLiteral(":/VibeAILab/ParameterMetaData.json");
}

ParameterMetaData* VibeAILabFirmwarePlugin::_createParameterMetaData()
{
    return new VibeAILabParameterMetaData();
}
