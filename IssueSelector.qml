import QtQuick 2.5
import QtQuick.Layouts 1.2

IssueSelectorForm {
    id: issueSelector // required by issueDelegate

    Component{
        id: issueDelegate

        Item{
            height: 25
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 0
            anchors.rightMargin: 0

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
}
