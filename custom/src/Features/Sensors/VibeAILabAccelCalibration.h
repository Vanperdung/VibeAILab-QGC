#pragma once

#include <QtCore/QList>
#include <array>

#include "VibeAILabCalibration.h"

/// Accel Calibration as the Sensors page shows it: the run and the state of each of the 6 sides.
///
/// Follows the FC's "[cal] " status texts. The texts and the rules are in the FC repo:
/// user/docs/accel_calibration/interface-0001-fc-qgc-accel-calibration.md
class VibeAILabAccelCalibration : public VibeAILabCalibration
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Owned by VibeAILabSensorsController")
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

    /// State of each side as an int, indexed by Side
    Q_PROPERTY(QList<int> sideStates READ sideStates NOTIFY sideStatesChanged)

public:
    /// Vehicle pose that puts one side down.
    enum class Side
    {
        RightSideUp,
        UpsideDown,
        LeftSideDown,
        RightSideDown,
        NoseDown,
        TailDown
    };
    Q_ENUM(Side)

    enum class SideState
    {
        Idle,        ///< No run to show
        NotDone,
        InProgress,  ///< The FC collects this side
        Done
    };
    Q_ENUM(SideState)

    explicit VibeAILabAccelCalibration(QObject* parent = nullptr);

    SideState sideState(Side side) const;
    QList<int> sideStates() const;

    void handleStatusText(const QString& text) final;

signals:
    void sideStatesChanged();

protected:
    void _resetRun() final;
    QString _runningHelpText() const final;

private:
    static constexpr int SideCount = 6;

    void _collectSide(Side side);
    void _loseSideInProgress();
    void _completeSide(Side side);
    void _setRunSideStates(const std::array<SideState, SideCount>& runSideStates);

    /// Side states of the run going on; never Idle
    std::array<SideState, SideCount> _runSideStates{};
};
