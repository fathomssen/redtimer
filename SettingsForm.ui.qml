import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

GridLayout {
	id: gridLayout1
	columns: 2
	width: 400
	Layout.minimumWidth: 400

	Label {
		id: label1
		text: qsTr("Redmine Connection URL")
	}

	TextField {
		id: url
		Layout.fillWidth: true
		objectName: "url"
		placeholderText: qsTr("Redmine Connection URL")
	}

	Label {
		id: label2
		text: qsTr("Redmine API Key")
	}

	TextField {
		id: apikey
		Layout.fillWidth: true
		objectName: "apikey"
		placeholderText: qsTr("Redmine API Key")
	}

	Label {
		id: label4
		text: qsTr("Maximum recent issues")
	}

	TextField {
		id: numRecentIssues
		Layout.fillWidth: true
		objectName: "numRecentIssues"
		placeholderText: qsTr("Maximum number of recently opened issues (-1: indefinitely)")
	}

	Label {
		id: label3
		text: qsTr("Worked on issue status")
	}

	ComboBox {
		id: workedOn
		Layout.fillWidth: true
		objectName: "workedOn"
		model: issueStatusModel
		textRole: "name"
	}

	Label {
		id: label5
		text: qsTr("Shortcut to show/hide RedTimer")
	}

	TextField {
		id: shortcutToggle
		Layout.fillWidth: true
		objectName: "shortcutToggle"
		placeholderText: qsTr("Ctrl+Alt+R")
	}

	Label {
		id: label6
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

	CheckBox {
		id: useSystemTrayIcon
		objectName: "useSystemTrayIcon"
		text: qsTr("Use system tray icon")
	}

	CheckBox {
		id: ignoreSslErrors
		objectName: "ignoreSslErrors"
		text: qsTr("Ignore SSL errors")
	}

	CheckBox {
		id: checkConnection
		objectName: "checkConnection"
		text: qsTr("Check network connection every 5s")
		Layout.columnSpan: 2
	}

	ToolButton {
		id: apply
		objectName: "apply"
		text: qsTr("A&pply")
		Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
		isDefault: true
	}

	ToolButton {
		id: cancel
		objectName: "cancel"
		text: qsTr("&Cancel")
		Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
	}
}

