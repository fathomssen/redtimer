import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

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
        focus: true
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
            id: useCurrentIssue
            Layout.preferredHeight: 24
            Layout.preferredWidth: 24
            objectName: "useCurrentIssue"
            iconSource: "qrc:/open-iconic/svg/vertical-align-bottom.svg"
            tooltip: "Use the last tracked issue as parent issue"
            enabled: false
        }

        Button {
            id: useCurrentIssueParent
            Layout.preferredHeight: 24
            Layout.preferredWidth: 24
            objectName: "useCurrentIssueParent"
            iconSource: "qrc:/open-iconic/svg/vertical-align-top.svg"
            tooltip: "Use the last tracked issue's parent as parent issue (default)"
            enabled: false
        }

        Button {
            id: selectParentIssue
            Layout.preferredHeight: 24
            Layout.preferredWidth: 24
            objectName: "selectParentIssue"
            iconSource: "qrc:/open-iconic/svg/list.svg"
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
        id: label8
        text: qsTr("Target version")
    }

    ComboBox {
        id: version
        objectName: "version"
        Layout.fillWidth: true
        model: versionModel
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
        Layout.minimumHeight: 100
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
