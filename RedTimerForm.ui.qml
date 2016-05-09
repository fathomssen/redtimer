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
    property alias counter: counter
    property alias issueData: issueData
    property alias issueStatus: issueStatus
    property alias quickPick: quickPick
    property alias selectIssue: selectIssue
    property alias settings: settings
    property alias startStop: startStop

    ToolBar {
        id: toolBar
        height: 32
        objectName: "toolBar"
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top

        ToolButton {
            id: createIssue
            width: 24
            height: 24
            objectName: "createIssue"
            text: "Create &new issue"
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 0
            anchors.left: parent.left
            tooltip: "Create new issue"
            iconSource: "qrc:///open-iconic/svg/plus.svg"
        }

        ToolButton {
            id: reload
            width: 24
            height: 24
            objectName: "reload"
            text: "&Reload"
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: settings.left
            tooltip: "Reload"
            iconSource: "qrc:///open-iconic/svg/reload.svg"
        }

        ToolButton {
            id: settings
            width: 24
            height: 24
            objectName: "settings"
            text: "&Settings"
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            tooltip: "Open Settings"
            iconSource: "qrc:///open-iconic/svg/cog.svg"
        }
    }


    GridLayout {
        id: gridLayout1
        flow: GridLayout.TopToBottom
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
            Layout.fillWidth: true
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

                        ComboBox {
                            id: quickPick
                            objectName: "quickPick"
                            Layout.fillWidth: true
                            editable: true
                            model: recentIssuesModel
                            textRole: "text"
                        }

                        ToolButton {
                            id: selectIssue
                            objectName: "selectIssue"
                            width: 24
                            height: 24
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
            Layout.fillWidth: true
            Layout.minimumHeight: 100

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
                            width: 24
                            height: 24
                            isDefault: true
                            iconSource: "qrc:///open-iconic/svg/media-play.svg"
                            tooltip: "Start time tracking"
                        }

                        TextField {
                            id: counter
                            objectName: "counter"
                            text: "00:00:00"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
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

