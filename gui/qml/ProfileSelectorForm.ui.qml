import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

ColumnLayout {
    id: columnLayout1

    property alias profile: profile

    ComboBox {
        id: profile
        Layout.fillWidth: true
        objectName: "profile"
        model: profileModel
        textRole: "name"
    }

    RowLayout {
        id: rowLayout1
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.columnSpan: 2

        Button {
            id: ok
            objectName: "ok"
            text: qsTr("OK")
            isDefault: true
        }

        Button {
            id: cancel
            objectName: "cancel"
            text: qsTr("Cancel")
        }
    }
}

