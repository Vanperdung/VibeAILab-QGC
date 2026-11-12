#include "VibeAILabCalibration.h"

namespace {

VibeAILabCalibration::Result startRefusedResult(VibeAILabCalibration::CommandReply reply)
{
    switch (reply) {
        case VibeAILabCalibration::CommandReply::VehicleArmed:
            return VibeAILabCalibration::Result::VehicleArmed;
        case VibeAILabCalibration::CommandReply::Unsupported:
            return VibeAILabCalibration::Result::Unsupported;
        case VibeAILabCalibration::CommandReply::NoResponse:
            return VibeAILabCalibration::Result::NoResponse;
        case VibeAILabCalibration::CommandReply::NotSent:
            return VibeAILabCalibration::Result::NotSent;
        case VibeAILabCalibration::CommandReply::Accepted:
        case VibeAILabCalibration::CommandReply::Rejected:
            break;
    }

    return VibeAILabCalibration::Result::Rejected;
}

}  // namespace

VibeAILabCalibration::VibeAILabCalibration(QObject* parent) : QObject(parent) {}

QString VibeAILabCalibration::statusText() const
{
    switch (_state) {
        case State::Starting:
            return tr("Waiting for the vehicle to start the calibration");
        case State::Running:
        case State::Restarting:
        case State::Cancelling:
            return _runningHelpText();
        case State::Idle:
            break;
    }

    switch (_result) {
        case Result::None:
            break;
        case Result::Done:
            return tr("Calibration complete");
        case Result::Failed:
            return tr("Calibration failed. The old values are kept.");
        case Result::Cancelled:
            return tr("Calibration cancelled");
        case Result::VehicleArmed:
            return tr("Calibration rejected: vehicle armed");
        case Result::Unsupported:
            return tr("This flight controller does not support this calibration");
        case Result::Rejected:
            return tr("Calibration rejected by the vehicle");
        case Result::NoResponse:
            return tr("No response from the vehicle");
        case Result::NotSent:
            return tr("Calibration not started: the vehicle has not replied to the previous command yet");
    }

    return {};
}

bool VibeAILabCalibration::requestStart()
{
    switch (_state) {
        case State::Idle:
            _setState(State::Starting, Result::None);
            return true;
        case State::Running:
            _setState(State::Restarting, Result::None);
            return true;
        case State::Starting:
        case State::Restarting:
        case State::Cancelling:
            break;
    }

    return false;
}

bool VibeAILabCalibration::requestCancel()
{
    if (_state != State::Running) {
        return false;
    }

    _setState(State::Cancelling, Result::None);
    return true;
}

void VibeAILabCalibration::handleStartReply(CommandReply reply)
{
    if (reply == CommandReply::Accepted) {
        switch (_state) {
            case State::Starting:
            case State::Restarting:
                _beginRun();
                break;
            case State::Idle:
                // The run going on ended while the command to start it again was on its way: a new run started
                _beginRun();
                break;
            case State::Running:
                // The "started" text came before this reply, and the run already has what came after it
            case State::Cancelling:
                break;
        }
        return;
    }

    switch (_state) {
        case State::Starting:
            _finish(startRefusedResult(reply));
            break;
        case State::Restarting:
            // The FC goes on with its run. The owner tells the user about the failed command.
            _setState(State::Running, Result::None);
            break;
        case State::Idle:
        case State::Running:
        case State::Cancelling:
            break;
    }
}

void VibeAILabCalibration::handleCancelReply(CommandReply reply)
{
    if (_state != State::Cancelling) {
        // A status text already ended the run
        return;
    }

    if (reply == CommandReply::Accepted) {
        // The FC sends no "cancelled" text when it had no run of its own to stop
        _finish(Result::Cancelled);
    } else {
        // The FC goes on with its run. The owner tells the user about the failed command.
        _setState(State::Running, Result::None);
    }
}

void VibeAILabCalibration::clearResult()
{
    if (_state == State::Idle) {
        _setState(State::Idle, Result::None);
    }
}

void VibeAILabCalibration::_handleStartedText()
{
    switch (_state) {
        case State::Starting:
        case State::Running:
        case State::Restarting:
            _beginRun();
            break;
        case State::Idle:
            // The reply to the start command was lost or late, but the FC got the command
            if (_result == Result::NoResponse) {
                _beginRun();
            }
            break;
        case State::Cancelling:
            break;
    }
}

void VibeAILabCalibration::_finish(Result result)
{
    _setState(State::Idle, result);
}

void VibeAILabCalibration::_beginRun()
{
    _resetRun();
    _setState(State::Running, Result::None);
}

void VibeAILabCalibration::_setState(State state, Result result)
{
    const bool stateDiffers = (_state != state);
    const bool resultDiffers = (_result != result);

    _state = state;
    _result = result;

    if (stateDiffers) {
        emit stateChanged();
    }
    if (resultDiffers) {
        emit resultChanged();
    }
    if (stateDiffers || resultDiffers) {
        emit statusTextChanged();
    }
}
