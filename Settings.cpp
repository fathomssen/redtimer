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

    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::CustomizeWindowHint  | Qt::WindowTitleHint;
    flags |= Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint;
    win_->setFlags( flags );

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

    url_ = item_->findChild<QQuickItem*>("url")->property( "text" ).toString();
    apiKey_ = item_->findChild<QQuickItem*>("apikey")->property( "text" ).toString();

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

    // GUI
    apiKey_ = settings_.value( "apikey" ).toString();
    url_    = settings_.value( "url" ).toString();

    // External
    activityId_ = settings_.value("activity").toInt();
    issueId_    = settings_.value("issue").toInt();
    projectId_  = settings_.value("project").toInt();

    DEBUG("Loaded settings from file:")(url_)(apiKey_)(activityId_)(issueId_)(projectId_);

    if( url_.isEmpty() || apiKey_.isEmpty() )
        display();

    applied();

    RETURN();
}

void
Settings::save()
{
    ENTER();

    // From GUI
    settings_.setValue( "apikey", apiKey_ );
    settings_.setValue( "url",    url_ );

    // From external
    settings_.setValue( "activity", activityId_ );
    settings_.setValue( "issue",    issueId_ );
    settings_.setValue( "project",  projectId_ );

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
Settings::setProject( int id )
{
    ENTER();
    projectId_ = id;
    RETURN();
}
