import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls

/// Chart and gain sliders of one controller (core PIDTuning), or a note that lists the gain params the FC does not serve.
ColumnLayout {
    id: root

    required property real   availableWidth
    required property real   availableHeight
    required property var    axis               ///< Same format as PIDTuning.axis
    required property string title
    required property string unit
    property double          chartDisplaySec:   8

    // Not re-checked if params appear later: parameterExists() is a call that QML does not track
    readonly property var _missingParams: {
        const missing = []
        for (let i = 0; i < axis.length; i++) {
            const params = axis[i].params
            for (let j = 0; j < params.count; j++) {
                const paramName = params.get(j).param
                if (!controller.parameterExists(-1, paramName)) {
                    missing.push(paramName)
                }
            }
        }
        return missing
    }

    QGCLabel {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * 60
        wrapMode:               Text.WordWrap
        visible:                root._missingParams.length > 0
        text:                   qsTr("The flight controller does not serve these parameters: %1. Update its firmware to tune them here.").arg(root._missingParams.join(", "))
    }

    // PIDTuning reads every listed param as soon as it is built, so build it only when all of them exist
    Loader {
        id:     pidTuningLoader
        active: root._missingParams.length === 0

        sourceComponent: PIDTuning {
            availableWidth:     root.availableWidth
            availableHeight:    root.availableHeight - pidTuningLoader.y
            title:              root.title
            tuningMode:         Vehicle.ModeRateAndAttitude
            unit:               root.unit
            axis:               root.axis
            chartDisplaySec:    root.chartDisplaySec
        }
    }
}
