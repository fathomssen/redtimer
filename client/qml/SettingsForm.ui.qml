import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

ColumnLayout {
    id: columnLayout1

    TabView {
        id: tabView1
        currentIndex: 0
        Layout.fillHeight: true
        Layout.fillWidth: true
        frameVisible: false

        Tab {
            id: tab1
            title: qsTr("Connection")
            active: true

            GridLayout {
                id: gridLayout3
                anchors.fill: parent
                columns: 2

                Item {
                    // Spacer
                    height: 15
                    Layout.columnSpan: 2
                }

                RowLayout {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true

                    Label {
                        id: label9
                        text: qsTr("Profile")
                    }

                    ComboBox {
                        id: profiles
                        objectName: "profiles"
                        model: profilesModel
                        textRole: "name"
                    }

                    Button {
                        id: createProfile
                        objectName: "createProfile"
                        tooltip: "Create new profile"
                        iconSource: "/open-iconic/svg/plus.svg"
                    }

                    Button {
                        id: deleteProfile
                        objectName: "deleteProfile"
                        tooltip: "Delete profile"
                        iconSource: "/open-iconic/svg/trash.svg"
                    }

                    Button {
                        id: renameProfile
                        objectName: "renameProfile"
                        tooltip: "Rename profile"
                        iconSource: "/open-iconic/svg/pencil.svg"
                    }
                }

                Label {
                    id: label1
                    text: qsTr("Redmine connection URL")
                }

                TextField {
                    id: url
                    Layout.fillWidth: true
                    objectName: "url"
                    placeholderText: qsTr("https://redmine.site.url")
                }

                Label {
                    id: label2
                    text: qsTr("Redmine API key")
                }

                TextField {
                    id: apikey
                    Layout.fillWidth: true
                    objectName: "apikey"
                    placeholderText: qsTr("4466f8177f6653e1b179f08fbad36e631f15b316")
                }

                Label {
                    id: label4
                    text: qsTr("Maximum recent issues")
                }

                TextField {
                    id: numRecentIssues
                    Layout.fillWidth: true
                    objectName: "numRecentIssues"
                    placeholderText: qsTr("10 (-1: indefinitely)")
                }

                Label {
                    id: label3
                    text: qsTr("Worked on issue status")
                }

                ComboBox {
                    id: workedOn
                    enabled: false
                    Layout.fillWidth: true
                    objectName: "workedOn"
                    model: issueStatusModel
                    textRole: "name"
                }

                CheckBox {
                    id: useCustomFields
                    objectName: "useCustomFields"
                    text: qsTr("Use custom fields (requires 'redmine_shared_api' plugin)")
                    Layout.columnSpan: 2
                }

                Label {
                    id: label5
                    text: qsTr("Start time field")
                }

                ComboBox {
                    id: startTime
                    enabled: false
                    Layout.fillWidth: true
                    objectName: "startTime"
                    model: startTimeModel
                    textRole: "name"
                }

                Label {
                    id: label6
                    text: qsTr("End time field")
                }

                ComboBox {
                    id: endTime
                    enabled: false
                    Layout.fillWidth: true
                    objectName: "endTime"
                    model: endTimeModel
                    textRole: "name"
                }

                CheckBox {
                    id: ignoreSslErrors
                    objectName: "ignoreSslErrors"
                    text: qsTr("Ignore SSL errors")
                    Layout.columnSpan: 2
                }

                CheckBox {
                    id: checkConnection
                    objectName: "checkConnection"
                    text: qsTr("Check network connection every 5s")
                    Layout.columnSpan: 2
                }

                Item {
                    // Spacer
                    Layout.fillHeight: true
                }
            }
        }

        Tab {
            id: tab3
            title: qsTr("Shortcuts")
            active: true

            GridLayout {
                id: gridLayout2
                anchors.fill: parent
                columns: 2

                Item {
                    // Spacer
                    height: 15
                    Layout.columnSpan: 2
                }

                Label {
                    id: label10
                    text: qsTr("Shortcut to show/hide RedTimer")
                }

                TextField {
                    id: shortcutToggle
                    Layout.fillWidth: true
                    objectName: "shortcutToggle"
                    placeholderText: qsTr("Ctrl+Alt+R")
                }

                Label {
                    id: label11
                    text: qsTr("Shortcut to start/stop RedTimer")
                }

                TextField {
                    id: shortcutStartStop
                    Layout.fillWidth: true
                    objectName: "shortcutStartStop"
                    placeholderText: qsTr("Ctrl+Alt+S")
                }

                Label {
                    id: label7
                    text: qsTr("Shortcut to create an issue")
                }

                TextField {
                    id: shortcutCreateIssue
                    Layout.fillWidth: true
                    objectName: "shortcutCreateIssue"
                    placeholderText: qsTr("Ctrl+Alt+C")
                }

                Label {
                    id: label8
                    text: qsTr("Shortcut to load an issue")
                }

                TextField {
                    id: shortcutSelectIssue
                    Layout.fillWidth: true
                    objectName: "shortcutSelectIssue"
                    placeholderText: qsTr("Ctrl+Alt+L")
                }

                Item {
                    // Spacer
                    Layout.fillHeight: true
                }
            }
        }

        Tab {
            id: tab4
            title: qsTr("Interface")
            active: true

            ColumnLayout {
                id: columnLayout2
                anchors.fill: parent

                Item {
                    // Spacer
                    height: 15
                }

                CheckBox {
                    id: useSystemTrayIcon
                    objectName: "useSystemTrayIcon"
                    text: qsTr("Use system tray icon")
                }

                CheckBox {
                    id: closeToTray
                    objectName: "closeToTray"
                    text: qsTr("Hide window instead of closing it")
                }

                Item {
                    // Spacer
                    Layout.fillHeight: true
                }
            }
        }
    }
    
    RowLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignCenter
        
        Button {
            id: save
            objectName: "save"
            text: qsTr("&Save")
            isDefault: true
        }

        Button {
            id: apply
            objectName: "apply"
            text: qsTr("A&pply")
        }

        Button {
            id: cancel
            objectName: "cancel"
            text: qsTr("&Cancel")
        }
    }
}
