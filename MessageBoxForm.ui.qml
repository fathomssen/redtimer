import QtQuick 2.4
import QtQuick.Layouts 1.1

Item {
    width: 200
    height: 30
    anchors.right: parent.right
    anchors.left: parent.left
    Layout.fillWidth: true

    Rectangle {
        id: messageBox
        radius: 1
        border.width: 1
        anchors.fill: parent
        objectName: "messageBox"
        z: 100
        Layout.fillWidth: true

        Text {
            id: errorText
            objectName: "message"
            text: qsTr("MESSAGE")
            anchors.fill: parent
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            font.pixelSize: 10
        }

        MouseArea {
            id: mouseArea
            objectName: "mouseArea"
            anchors.fill: parent
            onClicked: {parent.visible = false}
        }
    }

}
