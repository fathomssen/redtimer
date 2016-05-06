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
    win_->setFlags( Qt::Dialog );
    win_->setTitle( "Settings" );

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

    QString oldUrl = data.url;

    data.apiKey            = qml("apikey")->property("text").toString();
    data.checkConnection   = qml("checkConnection")->property("checked").toBool();
    data.ignoreSslErrors   = qml("ignoreSslErrors")->property("checked").toBool();
    data.numRecentIssues   = qml("numRecentIssues")->property("text").toInt();
    data.url               = qml("url")->property("text").toString();
    data.useSystemTrayIcon = qml("useSystemTrayIcon")->property("checked").toBool();

    if( oldUrl == data.url )
    {
        int workedOnIndex = qml("workedOn")->property("currentIndex").toInt();
        if( workedOnIndex )
            data.workedOnId = issueStatusModel_.at(workedOnIndex).id();
    }
    else
    {
        data.activityId      = NULL_ID;
        data.issueId         = NULL_ID;
        data.projectId       = NULL_ID;
        data.workedOnId      = NULL_ID;

        while( !data.recentIssues.isEmpty() )
            data.recentIssues.removeLast();
    }

    DEBUG("Changed settings to")(data.apiKey)(data.checkConnection)(data.ignoreSslErrors)
                                (data.numRecentIssues)(data.url)(data.useSystemTrayIcon)(data.workedOnId);

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

    qml("apikey")->setProperty( "text", data.apiKey );
    qml("checkConnection")->setProperty( "checked", data.checkConnection );
    qml("ignoreSslErrors")->setProperty( "checked", data.ignoreSslErrors );
    qml("numRecentIssues")->setProperty( "text", data.numRecentIssues );
    qml("url")->setProperty( "text", data.url );
    qml("useSystemTrayIcon")->setProperty( "checked", data.useSystemTrayIcon );

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
    if( !settings_.value("apikey").isNull() )
        data.apiKey = settings_.value("apikey").toString();
    if( !settings_.value("checkConnection").isNull() )
        data.checkConnection = settings_.value("checkConnection").toBool();
    if( !settings_.value("ignoreSslErrors").isNull() )
        data.ignoreSslErrors = settings_.value("ignoreSslErrors").toBool();
    if( !settings_.value("url").isNull() )
        data.url = settings_.value("url").toString();
    if( !settings_.value("useSystemTrayIcon").isNull() )
        data.useSystemTrayIcon = settings_.value("useSystemTrayIcon").toBool();
    if( !settings_.value("workedOnId").isNull() )
        data.workedOnId = settings_.value("workedOnId").toInt();

    if( !settings_.value("numRecentIssues").isNull() )
        data.numRecentIssues = settings_.value("numRecentIssues").toInt();

    // Other GUIs
    data.activityId  = settings_.value("activity").toInt();
    data.issueId     = settings_.value("issue").toInt();
    data.position    = settings_.value("position").toPoint();
    data.projectId   = settings_.value("project").toInt();

    int size = settings_.beginReadArray( "recentIssues" );
    for( int i = 0; i < size; ++i )
    {
        settings_.setArrayIndex( i );
        Issue issue;
        issue.id      = settings_.value("id").toInt();
        issue.subject = settings_.value("subject").toString();
        data.recentIssues.append( issue );
    }
    settings_.endArray();


    DEBUG("Loaded settings from file:")
            (data.apiKey)(data.checkConnection)(data.ignoreSslErrors)(data.numRecentIssues)(data.url)
            (data.useSystemTrayIcon)(data.workedOnId)
            (data.activityId)(data.issueId)(data.position)(data.projectId)(data.recentIssues);

    if( data.apiKey.isEmpty() || data.url.isEmpty() )
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
    settings_.setValue( "apikey",            data.apiKey );
    settings_.setValue( "checkConnection",   data.checkConnection );
    settings_.setValue( "ignoreSslErrors",   data.ignoreSslErrors );
    settings_.setValue( "numRecentIssues",   data.numRecentIssues );
    settings_.setValue( "url",               data.url );
    settings_.setValue( "useSystemTrayIcon", data.useSystemTrayIcon );
    settings_.setValue( "workedOnId",        data.workedOnId );

    // From other GUIs
    settings_.setValue( "activity", data.activityId );
    settings_.setValue( "issue",    data.issueId );
    settings_.setValue( "position", data.position );
    settings_.setValue( "project",  data.projectId );

    settings_.beginWriteArray( "recentIssues" );
    for( int i = 0; i < data.recentIssues.size(); ++i )
    {
        settings_.setArrayIndex( i );
        settings_.setValue( "id",      data.recentIssues.at(i).id );
        settings_.setValue( "subject", data.recentIssues.at(i).subject );
    }
    settings_.endArray();

    RETURN();
}

void
Settings::updateIssueStatuses()
{
    ENTER();

    if( data.apiKey.isEmpty() || data.url.isEmpty() )
    {
        issueStatusModel_.clear();
        issueStatusModel_.push_back( SimpleItem(NULL_ID, "Currently not available") );
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
        issueStatusModel_.push_back( SimpleItem(NULL_ID, "Choose issue status") );
        for( const auto& issueStatus : issueStatuses )
        {
            if( issueStatus.id == data.workedOnId )
                currentIndex = issueStatusModel_.rowCount();

            issueStatusModel_.push_back( SimpleItem(issueStatus) );
        }

        DEBUG()(issueStatusModel_)(data.workedOnId)(currentIndex);

        ctx_->setContextProperty( "issueStatusModel", &issueStatusModel_ );

        if( currentIndex != 0 )
            qml("workedOn")->setProperty( "currentIndex", currentIndex );

        RETURN();
    } );

    RETURN();
}

QQuickView*
Settings::window() const
{
    ENTER();
    RETURN( win_ );
}
