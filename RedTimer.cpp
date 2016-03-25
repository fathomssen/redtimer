#include "RedTimer.h"

#include "logging.h"

using namespace qtredmine;
using namespace redtimer;
using namespace std;

RedTimer::RedTimer( QObject* parent )
    : QObject( parent )
{
    ENTER();

    // Settings initialisation
    settings_ = new Settings( this );

    // Main window initialisation
    win_ = new QQuickView();
    win_->setResizeMode( QQuickView::SizeRootObjectToView );
    win_->setSource( QUrl(QStringLiteral("qrc:/RedTimer.qml")) );
    win_->setModality( Qt::ApplicationModal );

    // Main window access members
    ctx_ = win_->rootContext();
    item_ = qobject_cast<QQuickItem*>( win_->rootObject() );

    // Connect the settings button
    connect( item_->findChild<QQuickItem*>("settings"), SIGNAL(clicked()),
             settings_, SLOT(display()) );

    if( !win_->isVisible() )
    {
        DEBUG() << "Displaying main window";
        win_->show();
    }

    // Load settings
    settings_->load();

    // Connect to Redmine
    redmine_ = new Redmine( settings_->getUrl(), settings_->getApiKey(), this );

    // Initially update the GUI
    update();

    // Connect the settings saved signal to the reconnect slot
    connect( settings_, &Settings::saved, this, &RedTimer::reconnect );

    // Connect the project selected signal to the updateIssues slot
//    connect( item_->findChild<QQuickItem*>("project"), SIGNAL(activated(int)),
//             this, SLOT(projectSelected(int)) );

    RETURN();
}

void
RedTimer::projectSelected( int index )
{
    ENTER();

    DEBUG()(index);

    int projectId = projectModel_.at(index).id();
    DEBUG()(projectId);

    if( projectId == -1 )
        RETURN();

    // Search for project with ID
    // No map required here as there are usually just a few projects
    for( const auto& project : projects_ )
        if( project.id == projectId )
            project_ = project;

    DEBUG()(project_.id)(project_.name);

    updateIssues();

    RETURN();
}

void
RedTimer::reconnect()
{
    ENTER();

    redmine_->setUrl( settings_->getUrl() );
    redmine_->setAuthenticator( settings_->getApiKey() );

    update();

    RETURN();
}

void
RedTimer::refreshActivities()
{
    ENTER();

    QStringList list;
    list.append( "Choose activity" );
    for( const auto& activity : activities_ )
        list.append( activity.name );

    DEBUG(Activities:)(list);

    ctx_->setContextProperty( "activityModel", QVariant::fromValue(list) );

    RETURN();
}

void
RedTimer::refreshIssues()
{
    ENTER();

    QStringList list;
    for( const auto& issue : issues_ )
        list.append( issue.subject+" (#"+issue.id+")" );

    DEBUG() << "Issues:" << list;

    ctx_->setContextProperty( "issuesModel", QVariant::fromValue(list) );

    RETURN();
}

void
RedTimer::refreshIssueStatuses()
{
    ENTER();

    QStringList list;
    list.append( "Choose status" );
    for( const auto& issueStatus : issueStatuses_ )
        list.append( issueStatus.name );

    DEBUG(Issue statuses:)(list);

    ctx_->setContextProperty( "statusModel", QVariant::fromValue(list) );

    RETURN();
}

void
RedTimer::refreshProjects()
{
    ENTER();

    projectModel_.clear();
    projectModel_.insert( SimpleItem("Choose project") );
    for( const auto& project : projects_ )
        projectModel_.insert( SimpleItem(project) );

    DEBUG(Projects:)(projectModel_);

    ctx_->setContextProperty( "projectModel", &projectModel_ );

    RETURN();
}

void
RedTimer::refreshTrackers()
{
    ENTER();

    QStringList list;
    list.append( "Choose tracker" );
    for( const auto& tracker : trackers_ )
        list.append( tracker.name );

    ctx_->setContextProperty( "trackerModel", QVariant::fromValue(list) );

    RETURN();
}

void
RedTimer::update()
{
    ENTER();

    activity_   = Redmine::Enumeration();
    activities_ = Redmine::Enumerations();

    issue_  = Redmine::Issue();
    issues_ = Redmine::Issues();

    issueStatus_   = Redmine::IssueStatus();
    issueStatuses_ = Redmine::IssueStatuses();

    project_  = Redmine::Project();
    projects_ = Redmine::Projects();

    tracker_  = Redmine::Tracker();
    trackers_ = Redmine::Trackers();

    refreshActivities();
    refreshIssues();
    refreshIssueStatuses();
    refreshProjects();
    refreshTrackers();

    updateProjects();

    RETURN();
}

void
RedTimer::updateActivities()
{
    ENTER();

    redmine_->retrieveTimeEntryActivities( [&]( Redmine::Enumerations activities )
    {
        ENTER();

        activities_ = activities;
        //refreshActivities();

        RETURN();
    } );

    RETURN();
}

void
RedTimer::updateIssues()
{
    ENTER();

    updateIssues( -1 );

    RETURN();
}

void
RedTimer::updateIssues( int projectId )
{
    ENTER()(projectId);

    if( projectId == -1 )
        projectId = project_.id;

    DEBUG(Using project ID:)(projectId);

    redmine_->retrieveIssues( [=]( Redmine::Issues issues )
    {
        ENTER();

        issues_ = issues;
        qSort( issues_.begin(), issues_.end(),
               []( const Redmine::Issue& a, const Redmine::Issue& b ){ return a.id > b.id; } );

        refreshIssues();
    },
    QString("project_id=%1").arg(projectId) );
}

void
RedTimer::updateIssueStatuses()
{
    ENTER();

    redmine_->retrieveIssueStatuses( [&]( Redmine::IssueStatuses issueStatuses )
    {
        ENTER();

        issueStatuses_ = issueStatuses;
        //refreshIssueStatuses();

        RETURN();
    } );
}

void
RedTimer::updateProjects()
{
    ENTER();

    redmine_->retrieveProjects( [&]( Redmine::Projects projects )
    {
        ENTER();

        projects_ = projects;
        qSort( projects_.begin(), projects_.end(),
               []( const Redmine::Project& a, const Redmine::Project& b ){ return a.name < b.name; } );

        refreshProjects();

        RETURN();
    } );
}

void
RedTimer::updateTrackers()
{
    ENTER();

    redmine_->retrieveTrackers( [&]( Redmine::Trackers trackers )
    {
        ENTER();

        trackers_ = trackers;
        qSort( trackers_.begin(), trackers_.end(),
               []( const Redmine::Tracker& a, const Redmine::Tracker& b ){ return a.name < b.name; } );

        refreshTrackers();

        RETURN();
    } );
}
