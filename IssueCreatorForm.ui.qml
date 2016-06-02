import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import QtQuick.Window 2.0

GridLayout {
    id: maingrid
    objectName: "maingrid"
    columns: 2

    Label {
        id: label6
        text: qsTr("Project")
    }

    ComboBox {
        id: project
        objectName: "project"
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
        Layout.fillWidth: true
        model: trackerModel
        textRole: "name"
        enabled: false
    }

    Label {
        id: label2
        text: qsTr("Subject")
    }

    TextField {
        id: subject
        Layout.fillWidth: true
        objectName: "subject"
        enabled: false
    }

    Label {
        id: label4
        text: qsTr("Parent issue")
    }

    RowLayout {
        Layout.fillWidth: true

        TextField {
            id: parentIssue
            Layout.fillWidth: true
            objectName: "parentIssue"
            enabled: false
        }

        Button {
            id: selectParentIssue
            objectName: "selectParentIssue"
            iconSource: "open-iconic/svg/list.svg"
            tooltip: "Select parent issue from list"
            enabled: false
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
        enabled: false
    }

    Label {
        id: label7
        text: qsTr("Category")
    }

    ComboBox {
        id: category
        objectName: "category"
        Layout.fillWidth: true
        model: categoryModel
        textRole: "name"
        enabled: false
    }

    Label {
        id: customFields
        objectName: "customFields"
        text: qsTr("Custom fields")
        visible: false
        Layout.columnSpan: 2
        font.underline: true
    }

    Label {
        id: label3
        text: qsTr("Description")
    }

    TextArea {
        id: description
        Layout.minimumHeight: 100 * Screen.devicePixelRatio
        Layout.fillHeight: true
        Layout.fillWidth: true
        objectName: "description"
        enabled: false
    }

    RowLayout {
        id: rowLayout1
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.columnSpan: 2

        Button {
            id: create
            objectName: "create"
            text: qsTr("Create")
            enabled: false
            isDefault: true
        }

        Button {
            id: cancel
            objectName: "cancel"
            text: qsTr("Cancel")
        }
    }
}
