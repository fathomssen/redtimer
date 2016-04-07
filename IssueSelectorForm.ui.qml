import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

Item {
    width: 200
    height: 400
    Layout.minimumWidth: 200
    Layout.minimumHeight: 400

    property alias issues: issues
    property alias project: project
    property alias search: search

    GroupBox {
        id: groupBox1
        anchors.rightMargin: 5
        anchors.leftMargin: 5
        anchors.bottomMargin: 5
        anchors.topMargin: 5
        anchors.fill: parent
        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        Layout.fillHeight: true
        Layout.fillWidth: true
        title: qsTr("Issues")

        ColumnLayout {
            id: columnLayout1
            anchors.fill: parent

            ComboBox {
                id: project
                Layout.fillWidth: true
                objectName: "project"
                model: projectModel
                textRole: "name"
            }

            TextField {
                id: search
                objectName: "search"
                Layout.fillWidth: true
                anchors.right: parent.right
                anchors.rightMargin: 0
                anchors.left: parent.left
                anchors.leftMargin: 0
                placeholderText: qsTr("Search in issue list")
            }

            ListView {
                id: issues
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                objectName: "issues"
                model: issuesModel

                signal activated( int index )

                delegate: issueDelegate

                highlight: Rectangle {
                    color: "lightsteelblue";
                    radius: 5
                }

                Layout.fillWidth: true
                Layout.fillHeight: true
                anchors.right: parent.right
                anchors.rightMargin: 17
                anchors.left: parent.left
                anchors.leftMargin: 0
            }

        }
    }
}
