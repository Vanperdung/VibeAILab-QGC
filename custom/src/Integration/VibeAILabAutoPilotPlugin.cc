#include "VibeAILabAutoPilotPlugin.h"

#include <QtCore/QJsonDocument>

#include "ActuatorComponent.h"
#include "Actuators.h"
#include "JoystickComponent.h"
#include "JsonParsing.h"
#include "JsonSchemaValidator.h"
#include "ParameterManager.h"
#include "QGCLoggingCategory.h"
#include "Vehicle.h"

QGC_LOGGING_CATEGORY(VibeAILabAutoPilotPluginLog, "VibeAILab.AutoPilotPlugin")

static constexpr const char* ActuatorMetadataFile = ":/VibeAILab/ActuatorMetadata.json";

static QJsonDocument loadActuatorMetadata()
{
    QJsonDocument jsonDoc;
    QString errorString;
    if (!JsonParsing::isJsonFile(QString(ActuatorMetadataFile), jsonDoc, errorString)) {
        qCWarning(VibeAILabAutoPilotPluginLog) << "Actuator metadata open failed:" << errorString;
        return {};
    }

    if (!JsonSchemaValidator::validate(jsonDoc, QStringLiteral(":/json/component_metadata/actuators.schema.json"),
                                       errorString)) {
        qCWarning(VibeAILabAutoPilotPluginLog) << "Actuator metadata schema validation failed:" << errorString;
    }

    return jsonDoc;
}

VibeAILabAutoPilotPlugin::VibeAILabAutoPilotPlugin(Vehicle* vehicle, QObject* parent) : AutoPilotPlugin(vehicle, parent)
{
    _vehicle->setActuatorsMetadata(MAV_COMP_ID_AUTOPILOT1, QString(ActuatorMetadataFile), loadActuatorMetadata());
}

const QVariantList& VibeAILabAutoPilotPlugin::vehicleComponents()
{
    if (!_components.isEmpty()) {
        return _components;
    }

    if (!_vehicle->parameterManager()->parametersReady()) {
        qCWarning(VibeAILabAutoPilotPluginLog) << "Components requested before parameters are ready";
        return _components;
    }

    Actuators* const actuators = _vehicle->actuators();
    if (actuators != nullptr) {
        actuators->init();
        if (actuators->showUi()) {
            auto* const actuatorComponent = new ActuatorComponent(_vehicle, this, this);
            actuatorComponent->setupTriggerSignals();
            _components.append(QVariant::fromValue(qobject_cast<VehicleComponent*>(actuatorComponent)));
        } else {
            qCWarning(VibeAILabAutoPilotPluginLog) << "Actuators page hidden:" << actuators->initializationError();
        }
    }

    auto* const joystickComponent = new JoystickComponent(_vehicle, this, this);
    joystickComponent->setupTriggerSignals();
    _components.append(QVariant::fromValue(qobject_cast<VehicleComponent*>(joystickComponent)));

    return _components;
}
