/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Controls
import QtQuick.Window

import QGroundControl
import QGroundControl.Controllers
import QGroundControl.Controls
import QGroundControl.FactSystem
import QGroundControl.FlightDisplay
import QGroundControl.FlightMap
import QGroundControl.Palette
import QGroundControl.ScreenTools
import QGroundControl.Vehicle

import GeoWork

Item {
    id: airmobisRoot

    // These mirror the stock layer API expected by FlyView.qml
    property var parentToolInsets               // Provided by parent.
    property var totalToolInsets: airmobisRootToolInsets   // Overlay exposes its insets back to parent.
    property var mapControl                     // Provided by parent.

    // Pass-through insets object
    QGCToolInsets {
        id: airmobisRootToolInsets
        leftEdgeTopInset: airmobisRoot.parentToolInsets.leftEdgeTopInset
        leftEdgeCenterInset: airmobisRoot.parentToolInsets.leftEdgeCenterInset
        leftEdgeBottomInset: airmobisRoot.parentToolInsets.leftEdgeBottomInset
        rightEdgeTopInset: airmobisRoot.parentToolInsets.rightEdgeTopInset
        rightEdgeCenterInset: airmobisRoot.parentToolInsets.rightEdgeCenterInset
        rightEdgeBottomInset: airmobisRoot.parentToolInsets.rightEdgeBottomInset
        topEdgeLeftInset: airmobisRoot.parentToolInsets.topEdgeLeftInset
        topEdgeCenterInset: airmobisRoot.parentToolInsets.topEdgeCenterInset
        topEdgeRightInset: airmobisRoot.parentToolInsets.topEdgeRightInset
        bottomEdgeLeftInset: airmobisRoot.parentToolInsets.bottomEdgeLeftInset
        bottomEdgeCenterInset: airmobisRoot.parentToolInsets.bottomEdgeCenterInset
        bottomEdgeRightInset: airmobisRoot.parentToolInsets.bottomEdgeRightInset
    }

    Component.onCompleted: {
        console.log("[geowork][qml] overlay completed, attempting autoBindVideo");
        if (typeof GeoWork !== 'undefined')
            GeoWork.autoBindVideo();
        try {
            console.log("[geowork][qml] GeoWork object:", GeoWork);
            console.log("[geowork][qml] tokenStatus:", GeoWork.tokenStatus, "deviceName:", GeoWork.deviceName);
        } catch (e) {
            console.warn("[geowork][qml] GeoWork access failed:", e);
        }
        try {
            var v = QGroundControl.multiVehicleManager.activeVehicle;
            console.log("[geowork][qml] activeVehicle exists?", !!v);
            if (v && v.gps && v.gps.count)
                console.log("[geowork][qml] sats:", v.gps.count.rawValue);
        } catch (e2) {
            console.warn("[geowork][qml] vehicle access failed:", e2);
        }
    }

    Rectangle {
        x: 8
        y: 8
        width: 20
        height: 20
        color: "red"
    }

    // ---- Loader (required) – settings panel is loaded by qrc path ----
    Loader {
        id: airmobisRootGeoPanel
        anchors.fill: airmobisRoot
        asynchronous: false
        source: "qrc:/Custom/GeoWork/GeoWorkSettingsPanel.qml"
        onStatusChanged: {
            console.log("[geowork][qml] loader status:", status);
            if (status === Loader.Ready && item) {
                // item.anchors.fill = airmobisRoot;
                item.visible = false;
                console.log("[GeoWork] Settings panel loaded");
            } else if (status === Loader.Error) {
                console.warn("[geowork][qml] settings load ERROR; status:", status, "source:", source);
            }
        }
    }

    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: GeoWork.reportLocation()
    }

    // ---- Background fetch every 5 minutes ----
    Timer {
        id: airmobisRootGeoworkPoll
        interval: 5 * 60 * 1000
        repeat: true
        running: true
        triggeredOnStart: false
        onTriggered: {
            if (GeoWork.tokenStatus === GeoWork.Valid) {
                var nm = GeoWork.deviceName && GeoWork.deviceName.length ? GeoWork.deviceName : "BLUE001";
                GeoWork.checkActiveTaskAndFetchState(nm);
            }
        }
    }

    // ---- Right/middle Geowork control pod (2 buttons, semi-transparent) ----
    Rectangle {
        id: airmobisRootGeoworkPod
        width: ScreenTools.defaultFontPixelWidth * 20
        height: ScreenTools.defaultFontPixelHeight * 7
        radius: 10
        color: "#66000000"    // semi-transparent
        border.width: 3
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: ScreenTools.defaultFontPixelWidth
        z: 50

        Column {
            anchors.fill: parent
            anchors.margins: ScreenTools.defaultFontPixelWidth
            spacing: ScreenTools.defaultFontPixelHeight * 0.6

            // ---- Settings button with OK/BAD icon ----
            Button {
                id: airmobisRootGeoworkPodColumnSettingsButton
                width: parent.width - (ScreenTools.defaultFontPixelWidth * 2)
                implicitHeight: ScreenTools.defaultFontPixelHeight * 2.2
                background: Rectangle {
                    radius: 6
                    color: "#444444"      // darker grey
                    opacity: 0.95
                }
                contentItem: Row {
                    spacing: ScreenTools.defaultFontPixelWidth * 0.6
                    anchors.verticalCenter: parent.verticalCenter
                    Image {
                        id: settingsIcon
                        // Provide these SVGs in custom.qrc under prefix custom/img
                        source: (function () {
                                try {
                                    return GeoWork.tokenStatus === GeoWork.Valid ? "qrc:/custom/img/setting_OK.svg" : "qrc:/custom/img/setting_BAD.svg";
                                } catch (e) {
                                    console.warn("[geowork][qml] settings icon binding error:", e);
                                    return "qrc:/custom/img/setting_BAD.svg";
                                }
                            })()
                        fillMode: Image.PreserveAspectFit
                        width: ScreenTools.defaultFontPixelHeight * 1.4
                        height: width
                    }
                    Text {
                        text: "Settings"
                        color: "white"
                        font.pixelSize: ScreenTools.defaultFontPixelHeight * 1.2
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                onClicked: if (airmobisRootGeoPanel.item)
                    airmobisRootGeoPanel.item.open()
            }

            // ---- Create Marker button with stateful behavior ----
            Item {
                id: airmobisRootGeoworkPodColumnCreateButton
                width: parent.width - (ScreenTools.defaultFontPixelWidth * 2)
                height: ScreenTools.defaultFontPixelHeight * 2.6

                // Vehicle access (like your working baseline ~lines 140-146)
                readonly property var vehicle: QGroundControl.multiVehicleManager.activeVehicle
                readonly property bool connected: !!vehicle
                readonly property int sats: connected ? vehicle.gps.count.rawValue : 0

                // GeoWork state
                readonly property bool hasToken: GeoWork.tokenStatus === GeoWork.Valid
                readonly property bool taskActive: GeoWork.stateId && GeoWork.stateId.length > 0

                // Modes
                readonly property bool modeTransparent: !hasToken
                readonly property bool modeActive: hasToken && connected && sats >= 3 && taskActive
                readonly property bool modeOff: hasToken && (!taskActive || !connected || sats < 3)

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    opacity: airmobisRootGeoworkPodColumnCreateButton.modeTransparent ? 0.20 : 1.0

                    Image {
                        id: cmIcon
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        width: ScreenTools.defaultFontPixelHeight * 3
                        height: width
                        visible: !airmobisRootGeoworkPodColumnCreateButton.modeTransparent
                        // You already have icon-active.svg and icon-off.svg in custom/img
                        source: airmobisRootGeoworkPodColumnCreateButton.modeActive ? "qrc:/custom/img/icon-active.svg" : "qrc:/custom/img/icon-off.svg"
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: !airmobisRootGeoworkPodColumnCreateButton.modeTransparent
                        onClicked: {
                            if (airmobisRootGeoworkPodColumnCreateButton.modeActive) {
                                GeoWork.createMarker();
                            } else if (airmobisRootGeoworkPodColumnCreateButton.modeOff) {
                                // Re-fetch to check if a task started meanwhile
                                var nm = GeoWork.deviceName && GeoWork.deviceName.length ? GeoWork.deviceName : "BLUE001";
                                GeoWork.checkActiveTaskAndFetchState(nm);
                            }
                        }
                    }
                }
            }
        }
    }

    Timer {
        id: airmobisRootGeoworkHeartbeat
        interval: 3000
        running: true
        repeat: true
        onTriggered: console.log("[geowork][qml] heartbeat; has GeoWork:", typeof GeoWork !== 'undefined')
    }
}
