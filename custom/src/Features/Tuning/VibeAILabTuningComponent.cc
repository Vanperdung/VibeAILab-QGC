#include "VibeAILabTuningComponent.h"

VibeAILabTuningComponent::VibeAILabTuningComponent(Vehicle* vehicle, AutoPilotPlugin* autopilot, QObject* parent)
    : VehicleComponent(vehicle, autopilot, AutoPilotPlugin::UnknownVehicleComponent, parent)
{}

QString VibeAILabTuningComponent::name() const
{
    return tr("PID Tuning");
}

QString VibeAILabTuningComponent::description() const
{
    return tr("Tune the gains of the rate and attitude controllers.");
}

QString VibeAILabTuningComponent::iconResource() const
{
    return QStringLiteral("/qmlimages/TuningComponentIcon.png");
}

QUrl VibeAILabTuningComponent::setupSource() const
{
    return QUrl::fromUserInput(QStringLiteral("qrc:/qml/VibeAILab/Tuning/VibeAILabTuningComponent.qml"));
}
