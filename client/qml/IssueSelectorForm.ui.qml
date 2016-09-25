import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

ColumnLayout {
    id: columnLayout1

    property alias issues: issues
    property alias project: project
    property alias search: search
    property alias assignee: assignee

    ComboBox {
        id: project
        Layout.fillWidth: true
        objectName: "project"
        model: projectModel
        textRole: "name"
    }

    ComboBox {
        id: assignee
        Layout.fillWidth: true
        objectName: "assignee"
        model: assigneeModel
        textRole: "name"
    }

    ComboBox {
        id: version
        Layout.fillWidth: true
        objectName: "version"
        model: versionModel
        textRole: "name"
    }

    TextField {
        id: search
        objectName: "search"
        Layout.fillWidth: true
        placeholderText: qsTr("Search in issue list")
        focus: true
    }

    ListView {
        id: issues
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

        Layout.minimumHeight: 300
        Layout.fillWidth: true
        Layout.fillHeight: true
    }



}

