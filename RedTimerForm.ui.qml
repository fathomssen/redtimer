import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

Item {
    width: 250
    height: 400
    Layout.minimumWidth: 250
    Layout.minimumHeight: 400
    property alias tracker: tracker
    property alias status: status
    property alias activity: activity

    ToolBar {
        id: toolBar1
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top

        ToolButton {
            id: settings
            x: 0
            y: 0
            objectName: "settings"
            text: "&Settings"
            tooltip: "Open Settings"
            iconSource: "open-iconic/svg/wrench.svg"
        }
    }

    SplitView {
        id: splitView1
        anchors.rightMargin: 5
        anchors.leftMargin: 5
        anchors.bottomMargin: 5
        anchors.topMargin: 5
        anchors.top: toolBar1.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        orientation: Qt.Vertical

        Item {
            id: top
            Layout.minimumHeight: 200
            Layout.fillHeight: true

            GroupBox {
                id: groupBox2
                anchors.fill: parent
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                title: qsTr("Issue data")


                ColumnLayout {
                    id: columnLayout4
                    anchors.fill: parent

                    RowLayout {
                        id: rowLayout3
                        Layout.fillWidth: true

                        TextField {
                            id: quickPick
                            Layout.fillWidth: true
                            placeholderText: qsTr("Enter issue number")
                        }

                        ToolButton {
                            id: selectIssue
                            width: 23
                            iconSource: "open-iconic/svg/list.svg"
                            tooltip: "Select issue from list"
                        }
                    }


                    TextArea {
                        id: textArea1
                        height: 50
                        Layout.minimumHeight: 50
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        readOnly: true
                    }


                }
            }
        }

        Item {
            id: bottom
            Layout.minimumHeight: 150

            GroupBox {
                id: groupBox1
                anchors.fill: parent
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                title: qsTr("Tracking")

                ColumnLayout {
                    id: columnLayout3
                    anchors.fill: parent
                    Layout.fillWidth: true


                    RowLayout {
                        id: rowLayout2
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        anchors.fill: parent

                        ToolButton {
                            id: startStop
                            width: 40
                            height: 40
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            iconSource: "open-iconic/svg/media-play.svg"
                            tooltip: "Start time tracking"
                        }

                        TextField {
                            id: time
                            text: "00:00:00"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            readOnly: true
                            placeholderText: qsTr("")
                        }
                    }

                    ComboBox {
                        id: activity
                        Layout.fillWidth: true
                        objectName: "activity"
                        model: activityModel
                    }



                    ComboBox {
                        id: status
                        Layout.fillWidth: true
                        objectName: "status"
                        model: statusModel
                    }


                    ComboBox {
                        id: tracker
                        Layout.fillWidth: true
                        objectName: "tracker"
                        model: trackerModel
                    }


                }

            }
        }
    }

}

