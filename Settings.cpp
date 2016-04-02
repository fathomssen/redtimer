#include "Settings.h"

#include "logging.h"

#include <QAbstractButton>
#include <QQuickItem>

using namespace redtimer;
using namespace std;

Settings::Settings( QObject* parent )
    : QObject( parent ),
      settings_( "RedTimer.ini", QSettings::IniFormat, this )
{
    ENTER();

    win_ = new QQuickView();
    win_->setSource( QUrl(QStringLiteral("qrc:/Settings.qml")) );
    win_->setModality( Qt::ApplicationModal );
    win_->setFlags( Qt::Tool );

    item_ = qobject_cast<QQuickItem*>( win_->rootObject() );

    // Connect the cancel button
    connect( item_->findChild<QQuickItem*>("cancel"), SIGNAL(clicked()),
             this, SLOT(close()) );

    // Connect the save button
    connect( item_->findChild<QQuickItem*>("apply"), SIGNAL(clicked()),
             this, SLOT(apply()) );

    RETURN();
}

void
Settings::apply()
{
    ENTER();

    QString oldUrl    = url_;
    QString oldApiKey = apiKey_;

    url_ = item_->findChild<QQuickItem*>("url")->property( "text" ).toString();
    apiKey_ = item_->findChild<QQuickItem*>("apikey")->property( "text" ).toString();

    if( oldUrl != url_ || oldApiKey != apiKey_ )
    {
        activityId_ = -1;
        issueId_    = -1;
        projectId_  = -1;
    }

    DEBUG("Changed settings to")(url_)(apiKey_);

    DEBUG() << "Emitting applied() signal";
    applied();

    close();

    RETURN();
}

void
Settings::close()
{
    ENTER();

    if( win_->isVisible() )
    {
        DEBUG() << "Closing settings window";
        win_->close();
    }

    RETURN();
}

void
Settings::display()
{
    ENTER();

    item_->findChild<QQuickItem*>("url")->setProperty( "text", url_ );
    item_->findChild<QQuickItem*>("apikey")->setProperty( "text", apiKey_ );

    if( !win_->isVisible() )
    {
        DEBUG() << "Displaying settings window";
        win_->show();
    }

    RETURN();
}

void
Settings::load()
{
    ENTER();

    // Settings GUI
    apiKey_ = settings_.value( "apikey" ).toString();
    url_    = settings_.value( "url" ).toString();

    // Other GUIs
    activityId_ = settings_.value("activity").toInt();
    issueId_    = settings_.value("issue").toInt();
    projectId_  = settings_.value("project").toInt();

    position_   = settings_.value("position").toPoint();

    DEBUG("Loaded settings from file:")(url_)(apiKey_)(activityId_)(issueId_)(projectId_)(position_);

    if( url_.isEmpty() || apiKey_.isEmpty() )
        display();

    applied();

    RETURN();
}

void
Settings::save()
{
    ENTER();

    // From Settings GUI
    settings_.setValue( "apikey", apiKey_ );
    settings_.setValue( "url",    url_ );

    // From other GUIs
    settings_.setValue( "activity", activityId_ );
    settings_.setValue( "issue",    issueId_ );
    settings_.setValue( "project",  projectId_ );

    settings_.setValue( "position", position_ );

    RETURN();
}

int
Settings::getActivity()
{
    ENTER();
    RETURN( activityId_ );
}

QString
Settings::getApiKey() const
{
    ENTER();
    RETURN( apiKey_ );
}

int
Settings::getIssue()
{
    ENTER();
    RETURN( issueId_ );
}

QPoint
Settings::getPosition()
{
    ENTER();
    RETURN( position_ );
}

int
Settings::getProject()
{
    ENTER();
    RETURN( projectId_ );
}

QString
Settings::getUrl() const
{
    ENTER();
    RETURN( url_ );
}

void
Settings::setActivity( int id )
{
    ENTER();
    activityId_ = id;
    RETURN();
}

void
Settings::setIssue( int id )
{
    ENTER();
    issueId_ = id;
    RETURN();
}

void
Settings::setPosition( QPoint position )
{
    ENTER();
    position_ = position;
    RETURN();
}

void
Settings::setProject( int id )
{
    ENTER();
    projectId_ = id;
    RETURN();
}

QQuickView*
Settings::window() const
{
    ENTER();
    RETURN( win_ );
}
