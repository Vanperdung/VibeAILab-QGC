import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.FactControls

/// PID Tuning page: one tab per FC controller.
SetupPage {
    pageComponent: pageComponent

    Component {
        id: pageComponent

        ColumnLayout {
            id:         root
            spacing:    ScreenTools.defaultFontPixelWidth / 4

            readonly property real _availableWidth:     availableWidth
            readonly property real _availableHeight:    availableHeight

            // Also used by PIDTuning and the tab pages, which reach it by id
            FactPanelController {
                id: controller
            }

            QGCTabBar {
                id: tabBar

                Repeater {
                    model: tabModel

                    QGCTabButton {
                        objectName: "pidTuning_tab_" + buttonText.replace(/ /g, "")
                        text:       buttonText
                    }
                }
            }

            ListModel {
                id: tabModel

                ListElement {
                    buttonText: qsTr("Rate Controller")
                    tuningPage: "VibeAILabTuningRate.qml"
                }
                ListElement {
                    buttonText: qsTr("Attitude Controller")
                    tuningPage: "VibeAILabTuningAttitude.qml"
                }
            }

            Loader {
                id:     loader
                source: tabModel.get(tabBar.currentIndex).tuningPage

                // Read by the loaded tab page
                property real availableWidth:   root._availableWidth
                property real availableHeight:  root._availableHeight - loader.y
            }
        }
    }
}
