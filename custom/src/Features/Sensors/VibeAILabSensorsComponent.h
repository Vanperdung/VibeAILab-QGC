#pragma once

#include "VehicleComponent.h"

/// Setup page "Sensors" for VibeAI Lab flight controller: Gyro Calibration and Accel Calibration.
class VibeAILabSensorsComponent : public VehicleComponent
{
    Q_OBJECT

public:
    VibeAILabSensorsComponent(Vehicle* vehicle, AutoPilotPlugin* autopilot, QObject* parent = nullptr);

    QString name() const final;
    QString description() const final;
    QString iconResource() const final;

    bool requiresSetup() const final { return true; }

    bool setupComplete() const final;
    QUrl setupSource() const final;

    QUrl summaryQmlSource() const final { return {}; }

    QStringList setupCompleteChangedTriggerList() const final;

    QStringList sectionIds() const final;
    QString sectionDisplayName(const QString& sectionId) const final;

    bool showFirstSectionOnRootClick() const final { return true; }

    bool sectionSetupComplete(const QString& sectionId) const final;

private:
    bool _accelCalibrated() const;
};
