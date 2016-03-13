import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2

Item {
    visible: true
    width: 400
    height: 300

    property alias url: form1.url;
    property alias apikey: form1.apikey;

    function getUrl()
    {
        return url.text;
    }

    function getApiKey()
    {
        return apikey.text;
    }

    function setUrl( s )
    {
        url.text = s;
    }

    function setApiKey( s )
    {
        apikey.text = s;
    }

    SettingsForm {
        id: form1;
        anchors.rightMargin: 0
        anchors.bottomMargin: 0
        anchors.leftMargin: 0
        anchors.topMargin: 0
        anchors.fill: parent
    }
}
