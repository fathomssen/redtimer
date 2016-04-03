#include "Settings.h"
#include "logging.h"

#include <QAbstractButton>
#include <QQuickItem>

using namespace qtredmine;
using namespace redtimer;
using namespace std;

Settings::Settings( SimpleRedmineClient* redmine, QObject* parent )
    : QObject( parent ),
      redmine_( redmine ),
      settings_( "RedTimer.ini", QSettings::IniFormat, this )
{
    ENTER();

    // Settings window initialisation
    win_ = new QQuickView();
    win_->setResizeMode( QQuickView::SizeRootObjectToView );
    win_->setSource( QUrl(QStringLiteral("qrc:/Settings.qml")) );
    win_->setModality( Qt::ApplicationModal );
    win_->setFlags( Qt::Tool );

    // Settings window access members
    ctx_ = win_->rootContext();
    item_ = qobject_cast<QQuickItem*>( win_->rootObject() );

    // Connect the cancel button
    connect( qml("cancel"), SIGNAL(clicked()), this, SLOT(close()) );

    // Connect the save button
    connect( qml("apply"), SIGNAL(clicked()), this, SLOT(apply()) );

    RETURN();
}

void
Settings::apply()
{
    ENTER();

    QString oldUrl = url_;

    apiKey_ = qml("apikey")->property("text").toString();
    url_    = qml("url")->property("text").toString();

    if( oldUrl == url_ )
    {
        int workedOnIndex = qml("workedOn")->property("currentIndex").toInt();
        workedOnId_ = issueStatusModel_.at(workedOnIndex).id();
    }
    else
    {
        activityId_ = NULL_ID;
        issueId_    = NULL_ID;
        projectId_  = NULL_ID;
        workedOnId_ = NULL_ID;
    }

    DEBUG("Changed settings to")(apiKey_)(url_)(workedOnId_);

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

    qml("url")->setProperty( "text", url_ );
    qml("apikey")->setProperty( "text", apiKey_ );
    updateIssueStatuses();

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
    apiKey_     = settings_.value("apikey").toString();
    url_        = settings_.value("url").toString();
    workedOnId_ = settings_.value("workedOnId").toInt();

    // Other GUIs
    activityId_ = settings_.value("activity").toInt();
    issueId_    = settings_.value("issue").toInt();
    projectId_  = settings_.value("project").toInt();

    position_   = settings_.value("position").toPoint();

    DEBUG("Loaded settings from file:")
            (apiKey_)(url_)(workedOnId_)(activityId_)(issueId_)(projectId_)(position_);

    if( apiKey_.isEmpty() || url_.isEmpty() )
        display();

    applied();

    RETURN();
}

QQuickItem*
Settings::qml( QString qmlItem )
{
    ENTER()(qmlItem);
    RETURN( item_->findChild<QQuickItem*>(qmlItem) );
}

void
Settings::save()
{
    ENTER();

    // From Settings GUI
    settings_.setValue( "apikey",     apiKey_ );
    settings_.setValue( "url",        url_ );
    settings_.setValue( "workedOnId", workedOnId_ );

    // From other GUIs
    settings_.setValue( "activity", activityId_ );
    settings_.setValue( "issue",    issueId_ );
    settings_.setValue( "project",  projectId_ );

    settings_.setValue( "position", position_ );

    RETURN();
}

void
Settings::updateIssueStatuses()
{
    ENTER();

    if( apiKey_.isEmpty() || url_.isEmpty() )
    {
        issueStatusModel_.clear();
        issueStatusModel_.insert( SimpleItem("Currently not available") );
        ctx_->setContextProperty( "issueStatusModel", &issueStatusModel_ );

        RETURN();
    }

    redmine_->retrieveIssueStatuses( [&]( IssueStatuses issueStatuses )
    {
        ENTER();

        int currentIndex = 0;

        // Sort issues ascending by ID
        qSort( issueStatuses.begin(), issueStatuses.end(),
               []( const IssueStatus& a, const IssueStatus& b ){ return a.id < b.id; } );

        issueStatusModel_.clear();
        issueStatusModel_.insert( SimpleItem("Choose issue status") );
        for( const auto& issueStatus : issueStatuses )
        {
            if( issueStatus.id == workedOnId_ )
                currentIndex = issueStatusModel_.rowCount();

            issueStatusModel_.insert( SimpleItem(issueStatus) );
        }

        DEBUG()(issueStatusModel_)(workedOnId_)(currentIndex);

        ctx_->setContextProperty( "issueStatusModel", &issueStatusModel_ );

        if( currentIndex != 0 )
            qml("workedOn")->setProperty( "currentIndex", currentIndex );

        RETURN();
    } );

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

int
Settings::getWorkedOnId() const
{
    ENTER();
    RETURN( workedOnId_ );
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
