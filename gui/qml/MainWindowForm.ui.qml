import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2

Item {
    id: mainForm
    width: 270
    height: 400

    Layout.minimumWidth: 250
    Layout.minimumHeight: 400

    property alias activity: activity
    property alias counter: counter
    property alias counterMouseArea: counterMouseArea
    property alias description: description
    property alias issueStatus: issueStatus
    property alias profiles: profiles
    property alias quickPick: quickPick
    property alias selectIssue: selectIssue
    property alias settings: settings
    property alias startStop: startStop

    ToolBar {
        id: toolBar
        height: 28
        objectName: "toolBar"
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top

        Button {
            id: createIssue
            width: 24
            height: 24
            objectName: "createIssue"
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 0
            anchors.left: parent.left
            tooltip: "Create new issue"
            iconSource: "qrc:/open-iconic/svg/plus.svg"
        }

        Button {
            id: connectionStatus
            width: 24
            height: 24
            objectName: "connectionStatus"
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: reload.left
            tooltip: "Connection status"
            iconSource: "qrc:/open-iconic/svg/signal.svg"
            style: ButtonStyle{ background: Rectangle{
                    id: connectionStatusStyle
                    objectName: "connectionStatusStyle"
                    color: "transparent"
                } }
        }

        Button {
            id: reload
            width: 24
            height: 24
            objectName: "reload"
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: settings.left
            tooltip: "Reload"
            iconSource: "qrc:/open-iconic/svg/reload.svg"
        }

        Button {
            id: settings
            width: 24
            height: 24
            objectName: "settings"
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            tooltip: "Open Settings"
            iconSource: "qrc:/open-iconic/svg/cog.svg"
        }
    }

    ColumnLayout {
        id: columnLayout1
        anchors.rightMargin: 5
        anchors.leftMargin: 5
        anchors.bottomMargin: 5
        anchors.topMargin: 5
        anchors.top: toolBar.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: parent.left

        ComboBox {
            id: profiles
            Layout.fillWidth: true
            objectName: "profiles"
            model: profilesModel
            textRole: "name"
            visible: false // only display when there is more than one profile
        }

        RowLayout {
            id: rowLayout1
            height: 28
            Layout.fillWidth: true

            ComboBox {
                id: quickPick
                objectName: "quickPick"
                Layout.fillWidth: true
                editable: true
                model: recentIssuesModel
                textRole: "text"
            }

            Button {
                id: selectIssue
                Layout.preferredHeight: 24
                Layout.preferredWidth: 24
                objectName: "selectIssue"
                iconSource: "qrc:/open-iconic/svg/list.svg"
                tooltip: "Select issue from list"
            }
        }

        GroupBox {
            id: groupBox1
            Layout.fillHeight: true
            Layout.fillWidth: true
            title: qsTr("Issue data")

            ColumnLayout {
                id: columnLayout2
                anchors.fill: parent

                TextField {
                    id: issueId
                    objectName: "issueId"
                    Layout.fillWidth: true
                    readOnly: true
                    placeholderText: qsTr("ID")
                }

                TextField {
                    id: subject
                    readOnly: true
                    Layout.fillWidth: true
                    objectName: "subject"
                    placeholderText: qsTr("Subject")
                }

                SplitView {
                    id: splitView1
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    orientation: Qt.Vertical

                    TextArea {
                        id: description
                        implicitHeight: 80
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        objectName: "description"
                        readOnly: true
                        verticalScrollBarPolicy: Qt.ScrollBarAsNeeded
                    }

                    TextArea {
                        id: more
                        implicitHeight: 40
                        visible: false
                        objectName: "more"
                        Layout.fillWidth: true
                        readOnly: true
                        textFormat: TextEdit.RichText
                        verticalScrollBarPolicy: Qt.ScrollBarAsNeeded
                    }
                }


            }
        }

        RowLayout {
            id: rowLayout2
            height: 26
            Layout.fillWidth: true

            Button {
                id: startStop
                Layout.preferredHeight: 24
                Layout.preferredWidth: 24
                objectName: "startStop"
                isDefault: true
                iconSource: "qrc:/open-iconic/svg/media-play.svg"
                tooltip: "Start time tracking"
                focus: true
                activeFocusOnPress: true
            }

            TextField {
                id: counter
                objectName: "counter"
                text: "00:00:00"
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.fillWidth: true
                
                MouseArea {
                    id: counterMouseArea
                    anchors.fill: parent
                }
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

