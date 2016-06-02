import QtQuick 2.5
import QtQuick.Window 2.0

Item {
   height: 300 * Screen.devicePixelRatio
   width: 400 * Screen.devicePixelRatio

   IssueCreatorForm {
        anchors.margins: 5
        anchors.fill: parent
    }
}