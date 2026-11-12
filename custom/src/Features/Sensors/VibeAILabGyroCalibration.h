#pragma once

#include "VibeAILabCalibration.h"

/// Gyro Calibration as the Sensors page shows it.
///
/// The FC accepts the start command at once, then reports the end of the run with a status text.
/// A run has no time limit and never fails: it ends when the vehicle was still long enough.
class VibeAILabGyroCalibration : public VibeAILabCalibration
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Owned by VibeAILabSensorsController")

public:
    explicit VibeAILabGyroCalibration(QObject* parent = nullptr);

    void handleStatusText(const QString& text) final;

protected:
    QString _runningHelpText() const final;
};
