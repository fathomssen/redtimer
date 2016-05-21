import QtQuick 2.5
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

ColumnLayout {
    id: columnLayout1
    width: 200

    property alias issues: issues
    property alias project: project
    property alias search: search

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
        height: 300
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
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
    }
}

