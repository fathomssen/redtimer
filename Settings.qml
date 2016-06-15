import QtQuick 2.5
import QtQuick.Layouts 1.2
import QtQuick.Window 2.0

Item {
    height: 300 * Screen.devicePixelRatio
    width: 400 * Screen.devicePixelRatio

    SettingsForm {
        anchors.margins: 5
        anchors.fill: parent
    }
}
