#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtQmlIntegration/QtQmlIntegration>

/// What the Sensors page shows for one FC sensor calibration.
///
/// It follows the user's clicks, the FC's replies to the calibration commands and the FC's status texts.
/// It sends nothing itself: the owner sends a command when requestStart() or requestCancel() returns true,
/// and passes the reply to that command to handleStartReply() or handleCancelReply().
///
///     Idle -> Starting -> Running -> Idle
///                         Running -> Restarting -> Running
///                         Running -> Cancelling -> Idle
class VibeAILabCalibration : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Owned by VibeAILabSensorsController")
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool active READ active NOTIFY stateChanged)
    Q_PROPERTY(bool runGoingOn READ runGoingOn NOTIFY stateChanged)
    Q_PROPERTY(Result result READ result NOTIFY resultChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)

public:
    enum class State
    {
        Idle,        ///< No run
        Starting,    ///< Start command sent, FC has not started the run yet
        Running,     ///< FC run goes on
        Restarting,  ///< FC run goes on, start command sent to start it again
        Cancelling   ///< FC run goes on, cancel command sent
    };
    Q_ENUM(State)

    /// How the last run ended. None before the first run and while a run is active.
    enum class Result
    {
        None,
        Done,
        Failed,
        Cancelled,
        VehicleArmed,  ///< Start rejected: the vehicle is armed
        Unsupported,   ///< Start rejected: the FC has no such calibration
        Rejected,      ///< Start rejected for another reason
        NoResponse,    ///< No reply to the start command
        NotSent        ///< The start command was not sent
    };
    Q_ENUM(Result)

    /// What came back for a start or cancel command.
    enum class CommandReply
    {
        Accepted,
        VehicleArmed,
        Unsupported,
        Rejected,
        NoResponse,  ///< The FC did not reply in time
        NotSent      ///< QGC did not send the command
    };
    Q_ENUM(CommandReply)

    State state() const { return _state; }

    bool active() const { return _state != State::Idle; }

    /// True while the FC run goes on.
    bool runGoingOn() const
    {
        return (_state == State::Running) || (_state == State::Restarting) || (_state == State::Cancelling);
    }

    Result result() const { return _result; }

    QString statusText() const;

    /// The user asks to start a run, or to start the run going on again.
    /// @return true when the start command must be sent
    [[nodiscard]] bool requestStart();

    /// The user asks to cancel the run.
    /// @return true when the cancel command must be sent
    [[nodiscard]] bool requestCancel();

    /// The reply to the start command sent after requestStart().
    void handleStartReply(CommandReply reply);

    /// The reply to the cancel command sent after requestCancel().
    void handleCancelReply(CommandReply reply);

    /// The FC sent a status text.
    virtual void handleStatusText(const QString& text) = 0;

    /// Forgets how the last run ended. Does nothing while a run is active.
    Q_INVOKABLE void clearResult();

signals:
    void stateChanged();
    void resultChanged();
    void statusTextChanged();

protected:
    explicit VibeAILabCalibration(QObject* parent = nullptr);

    /// The FC sent the text that says it started the run.
    void _handleStartedText();

    /// The run ended.
    void _finish(Result result);

    /// Clears what was collected for the previous run.
    virtual void _resetRun() {}

    /// What the user must do while the FC run goes on.
    virtual QString _runningHelpText() const = 0;

private:
    void _beginRun();
    void _setState(State state, Result result);

    State _state = State::Idle;
    Result _result = Result::None;
};
