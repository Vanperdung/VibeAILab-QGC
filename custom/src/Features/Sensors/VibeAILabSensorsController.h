#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QString>
#include <QtQmlIntegration/QtQmlIntegration>
#include <optional>

#include "FactPanelController.h"

class VibeAILabAccelCalibration;
class VibeAILabCalibration;
class VibeAILabGyroCalibration;

Q_DECLARE_LOGGING_CATEGORY(VibeAILabSensorsControllerLog)

/// Controller of the Sensors page.
///
/// Sends the calibration commands to the FC and passes the FC's command replies and status texts to the
/// calibrations. Only one calibration is active at a time.
///
/// Start, start again and cancel are all the same MAVLink command, and QGC's command queue keeps only one of them
/// waiting for a reply. So the controller sends no calibration command while one waits, and it records which
/// calibration and which request the waiting command belongs to.
class VibeAILabSensorsController : public FactPanelController
{
    Q_OBJECT
    QML_ELEMENT
    Q_MOC_INCLUDE("VibeAILabAccelCalibration.h")
    Q_MOC_INCLUDE("VibeAILabGyroCalibration.h")

    Q_PROPERTY(VibeAILabGyroCalibration* gyro READ gyro CONSTANT)
    Q_PROPERTY(VibeAILabAccelCalibration* accel READ accel CONSTANT)
    Q_PROPERTY(bool calibrationActive READ calibrationActive NOTIFY calibrationActiveChanged)
    Q_PROPERTY(bool commandPending READ commandPending NOTIFY commandPendingChanged)

public:
    explicit VibeAILabSensorsController(QObject* parent = nullptr);

    VibeAILabGyroCalibration* gyro() const { return _gyro; }

    VibeAILabAccelCalibration* accel() const { return _accel; }

    bool calibrationActive() const { return _activeCalibration() != nullptr; }

    /// True while a calibration command waits for its reply. No start or cancel is sent meanwhile.
    bool commandPending() const { return _pendingCommand.has_value(); }

    Q_INVOKABLE void startGyroCalibration();

    /// Starts the Accel Calibration, or starts it again when it is active.
    Q_INVOKABLE void startAccelCalibration();

    /// Cancels the active calibration.
    Q_INVOKABLE void cancelCalibration();

signals:
    void calibrationActiveChanged();
    void commandPendingChanged();

private slots:
    void _handleCommandResult(int vehicleId, int targetComponent, int command, int ackResult, int failureCode);
    void _handleTextMessage(int vehicleId, int componentId, int severity, const QString& text,
                            const QString& description);

private:
    enum class CommandKind
    {
        Start,
        Cancel
    };

    struct PendingCommand
    {
        VibeAILabCalibration* calibration = nullptr;
        CommandKind kind = CommandKind::Start;
    };

    VibeAILabCalibration* _activeCalibration() const;
    bool _canSendCommand() const;
    void _setPendingCommand(const std::optional<PendingCommand>& pendingCommand);
    void _startCalibration(VibeAILabCalibration* calibration, float gyroParam, float accelParam);
    void _refreshParams(const QString& namePattern);

    VibeAILabGyroCalibration* _gyro = nullptr;
    VibeAILabAccelCalibration* _accel = nullptr;
    std::optional<PendingCommand> _pendingCommand;
};
