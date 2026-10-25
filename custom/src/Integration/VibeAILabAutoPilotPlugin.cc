#include "VibeAILabAutoPilotPlugin.h"

#include "JoystickComponent.h"

VibeAILabAutoPilotPlugin::VibeAILabAutoPilotPlugin(Vehicle* vehicle, QObject* parent) : AutoPilotPlugin(vehicle, parent)
{}

const QVariantList& VibeAILabAutoPilotPlugin::vehicleComponents()
{
    if (_components.isEmpty()) {
        auto* const joystickComponent = new JoystickComponent(_vehicle, this, this);
        joystickComponent->setupTriggerSignals();
        _components.append(QVariant::fromValue(qobject_cast<VehicleComponent*>(joystickComponent)));
    }

    return _components;
}
