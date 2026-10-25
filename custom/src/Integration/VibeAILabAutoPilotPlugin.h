#pragma once

#include "AutoPilotPlugin.h"

/// Setup plugin for VibeAI Lab flight controller.
class VibeAILabAutoPilotPlugin : public AutoPilotPlugin
{
    Q_OBJECT

public:
    explicit VibeAILabAutoPilotPlugin(Vehicle* vehicle, QObject* parent = nullptr);

    const QVariantList& vehicleComponents() final;

    QString prerequisiteSetup(VehicleComponent* /*component*/) const final { return {}; }

private:
    QVariantList _components;
};
