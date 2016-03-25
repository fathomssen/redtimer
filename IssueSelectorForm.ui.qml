import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

Item {
    id: issueSelector
    width: 200
    property alias search: search
    property alias issues: issues
    property alias project: project
    Layout.minimumWidth: 200

    GroupBox {
        id: groupBox1
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
                Layout.fillWidth: true
                anchors.right: parent.right
                anchors.rightMargin: 0
                anchors.left: parent.left
                anchors.leftMargin: 0
                placeholderText: qsTr("Search in issue list")
            }

            ListView {
                id: issues
                objectName: "issues"
                model: issuesModel

                delegate: Item {
                    height: 25
                    width: 100
                    Text { text: modelData }
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
