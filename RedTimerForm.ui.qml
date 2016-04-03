import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

Item {
    id: redTimer
    objectName: "redTimer"
    width: 250
    height: 400
    Layout.minimumWidth: 250
    Layout.minimumHeight: 400

    property alias activity: activity
    property alias issueData: issueData
    property alias quickPick: quickPick
    property alias selectIssue: selectIssue
    property alias settings: settings
    property alias startStop: startStop
    property alias issueStatus: issueStatus
    property alias counter: counter

    ToolBar {
        id: toolBar
        objectName: "toolBar"
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top

        ToolButton {
            id: settings
            objectName: "settings"
            text: "&Settings"
            anchors.right: parent.right
            tooltip: "Open Settings"
            iconSource: "qrc:///open-iconic/svg/cog.svg"
        }

        ToolButton {
            id: reload
            objectName: "reload"
            text: "&Reload"
            anchors.right: settings.left
            tooltip: "Reload"
            iconSource: "qrc:///open-iconic/svg/reload.svg"
        }
    }


    SplitView {
        id: splitView1
        orientation: Qt.Vertical
        anchors.rightMargin: 5
        anchors.leftMargin: 5
        anchors.bottomMargin: 5
        anchors.topMargin: 5
        anchors.top: toolBar.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: parent.left

        Item {
            id: item1
            Layout.fillHeight: true
            Layout.minimumHeight: 100

            GroupBox {
                id: groupBox1
                anchors.fill: parent
                title: qsTr("Issue")


                ColumnLayout {
                    id: columnLayout2
                    anchors.fill: parent

                    RowLayout {
                        id: rowLayout1
                        Layout.fillWidth: true

                        TextField {
                            id: quickPick
                            objectName: "quickPick"
                            Layout.fillWidth: true
                            placeholderText: qsTr("Enter issue number")
                        }

                        ToolButton {
                            id: selectIssue
                            objectName: "selectIssue"
                            width: 23
                            iconSource: "qrc:///open-iconic/svg/list.svg"
                            tooltip: "Select issue from list"
                        }
                    }


                    TextArea {
                        id: issueData
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        objectName: "issueData"
                        readOnly: true
                    }


                }
            }
        }

        Item {
            id: item2
            height: 100
            Layout.minimumHeight: 100
            Layout.maximumHeight: 100

            GroupBox {
                id: groupBox2
                anchors.fill: parent
                title: qsTr("Tracking")

                ColumnLayout {
                    id: columnLayout3
                    anchors.fill: parent

                    RowLayout {
                        id: rowLayout2
                        Layout.fillWidth: true

                        ToolButton {
                            id: startStop
                            objectName: "startStop"
                            width: 40
                            height: 40
                            isDefault: true
                            iconSource: "qrc:///open-iconic/svg/media-play.svg"
                            tooltip: "Start time tracking"
                        }

                        TextField {
                            id: counter
                            objectName: "counter"
                            text: "00:00:00"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            readOnly: true
                        }
                    }

                    ComboBox {
                        id: activity
                        Layout.fillWidth: true
                        objectName: "activity"
                        model: activityModel
                        textRole: "name"
                    }



                    ComboBox {
                        id: issueStatus
                        Layout.fillWidth: true
                        objectName: "issueStatus"
                        model: issueStatusModel
                        textRole: "name"
                    }




                }
            }
        }
    }



}

