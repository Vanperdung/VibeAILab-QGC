#include "VibeAILabSensorsController.h"

#include "ParameterManager.h"
#include "QGCLoggingCategory.h"
#include "Vehicle.h"
#include "VibeAILabAccelCalibration.h"
#include "VibeAILabGyroCalibration.h"

QGC_LOGGING_CATEGORY(VibeAILabSensorsControllerLog, "VibeAILab.SensorsController")

namespace {

// Params the FC sets when a calibration is done
constexpr const char* gyroCalParamPattern = "IMU_GYRO_*";
constexpr const char* accelCalParamPattern = "IMU_ACCEL_*";

VibeAILabCalibration::CommandReply calibrationCommandReply(int ackResult, int failureCode)
{
    using CommandReply = VibeAILabCalibration::CommandReply;

    switch (failureCode) {
        case Vehicle::MavCmdResultCommandResultOnly:
            break;
        case Vehicle::MavCmdResultFailureNoResponseToCommand:
            return CommandReply::NoResponse;
        default:
            // The command queue refused the command: the same command already waits for a reply
            return CommandReply::NotSent;
    }

    switch (ackResult) {
        case MAV_RESULT_ACCEPTED:
            return CommandReply::Accepted;
        case MAV_RESULT_TEMPORARILY_REJECTED:
            // The FC gives this reply only while armed
            return CommandReply::VehicleArmed;
        case MAV_RESULT_UNSUPPORTED:
            return CommandReply::Unsupported;
        default:
            return CommandReply::Rejected;
    }
}

}  // namespace

VibeAILabSensorsController::VibeAILabSensorsController(QObject* parent)
    : FactPanelController(parent),
      _gyro(new VibeAILabGyroCalibration(this)),
      _accel(new VibeAILabAccelCalibration(this))
{
    (void) connect(_gyro, &VibeAILabCalibration::stateChanged, this,
                   &VibeAILabSensorsController::calibrationActiveChanged);
    (void) connect(_accel, &VibeAILabCalibration::stateChanged, this,
                   &VibeAILabSensorsController::calibrationActiveChanged);

    (void) connect(_gyro, &VibeAILabCalibration::resultChanged, this, [this]() {
        if (_gyro->result() == VibeAILabCalibration::Result::Done) {
            _refreshParams(gyroCalParamPattern);
        }
    });
    (void) connect(_accel, &VibeAILabCalibration::resultChanged, this, [this]() {
        if (_accel->result() == VibeAILabCalibration::Result::Done) {
            _refreshParams(accelCalParamPattern);
        }
    });

    if (!_vehicle) {
        qCWarning(VibeAILabSensorsControllerLog) << "No vehicle";
        return;
    }

    (void) connect(_vehicle, &Vehicle::mavCommandResult, this, &VibeAILabSensorsController::_handleCommandResult);
    (void) connect(_vehicle, &Vehicle::textMessageReceived, this, &VibeAILabSensorsController::_handleTextMessage);
}

void VibeAILabSensorsController::startGyroCalibration()
{
    _startCalibration(_gyro, 1 /* gyroParam: calibrate */, 0 /* accelParam */);
}

void VibeAILabSensorsController::startAccelCalibration()
{
    _startCalibration(_accel, 0 /* gyroParam */, PREFLIGHT_CALIBRATION_ACCELEROMETER_FULL);
}

void VibeAILabSensorsController::cancelCalibration()
{
    VibeAILabCalibration* const calibration = _activeCalibration();
    if (!calibration || !_canSendCommand() || !calibration->requestCancel()) {
        return;
    }

    // Calibration command with all params 0. A failed cancel leaves the page as it is, so the command queue shows
    // it to the user.
    _setPendingCommand(PendingCommand{.calibration = calibration, .kind = CommandKind::Cancel});
    _vehicle->stopCalibration(true /* showError */);
}

void VibeAILabSensorsController::_handleCommandResult(int /*vehicleId*/, int targetComponent, int command,
                                                      int ackResult, int failureCode)
{
    if (!_vehicle || !_pendingCommand || (command != MAV_CMD_PREFLIGHT_CALIBRATION) ||
        (targetComponent != _vehicle->defaultComponentId())) {
        return;
    }

    qCDebug(VibeAILabSensorsControllerLog) << "Calibration command reply"
                                           << "ackResult:" << ackResult << "failureCode:" << failureCode;

    const PendingCommand pendingCommand = *_pendingCommand;
    _setPendingCommand(std::nullopt);

    const VibeAILabCalibration::CommandReply reply = calibrationCommandReply(ackResult, failureCode);
    switch (pendingCommand.kind) {
        case CommandKind::Start:
            pendingCommand.calibration->handleStartReply(reply);
            break;
        case CommandKind::Cancel:
            pendingCommand.calibration->handleCancelReply(reply);
            break;
    }
}

void VibeAILabSensorsController::_handleTextMessage(int /*vehicleId*/, int /*componentId*/, int /*severity*/,
                                                    const QString& text, const QString& /*description*/)
{
    _gyro->handleStatusText(text);
    _accel->handleStatusText(text);
}

VibeAILabCalibration* VibeAILabSensorsController::_activeCalibration() const
{
    if (_gyro->active()) {
        return _gyro;
    }
    if (_accel->active()) {
        return _accel;
    }

    return nullptr;
}

bool VibeAILabSensorsController::_canSendCommand() const
{
    if (!_vehicle) {
        qCWarning(VibeAILabSensorsControllerLog) << "No vehicle";
        return false;
    }

    if (_pendingCommand) {
        qCWarning(VibeAILabSensorsControllerLog) << "A calibration command still waits for its reply";
        return false;
    }

    return true;
}

void VibeAILabSensorsController::_setPendingCommand(const std::optional<PendingCommand>& pendingCommand)
{
    const bool wasPending = commandPending();
    _pendingCommand = pendingCommand;
    if (wasPending != commandPending()) {
        emit commandPendingChanged();
    }
}

void VibeAILabSensorsController::_startCalibration(VibeAILabCalibration* calibration, float gyroParam, float accelParam)
{
    if (!_canSendCommand()) {
        return;
    }

    const VibeAILabCalibration* const activeCalibration = _activeCalibration();
    if (activeCalibration && (activeCalibration != calibration)) {
        qCWarning(VibeAILabSensorsControllerLog) << "Another calibration is active";
        return;
    }

    // The page shows a failed start of a new run itself. A failed start of the run going on leaves the page as it
    // is, so the command queue shows it to the user.
    const bool startAgain = calibration->active();
    if (!calibration->requestStart()) {
        return;
    }

    // Sent through the command queue, without retry, so the reply or its absence comes back in mavCommandResult.
    // The result can come back before sendMavCommand() returns, so the pending command is set first.
    _setPendingCommand(PendingCommand{.calibration = calibration, .kind = CommandKind::Start});
    _vehicle->sendMavCommand(_vehicle->defaultComponentId(), MAV_CMD_PREFLIGHT_CALIBRATION, startAgain /* showError */,
                             gyroParam, 0 /* magnetometer */, 0 /* pressure */, 0 /* radio */, accelParam);
}

void VibeAILabSensorsController::_refreshParams(const QString& namePattern)
{
    ParameterManager* const parameterManager = _vehicle ? _vehicle->parameterManager() : nullptr;
    if (!parameterManager) {
        return;
    }

    // The FC does not send a param by itself when it changes one
    parameterManager->bulkRefresh(ParameterManager::defaultComponentId, {namePattern}, false /* notifyFailure */);
}
