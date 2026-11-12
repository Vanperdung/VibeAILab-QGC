#include "VibeAILabSensorsComponent.h"

#include <QtCore/QLatin1StringView>
#include <QtCore/QtNumeric>
#include <iterator>

#include "ParameterManager.h"
#include "Vehicle.h"

namespace {

// Untranslated section ids; the Sensors page QML compares against the same texts
constexpr QLatin1StringView sensorsGyroSectionId("Gyroscope");
constexpr QLatin1StringView sensorsAccelSectionId("Accelerometer");

constexpr QLatin1StringView sensorsAccelOffsetParams[] = {
    QLatin1StringView("IMU_ACCEL_OFFSET_X"),
    QLatin1StringView("IMU_ACCEL_OFFSET_Y"),
    QLatin1StringView("IMU_ACCEL_OFFSET_Z"),
};

}  // namespace

VibeAILabSensorsComponent::VibeAILabSensorsComponent(Vehicle* vehicle, AutoPilotPlugin* autopilot, QObject* parent)
    : VehicleComponent(vehicle, autopilot, AutoPilotPlugin::KnownSensorsVehicleComponent, parent)
{}

QString VibeAILabSensorsComponent::name() const
{
    return tr("Sensors");
}

QString VibeAILabSensorsComponent::description() const
{
    return tr("Calibrate the gyroscope and the accelerometer.");
}

QString VibeAILabSensorsComponent::iconResource() const
{
    return QStringLiteral("/qmlimages/SensorsComponentIcon.png");
}

bool VibeAILabSensorsComponent::setupComplete() const
{
    return _accelCalibrated();
}

QUrl VibeAILabSensorsComponent::setupSource() const
{
    return QUrl::fromUserInput(QStringLiteral("qrc:/qml/VibeAILab/Sensors/VibeAILabSensorsComponent.qml"));
}

QStringList VibeAILabSensorsComponent::setupCompleteChangedTriggerList() const
{
    return {std::begin(sensorsAccelOffsetParams), std::end(sensorsAccelOffsetParams)};
}

QStringList VibeAILabSensorsComponent::sectionIds() const
{
    return {sensorsGyroSectionId, sensorsAccelSectionId};
}

QString VibeAILabSensorsComponent::sectionDisplayName(const QString& sectionId) const
{
    if (sectionId == sensorsGyroSectionId) {
        return tr("Gyroscope");
    }
    if (sectionId == sensorsAccelSectionId) {
        return tr("Accelerometer");
    }

    return sectionId;
}

bool VibeAILabSensorsComponent::sectionSetupComplete(const QString& sectionId) const
{
    // The FC calibrates the gyro at every boot, so only the accel can need setup
    return (sectionId == sensorsAccelSectionId) ? _accelCalibrated() : true;
}

/// Same rule as the FC's arm check: the accel is uncalibrated while any offset is 0 or not finite.
bool VibeAILabSensorsComponent::_accelCalibrated() const
{
    ParameterManager* const parameterManager = _vehicle ? _vehicle->parameterManager() : nullptr;
    if (!parameterManager) {
        return true;
    }

    for (const QLatin1StringView paramName : sensorsAccelOffsetParams) {
        const Fact* const offsetFact =
            parameterManager->parameterExists(ParameterManager::defaultComponentId, paramName)
                ? parameterManager->getParameter(ParameterManager::defaultComponentId, paramName)
                : nullptr;
        if (!offsetFact) {
            // An FC that does not serve the offsets gives nothing to judge by
            return true;
        }

        const double offset = offsetFact->rawValue().toDouble();
        if ((offset == 0.0) || !qIsFinite(offset)) {
            return false;
        }
    }

    return true;
}
