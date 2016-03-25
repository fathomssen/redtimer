import QtQuick 2.5

Item {
    visible: true
    width: 400
    height: 100
    property alias url: settings.url
    property alias apikey: settings.apikey

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
        id: settings;
        anchors.rightMargin: 0
        anchors.bottomMargin: 0
        anchors.leftMargin: 0
        anchors.topMargin: 0
        anchors.fill: parent
    }
}
