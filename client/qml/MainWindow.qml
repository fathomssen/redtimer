import QtQuick 2.5

Item {
    width: 270
    height: 400

    signal counterEntered()

    function counterKeyEvent( event )
    {
        if( event.key == Qt.Key_Enter || event.key == Qt.Key_Return )
            event.accept = true
        else
            counterEntered()
    }

    function counterMouseAreaEvent( mouse )
    {
        counterEntered()
        mouse.accepted = false
    }

    Component.onCompleted: {
        mainForm.counter.Keys.pressed.connect( counterKeyEvent )
    }

    Connections {
        target: mainForm.counterMouseArea
        onPressed: { counterMouseAreaEvent(mouse) }
    }

    MainWindowForm {
        id: mainForm

        anchors.margins: 0
        anchors.fill: parent
    }
}
