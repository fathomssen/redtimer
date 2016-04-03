import QtQuick 2.5
import QtQuick.Layouts 1.2

Item {
    width: 200
    height: 400

    Component{
        id: issueDelegate

        Item{
            height: 25
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 0

            Text {
                text: subject + " (#" + id + ")"
                verticalAlignment: Text.AlignVCenter
            }

            MouseArea {
                anchors.fill: parent
                onClicked: issueSelector.issues.currentIndex = index
                onDoubleClicked: issueSelector.issues.activated( index )
            }
        }
    }

    IssueSelectorForm {
        id: issueSelector // required by issueDelegate

        anchors.rightMargin: 0
        anchors.bottomMargin: 0
        anchors.leftMargin: 0
        anchors.topMargin: 0
        anchors.fill: parent
    }
}
