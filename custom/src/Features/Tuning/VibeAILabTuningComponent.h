#pragma once

#include "VehicleComponent.h"

/// Setup page "PID Tuning" for VibeAI Lab flight controller: Rate Controller and Attitude Controller gains.
class VibeAILabTuningComponent : public VehicleComponent
{
    Q_OBJECT

public:
    VibeAILabTuningComponent(Vehicle* vehicle, AutoPilotPlugin* autopilot, QObject* parent = nullptr);

    QString name() const final;
    QString description() const final;
    QString iconResource() const final;

    bool requiresSetup() const final { return false; }

    bool setupComplete() const final { return true; }

    QUrl setupSource() const final;

    QUrl summaryQmlSource() const final { return {}; }

    QStringList setupCompleteChangedTriggerList() const final { return {}; }

    // Gains are tuned live while the motors spin, with the quad on a test stand
    bool allowSetupWhileArmed() const final { return true; }

    bool allowSetupWhileFlying() const final { return true; }
};
