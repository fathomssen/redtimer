import QtQuick 2.5

Item {
    width: 250
    height: 400

    MainWindowForm {
        objectName: "redTimer"

        signal counterAnyKeyPressed()

        function counterKeyEvent( event )
        {
            if( event.key == Qt.Key_Enter || event.key == Qt.Key_Return )
                event.accept = true
            else
                counterAnyKeyPressed()
        }

        Component.onCompleted: counter.Keys.pressed.connect( counterKeyEvent )

        anchors.rightMargin: 0
        anchors.bottomMargin: 0
        anchors.leftMargin: 0
        anchors.topMargin: 0
        anchors.fill: parent
    }
}
