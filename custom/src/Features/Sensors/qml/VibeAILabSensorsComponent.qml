import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls

/// Sensors page: runs the Gyro Calibration or the Accel Calibration, picked by the selected section.
SetupPage {
    pageComponent: pageComponent

    Component {
        id: pageComponent

        Item {
            width:  availableWidth
            height: availableHeight

            /// Set by SetupPage. The ids are the ones of VibeAILabSensorsComponent::sectionIds().
            property string sectionIdFilter: ""

            readonly property string _gyroHelp:  qsTr("For Gyroscope calibration, place the vehicle on a surface and leave it still.")
            readonly property string _accelHelp: qsTr("For Accelerometer calibration, place the vehicle on each of its six sides, in any order, and hold it still on each side until it shows Completed.")

            // Navigation is blocked while a calibration is active, so the selected one is also the active one
            readonly property bool _gyroSelected:   sectionIdFilter !== "Accelerometer"
            readonly property var  _calibration:    _gyroSelected ? controller.gyro : controller.accel

            readonly property var _accelSides: [
                { side: VibeAILabAccelCalibration.Side.RightSideUp,   name: "rightSideUp",   image: "qrc:///qmlimages/VehicleDown.png" },
                { side: VibeAILabAccelCalibration.Side.UpsideDown,    name: "upsideDown",    image: "qrc:///qmlimages/VehicleUpsideDown.png" },
                { side: VibeAILabAccelCalibration.Side.NoseDown,      name: "noseDown",      image: "qrc:///qmlimages/VehicleNoseDown.png" },
                { side: VibeAILabAccelCalibration.Side.TailDown,      name: "tailDown",      image: "qrc:///qmlimages/VehicleTailDown.png" },
                { side: VibeAILabAccelCalibration.Side.LeftSideDown,  name: "leftSideDown",  image: "qrc:///qmlimages/VehicleLeft.png" },
                { side: VibeAILabAccelCalibration.Side.RightSideDown, name: "rightSideDown", image: "qrc:///qmlimages/VehicleRight.png" }
            ]

            readonly property int _accelDoneSideCount: controller.accel.sideStates.filter(
                sideState => sideState === VibeAILabAccelCalibration.SideState.Done).length

            // The gyro is calibrated in one pose only, shown by the "right side up" tile
            readonly property int _gyroTileState: {
                if (_calibration.runGoingOn) {
                    return VehicleRotationCal.CalState.InProgress
                }
                return _calibration.result === VibeAILabCalibration.Result.Done
                        ? VehicleRotationCal.CalState.Completed
                        : VehicleRotationCal.CalState.Idle
            }

            function _accelTileState(sideState) {
                switch (sideState) {
                case VibeAILabAccelCalibration.SideState.NotDone:
                    return VehicleRotationCal.CalState.Incomplete
                case VibeAILabAccelCalibration.SideState.InProgress:
                    return VehicleRotationCal.CalState.InProgress
                case VibeAILabAccelCalibration.SideState.Done:
                    return VehicleRotationCal.CalState.Completed
                default:
                    return VehicleRotationCal.CalState.Idle
                }
            }

            // A result belongs to the calibration that was just run; do not show it again on a later visit
            onSectionIdFilterChanged: {
                controller.gyro.clearResult()
                controller.accel.clearResult()
            }

            Component.onDestruction: globals.navigationBlockedReason = ""

            QGCPalette { id: qgcPal; colorGroupEnabled: true }

            // Start, Restart and Cancel are disabled while controller.commandPending: one command at a time
            VibeAILabSensorsController {
                id: controller

                onCalibrationActiveChanged: {
                    globals.navigationBlockedReason = calibrationActive
                            ? qsTr("Complete or cancel the current calibration first")
                            : ""
                }
            }

            ColumnLayout {
                anchors.fill:   parent
                spacing:        ScreenTools.defaultFontPixelHeight / 2

                QGCButton {
                    objectName:         "vibeailabSensors_start"
                    Layout.fillWidth:   true
                    visible:            !_calibration.active
                    enabled:            !controller.commandPending
                    text:               _gyroSelected ? qsTr("Calibrate Gyroscope") : qsTr("Calibrate Accelerometer")
                    onClicked:          _gyroSelected ? controller.startGyroCalibration() : controller.startAccelCalibration()
                }

                RowLayout {
                    Layout.fillWidth:   true
                    visible:            _calibration.active
                    spacing:            ScreenTools.defaultFontPixelWidth

                    // The FC reports no progress for the gyro
                    ProgressBar {
                        objectName:         "vibeailabSensors_progressBar"
                        Layout.fillWidth:   true
                        indeterminate:      _gyroSelected
                        value:              _accelDoneSideCount / _accelSides.length
                    }

                    QGCButton {
                        objectName: "vibeailabSensors_restart"
                        text:       qsTr("Restart")
                        visible:    !_gyroSelected
                        enabled:    !controller.commandPending
                        onClicked:  controller.startAccelCalibration()
                    }

                    QGCButton {
                        objectName: "vibeailabSensors_cancel"
                        text:       qsTr("Cancel")
                        enabled:    !controller.commandPending
                        onClicked:  controller.cancelCalibration()
                    }
                }

                Rectangle {
                    Layout.fillWidth:   true
                    Layout.fillHeight:  true
                    color:              qgcPal.windowShade

                    QGCLabel {
                        id:                 statusLabel
                        objectName:         "vibeailabSensors_status"
                        anchors.margins:    ScreenTools.defaultFontPixelWidth
                        anchors.top:        parent.top
                        anchors.left:       parent.left
                        anchors.right:      parent.right
                        wrapMode:           Text.WordWrap
                        font.pointSize:     ScreenTools.mediumFontPointSize
                        text:               _calibration.statusText !== "" ? _calibration.statusText
                                                                           : (_gyroSelected ? _gyroHelp : _accelHelp)
                    }

                    Flow {
                        id:                 tileFlow
                        anchors.margins:    ScreenTools.defaultFontPixelWidth
                        anchors.top:        statusLabel.bottom
                        anchors.bottom:     parent.bottom
                        anchors.left:       parent.left
                        anchors.right:      parent.right
                        spacing:            ScreenTools.defaultFontPixelWidth / 2

                        readonly property real tileWidth:   (width / 3) - (spacing * 2)
                        readonly property real tileHeight:  (height / 2) - spacing

                        Repeater {
                            model: _accelSides

                            VehicleRotationCal {
                                required property var modelData

                                objectName:     "vibeailabSensors_" + modelData.name
                                width:          tileFlow.tileWidth
                                height:         tileFlow.tileHeight
                                visible:        !_gyroSelected || modelData.side === VibeAILabAccelCalibration.Side.RightSideUp
                                calState:       _gyroSelected ? _gyroTileState
                                                              : _accelTileState(controller.accel.sideStates[modelData.side])
                                imageSource:    modelData.image
                            }
                        }
                    }
                }
            }
        }
    }
}
