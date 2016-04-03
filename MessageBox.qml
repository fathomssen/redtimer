import QtQuick 2.4

MessageBoxForm {
    mouseArea.onClicked: { messageBox.visible = false; }
    anchors.right: parent.right
    anchors.left: parent.left
}
