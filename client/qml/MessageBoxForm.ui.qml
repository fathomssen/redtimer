import QtQuick 2.5
import QtQuick.Layouts 1.2

Item {
    width: 200
    height: 30
    property alias messageBox: messageBox
    property alias mouseArea: mouseArea
    Layout.fillWidth: true

    Rectangle {
        id: messageBox
        radius: 1
        anchors.fill: parent
        border.width: 1
        objectName: "messageBox"
        z: 100
        Layout.fillWidth: true

        Text {
            id: errorText
            objectName: "message"
            text: qsTr("MESSAGE")
            anchors.rightMargin: 5
            anchors.leftMargin: 5
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            font.pixelSize: 10
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
        }
    }

}
