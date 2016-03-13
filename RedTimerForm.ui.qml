import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

Item {
    width: 600
    height: 400

    ToolBar {
        id: toolBar1

        RowLayout {
            id: rowLayout1
            anchors.right: parent.right
            anchors.rightMargin: 488
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.top: parent.top
            anchors.topMargin: 0

            ToolButton {
                id: settings
                objectName: "settings"
                text: "Settings"
                anchors.top: parent.top
                anchors.topMargin: 0
            }
        }
    }

    GridLayout {
        id: gridLayout1
        anchors.top: toolBar1.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.topMargin: 0
        anchors.rightMargin: 5
        anchors.leftMargin: 5
        anchors.bottomMargin: 5


        ColumnLayout {
            id: columnLayout1
            width: 220
            height: 100
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.top: parent.top
            anchors.topMargin: 0

            TextField {
                id: searchText
                anchors.right: parent.right
                anchors.rightMargin: 0
                anchors.left: parent.left
                anchors.leftMargin: 0
                placeholderText: qsTr("Issue ID")
            }

            ListView {
                id: listView1
                y: 0
                height: 160
                anchors.right: parent.right
                anchors.rightMargin: 17
                anchors.left: parent.left
                anchors.leftMargin: 0
                model: ListModel {
                    ListElement {
                        name: "Grey"
                        colorCode: "grey"
                    }

                    ListElement {
                        name: "Red"
                        colorCode: "red"
                    }

                    ListElement {
                        name: "Blue"
                        colorCode: "blue"
                    }

                    ListElement {
                        name: "Green"
                        colorCode: "green"
                    }
                }
                delegate: Item {
                    x: 5
                    width: 80
                    height: 40
                    Row {
                        id: row1
                        Rectangle {
                            width: 40
                            height: 40
                            color: colorCode
                        }

                        Text {
                            text: name
                            anchors.verticalCenter: parent.verticalCenter
                            font.bold: true
                        }
                        spacing: 10
                    }
                }
            }

        }

        ColumnLayout {
            id: columnLayout2
            height: 100
            anchors.right: parent.right
            anchors.rightMargin: -156
            anchors.left: parent.left
            anchors.leftMargin: 220
            anchors.top: parent.top
            anchors.topMargin: 0


            RowLayout {
                id: rowLayout2
                width: 100
                height: 100

                ToolButton {
                    id: startStop
                    width: 40
                    height: 40
                    iconSource: "icons/play.png"
                    tooltip: "Start"
                }

                TextField {
                    id: time
                    text: "00:00:00"
                    readOnly: true
                    placeholderText: qsTr("")
                }
            }
            GroupBox {
                id: groupBox1
                title: qsTr("Properties")

                ColumnLayout {
                    id: columnLayout3


                    ComboBox {
                        id: activity
                        model: ListModel {
                            id: activityItems
                            ListElement { text: "Choose an activity"; color: "Yellow" }
                        }
                    }


                    ComboBox {
                        id: status
                        model: ListModel {
                            id: statusItems
                            ListElement { text: "Choose a status"; color: "Yellow" }
                        }
                    }



                    ComboBox {
                        id: tracker
                        model: ListModel {
                            id: tItems
                            ListElement { text: "Choose a status"; color: "Yellow" }
                        }
                    }

                    Label {
                        id: label1
                        text: qsTr("Add Comment")
                    }

                    RowLayout {
                        id: rowLayout3

                        TextArea {
                            id: comment
                            width: 100
                            height: 50
                        }

                        ToolButton {
                            id: sendComment
                            text: qsTr("Send")
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: 0
                        }
                    }

                }
            }
        }
    }
}

