import QtQuick 2.5
import QtQuick.Layouts 1.2
import QtQuick.Window 2.0

Item {
    height: 400 * Screen.devicePixelRatio
    width: 200 * Screen.devicePixelRatio

    Component{
        id: issueDelegate

        Item{
            height: 25 * Screen.devicePixelRatio
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

    IssueSelectorForm {
        id: issueSelector // required by issueDelegate
        anchors.margins: 5
        anchors.fill: parent
   }
}
