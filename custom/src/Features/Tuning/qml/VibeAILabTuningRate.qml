import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.FactControls

/// Rate tab of the PID Tuning page: the Thrust Curve and the Rate Controller gains.
ColumnLayout {
    id: root

    // availableWidth and availableHeight come from the tab Loader
    readonly property real _availableWidth:     availableWidth
    readonly property real _availableHeight:    availableHeight
    readonly property var  _vehicle:            globals.activeVehicle
    readonly property Fact _thrustCurve:        controller.getParameterFact(-1, "MOT_THR_CURVE", false)

    // Slider ranges are around the values the FC was built with (roll/pitch P 0.15, I 0.2, D 0.003; yaw P 0.3, I 0.05, D 0)
    property var roll: QtObject {
        property string name: qsTr("Roll")
        property var plot: [
            { name: qsTr("Response"), value: root._vehicle ? root._vehicle.rollRate.value : NaN },
            { name: qsTr("Setpoint"), value: root._vehicle ? root._vehicle.setpoint.rollRate.value : NaN }
        ]
        property var params: ListModel {
            ListElement {
                title:          qsTr("Proportional Gain (RATE_P_X)")
                description:    qsTr("Main gain: increase for a faster response, reduce if the rate overshoots or shakes.")
                param:          "RATE_P_X"
                min:            0
                max:            0.5
                step:           0.05
            }
            ListElement {
                title:          qsTr("Differential Gain (RATE_D_X)")
                description:    qsTr("Damping: increase to reduce overshoot and shaking, but not higher than needed.")
                param:          "RATE_D_X"
                min:            0
                max:            0.01
                step:           0.0005
            }
            ListElement {
                title:          qsTr("Integral Gain (RATE_I_X)")
                description:    qsTr("Removes a steady offset. Reduce it when you see slow oscillations.")
                param:          "RATE_I_X"
                min:            0
                max:            0.6
                step:           0.05
            }
        }
    }

    property var pitch: QtObject {
        property string name: qsTr("Pitch")
        property var plot: [
            { name: qsTr("Response"), value: root._vehicle ? root._vehicle.pitchRate.value : NaN },
            { name: qsTr("Setpoint"), value: root._vehicle ? root._vehicle.setpoint.pitchRate.value : NaN }
        ]
        property var params: ListModel {
            ListElement {
                title:          qsTr("Proportional Gain (RATE_P_Y)")
                description:    qsTr("Main gain: increase for a faster response, reduce if the rate overshoots or shakes.")
                param:          "RATE_P_Y"
                min:            0
                max:            0.5
                step:           0.05
            }
            ListElement {
                title:          qsTr("Differential Gain (RATE_D_Y)")
                description:    qsTr("Damping: increase to reduce overshoot and shaking, but not higher than needed.")
                param:          "RATE_D_Y"
                min:            0
                max:            0.01
                step:           0.0005
            }
            ListElement {
                title:          qsTr("Integral Gain (RATE_I_Y)")
                description:    qsTr("Removes a steady offset. Reduce it when you see slow oscillations.")
                param:          "RATE_I_Y"
                min:            0
                max:            0.6
                step:           0.05
            }
        }
    }

    property var yaw: QtObject {
        property string name: qsTr("Yaw")
        property var plot: [
            { name: qsTr("Response"), value: root._vehicle ? root._vehicle.yawRate.value : NaN },
            { name: qsTr("Setpoint"), value: root._vehicle ? root._vehicle.setpoint.yawRate.value : NaN }
        ]
        property var params: ListModel {
            ListElement {
                title:          qsTr("Proportional Gain (RATE_P_Z)")
                description:    qsTr("Main gain: increase for a faster response, reduce if the rate overshoots or shakes.")
                param:          "RATE_P_Z"
                min:            0
                max:            1
                step:           0.05
            }
            ListElement {
                title:          qsTr("Differential Gain (RATE_D_Z)")
                description:    qsTr("Damping: increase to reduce overshoot and shaking, but not higher than needed.")
                param:          "RATE_D_Z"
                min:            0
                max:            0.005
                step:           0.0005
            }
            ListElement {
                title:          qsTr("Integral Gain (RATE_I_Z)")
                description:    qsTr("Removes a steady offset. Reduce it when you see slow oscillations.")
                param:          "RATE_I_Z"
                min:            0
                max:            0.3
                step:           0.02
            }
        }
    }

    RowLayout {
        spacing: ScreenTools.defaultFontPixelWidth

        QGCLabel {
            text: root._thrustCurve ? qsTr("Thrust Curve (0 = linear, 1 = quadratic)")
                                    : qsTr("Thrust Curve: the flight controller does not serve MOT_THR_CURVE.")
        }
        FactTextField {
            fact:       root._thrustCurve
            visible:    !!root._thrustCurve
        }
    }

    VibeAILabTuningPage {
        id:                 tuningPage
        availableWidth:     root._availableWidth
        availableHeight:    root._availableHeight - tuningPage.y
        title:              qsTr("Rate")
        unit:               qsTr("deg/s")
        axis:               [ root.roll, root.pitch, root.yaw ]
        chartDisplaySec:    3
    }
}
