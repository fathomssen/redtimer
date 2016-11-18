import QtQuick 2.5

Item {
    width: 270
    height: 420

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

    function quickPickToggle()
    {
        console.log( "Quick pick" )
    }

    Component.onCompleted: {
        mainForm.counter.Keys.pressed.connect( counterKeyEvent )
    }

    Connections {
        target: mainForm.counterMouseArea
        onPressed: { counterMouseAreaEvent(mouse) }
    }

    Connections {
        target: mainForm.quickPick
        onFocusChanged: {
            if( mainForm.quickPick.focus ) {
                mainForm.quickPick.editText=""
            }
            else {
                mainForm.quickPick.editText="<Enter or select issue>"
            }
        }
    }

    MainWindowForm {
        id: mainForm

        anchors.margins: 0
        anchors.fill: parent
    }
}
