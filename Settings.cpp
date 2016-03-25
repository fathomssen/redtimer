#include "Settings.h"

#include "logging.h"

#include <QAbstractButton>
#include <QQuickItem>

using namespace redtimer;
using namespace std;

Settings::Settings( QObject* parent )
    : QObject( parent ),
      settings_( "Thomssen IT", "RedTimer" )
{
    ENTER();

    win_ = new QQuickView();
    win_->setSource( QUrl(QStringLiteral("qrc:/Settings.qml")) );
    win_->setModality( Qt::ApplicationModal );

    item_ = qobject_cast<QQuickItem*>( win_->rootObject() );

    // Connect the cancel button
    connect( item_->findChild<QQuickItem*>("cancel"), SIGNAL(clicked()),
             this, SLOT(close()) );

    // Connect the save button
    connect( item_->findChild<QQuickItem*>("save"), SIGNAL(clicked()),
             this, SLOT(save()) );
}

void
Settings::close()
{
    ENTER();

    if( win_->isVisible() )
    {
        DEBUG() << "Closing settings window";

        QVariant ret;

        QMetaObject::invokeMethod( item_, "setUrl", Q_RETURN_ARG(QVariant, ret), Q_ARG(QVariant, "") );
        QMetaObject::invokeMethod( item_, "setApiKey", Q_RETURN_ARG(QVariant, ret), Q_ARG(QVariant, "") );

        win_->close();
    }
}

void
Settings::display()
{
    ENTER();

    QVariant ret;

    QMetaObject::invokeMethod( item_, "setUrl", Q_RETURN_ARG(QVariant, ret), Q_ARG(QVariant, url_) );
    QMetaObject::invokeMethod( item_, "setApiKey", Q_RETURN_ARG(QVariant, ret), Q_ARG(QVariant, apiKey_) );

    if( !win_->isVisible() )
    {
        DEBUG() << "Displaying settings window";
        win_->show();
    }
}

QString
Settings::getApiKey() const
{
    ENTER();
    RETURN( apiKey_ );
}

QString
Settings::getUrl() const
{
    ENTER();
    RETURN( url_ );
}

void
Settings::load()
{
    ENTER();

    url_ = settings_.value( "url" ).toString();
    apiKey_ = settings_.value( "apikey" ).toString();

    DEBUG() << "Loaded settings from file:";
    DEBUG()(url_)(apiKey_);

    if( url_.isEmpty() || apiKey_.isEmpty() )
        display();
}

void
Settings::save()
{
    ENTER();

    QVariant ret;

    QMetaObject::invokeMethod( item_, "getUrl", Q_RETURN_ARG(QVariant, ret) );
    url_ = ret.toString();
    settings_.setValue( "url", url_ );

    QMetaObject::invokeMethod( item_, "getApiKey", Q_RETURN_ARG(QVariant, ret) );
    apiKey_ = ret.toString();
    settings_.setValue( "apikey", apiKey_ );

    DEBUG() << "Changed settings to";
    DEBUG(url_)(apiKey_);

    DEBUG() << "Emitting saved() signal";
    saved();

    close();
}
