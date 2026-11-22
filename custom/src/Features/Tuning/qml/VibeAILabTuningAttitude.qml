import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls

/// Attitude tab of the PID Tuning page: the Attitude Controller gains.
ColumnLayout {
    id: root

    // availableWidth and availableHeight come from the tab Loader
    readonly property real _availableWidth:     availableWidth
    readonly property real _availableHeight:    availableHeight
    readonly property var  _vehicle:            globals.activeVehicle

    // Slider ranges are around the values the FC was built with (roll/pitch P 3.0, yaw P 4.0)
    property var roll: QtObject {
        property string name: qsTr("Roll")
        property var plot: [
            { name: qsTr("Response"), value: root._vehicle ? root._vehicle.roll.value : NaN },
            { name: qsTr("Setpoint"), value: root._vehicle ? root._vehicle.setpoint.roll.value : NaN }
        ]
        property var params: ListModel {
            ListElement {
                title:          qsTr("Proportional Gain (ATT_P_X)")
                description:    qsTr("Increase for a faster angle response, reduce if the angle overshoots.")
                param:          "ATT_P_X"
                min:            1
                max:            10
                step:           0.5
            }
        }
    }

    property var pitch: QtObject {
        property string name: qsTr("Pitch")
        property var plot: [
            { name: qsTr("Response"), value: root._vehicle ? root._vehicle.pitch.value : NaN },
            { name: qsTr("Setpoint"), value: root._vehicle ? root._vehicle.setpoint.pitch.value : NaN }
        ]
        property var params: ListModel {
            ListElement {
                title:          qsTr("Proportional Gain (ATT_P_Y)")
                description:    qsTr("Increase for a faster angle response, reduce if the angle overshoots.")
                param:          "ATT_P_Y"
                min:            1
                max:            10
                step:           0.5
            }
        }
    }

    property var yaw: QtObject {
        property string name: qsTr("Yaw")
        property var plot: [
            { name: qsTr("Response"), value: root._vehicle ? root._vehicle.heading.value : NaN },
            { name: qsTr("Setpoint"), value: root._vehicle ? root._vehicle.setpoint.yaw.value : NaN }
        ]
        property var params: ListModel {
            ListElement {
                title:          qsTr("Proportional Gain (ATT_P_Z)")
                description:    qsTr("Increase for a faster heading response, reduce if the heading overshoots.")
                param:          "ATT_P_Z"
                min:            1
                max:            10
                step:           0.5
            }
        }
    }

    VibeAILabTuningPage {
        id:                 tuningPage
        availableWidth:     root._availableWidth
        availableHeight:    root._availableHeight - tuningPage.y
        title:              qsTr("Attitude")
        unit:               qsTr("deg")
        axis:               [ root.roll, root.pitch, root.yaw ]
    }
}
