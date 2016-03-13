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
  qEnter();

  win_ = new QQuickView();
  win_->setSource( QUrl(QStringLiteral("qrc:/Settings.qml")) );
  win_->setModality( Qt::ApplicationModal );

  item_ = qobject_cast<QQuickItem*>( win_->rootObject() );

  // Connect the cancel button
  connect( win_->rootObject()->findChild<QObject*>("cancel"), SIGNAL(clicked()),
           this, SLOT(close()) );

  // Connect the save button
  connect( win_->rootObject()->findChild<QObject*>("save"), SIGNAL(clicked()),
           this, SLOT(save()) );
}

void
Settings::close()
{
  qEnter();

  if( win_->isVisible() )
  {
    qDebug() << "Closing settings window";

    QVariant ret;

    QMetaObject::invokeMethod( item_, "setUrl", Q_RETURN_ARG(QVariant, ret), Q_ARG(QVariant, "") );
    QMetaObject::invokeMethod( item_, "setApiKey", Q_RETURN_ARG(QVariant, ret), Q_ARG(QVariant, "") );

    win_->close();
  }
}

void
Settings::display()
{
  qEnter();

  QVariant ret;

  QMetaObject::invokeMethod( item_, "setUrl", Q_RETURN_ARG(QVariant, ret), Q_ARG(QVariant, url_) );
  QMetaObject::invokeMethod( item_, "setApiKey", Q_RETURN_ARG(QVariant, ret), Q_ARG(QVariant, apikey_) );

  if( !win_->isVisible() )
  {
    qDebug() << "Displaying settings window";
    win_->show();
  }
}

void
Settings::load()
{
  qEnter();

  url_ = settings_.value( "url" ).toString();
  apikey_ = settings_.value( "apikey" ).toString();

  qDebug() << "Loaded settings from file:" << _(url_) << _(apikey_);

  if( url_.isEmpty() || apikey_.isEmpty() )
    display();
}

void
Settings::save()
{
  qEnter();

  QVariant ret;

  QMetaObject::invokeMethod( item_, "getUrl", Q_RETURN_ARG(QVariant, ret) );
  url_ = ret.toString();
  settings_.setValue( "url", url_ );

  QMetaObject::invokeMethod( item_, "getApiKey", Q_RETURN_ARG(QVariant, ret) );
  apikey_ = ret.toString();
  settings_.setValue( "apikey", apikey_ );

  qDebug() << "Changed settings to" << _(url_) << _(apikey_);

  close();
}
