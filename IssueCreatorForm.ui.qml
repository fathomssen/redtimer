import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

GridLayout {
    id: gridLayout1
    columns: 2

    Label {
        id: label6
        text: qsTr("Project")
    }

    ComboBox {
        id: project
        objectName: "project"
        Layout.minimumWidth: 200
        Layout.fillWidth: true
        model: projectModel
        textRole: "name"
    }

    Label {
        id: label1
        text: qsTr("Tracker")
    }

    ComboBox {
        id: tracker
        objectName: "tracker"
        Layout.minimumWidth: 200
        Layout.fillWidth: true
        model: trackerModel
        textRole: "name"
    }

    Label {
        id: label2
        text: qsTr("Subject")
    }

    TextField {
        id: subject
        Layout.fillWidth: true
        objectName: "subject"
        placeholderText: qsTr("")
    }


    Label {
        id: label4
        text: qsTr("Parent issue")
    }

    RowLayout {
        width: 100
        height: 100
        Layout.fillWidth: true

        TextField {
            id: parentIssue
            Layout.fillWidth: true
            objectName: "parentIssue"
            placeholderText: qsTr("")
        }

        ToolButton {
            id: selectParentIssue
            objectName: "selectParentIssue"
            width: 23
            iconSource: "open-iconic/svg/list.svg"
            tooltip: "Select parent issue from list"
        }
    }

    Label {
        id: label5
        text: qsTr("Estimated time")
    }

    TextField {
        id: estimatedTime
        Layout.fillWidth: true
        objectName: "estimatedTime"
        placeholderText: qsTr("")
    }

    Label {
        id: label3
        text: qsTr("Description")
    }

    TextArea {
        id: description
        Layout.minimumHeight: 100
        Layout.fillHeight: true
        Layout.fillWidth: true
        objectName: "description"
    }

    RowLayout {
        id: rowLayout1
        width: 100
        height: 100
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.columnSpan: 2

        Button {
            id: create
            objectName: "create"
            text: qsTr("Create")
            isDefault: true
        }

        Button {
            id: cancel
            objectName: "cancel"
            text: qsTr("Cancel")
        }
    }
}

