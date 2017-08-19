#include "qtredmine/Logging.h"

#include "IssueSelector.h"
#include "Settings.h"

using namespace qtredmine;
using namespace std;

namespace redtimer {

IssueSelector::IssueSelector( SimpleRedmineClient* redmine, MainWindow* mainWindow )
    : Window( "IssueSelector", mainWindow ),
      redmine_( redmine )
{
    ENTER();

    // Issue selector window initialisation
    setModality( Qt::ApplicationModal );
    setFlags( Qt::Dialog );
    setTitle( "Issue Selector" );

    // Set the models
    setCtxProperty( "assigneeModel", &assigneeModel_ );
    setCtxProperty( "projectModel", &projectModel_ );
    setCtxProperty( "versionModel", &versionModel_ );

    issuesProxyModel_.setSourceModel( &issuesModel_ );
    issuesProxyModel_.setSortRole( IssueModel::IssueRoles::IdRole );
    issuesProxyModel_.setDynamicSortFilter( true );
    issuesProxyModel_.setFilterCaseSensitivity( Qt::CaseInsensitive );
    issuesProxyModel_.setFilterRole( IssueModel::IssueRoles::SubjectRole );
    setCtxProperty( "issuesModel", &issuesProxyModel_ );

    // Connect the assignee selected signal to the assigneeSelected slot
    connect( qml("assignee"), SIGNAL(activated(int)), this, SLOT(assigneeSelected(int)) );

    // Connect the issue selected signal to the issueSelected slot
    connect( qml("issues"), SIGNAL(activated(int)), this, SLOT(issueSelected(int)) );

    // Connect the project selected signal to the projectSelected slot
    connect( qml("project"), SIGNAL(activated(int)), this, SLOT(projectSelected(int)) );

    // Connect the search accepted signal to the filterIssues slot
    connect( qml("search"), SIGNAL(textChanged()), this, SLOT(filterIssues()) );

    // Connect the version selected signal to the versionSelected slot
    connect( qml("version"), SIGNAL(activated(int)), this, SLOT(versionSelected(int)) );

    // Connect the closed signal to the close slot
    connect( this, &Window::closed, [=](){ close(); } );

    projectId_ = profileData()->projectId;
    loadProjects();
    loadAssignees();
    loadVersions();

    RETURN();
}

void
IssueSelector::close()
{
    ENTER();

    settings()->windowData()->issueSelector = getWindowData();
    settings()->save();

    Window::close();

    RETURN();
}

void
IssueSelector::display()
{
    ENTER();

    setWindowData( settings()->windowData()->issueSelector );

    show();

    if( projectModel_.rowCount() == 0 )
        loadProjects();

    RETURN();
}

void
IssueSelector::filterIssues()
{
    ENTER();

    QString filter = qml("search")->property("text").toString();
    issuesProxyModel_.setFilterFixedString( filter );

    RETURN();
}

void
IssueSelector::assigneeSelected( int index )
{
    ENTER();

    assigneeId_ = assigneeModel_.at(index).id();
    DEBUG()(index)(assigneeId_);

    loadIssues();

    RETURN();
}

void
IssueSelector::issueSelected( int index )
{
    ENTER();

    QModelIndex proxyIndex = issuesProxyModel_.index( index, 0 );
    int issueId = proxyIndex.data(SimpleModel::IdRole).toInt();
    DEBUG()(index)(proxyIndex)(issueId);

    emit selected( issueId );

    close();

    RETURN();
}

void
IssueSelector::projectSelected( int index )
{
    ENTER();

    projectId_ = projectModel_.at(index).id();
    DEBUG()(index)(projectId_);

    profileData()->projectId = projectId_;

    // Ensure that no old issues are displayed
    issuesModel_.clear();

    // Load dependent filters
    loadAssignees();
    loadVersions();

    loadIssues();

    RETURN();
}

void
IssueSelector::versionSelected( int index )
{
    ENTER();

    versionId_ = versionModel_.at(index).id();
    DEBUG()(index)(versionId_);

    loadIssues();

    RETURN();
}

void
IssueSelector::loadAssignees()
{
    ENTER()(projectId_);

    if( projectId_ == NULL_ID )
    {
        // Clear and set first item at once and not wait for callback
        assigneeId_ = NULL_ID;
        assigneeModel_.clear();
        assigneeModel_.push_back( SimpleItem(NULL_ID, "Choose assignee") );
        qml("assignee")->setProperty( "currentIndex", -1 );
        qml("assignee")->setProperty( "currentIndex", 0 );
        RETURN();
    }

    if( !connected() )
        RETURN();

    ++callbackCounter_;
    redmine_->retrieveMemberships( [=]( Memberships assignees, RedmineError redmineError, QStringList errors )
    {
        CBENTER();

        if( redmineError != RedmineError::NO_ERR )
        {
            QString errorMsg = tr("Could not load assignees.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        int currentIndex = 0;

        // Reset in case this has changed since calling loadAssignees()
        assigneeModel_.clear();
        assigneeModel_.push_back( SimpleItem(NULL_ID, "Choose assignee") );

        // Sort assignees by name
        sort( assignees.begin(), assignees.end(),
              []( const Membership& l, const Membership& r )
              {
                QString lname, rname;

                if( l.user.id != NULL_ID )
                  lname = l.user.name;
                else if( l.group.id != NULL_ID )
                  lname = l.group.name;

                if( r.user.id != NULL_ID )
                  rname = r.user.name;
                else if( r.group.id != NULL_ID )
                  rname = r.group.name;

                return lname < rname;
              } );

        for( const auto& assignee : assignees )
        {
            if( assignee.id == assigneeId_ )
                currentIndex = assigneeModel_.rowCount();

            if( assignee.user.id != NULL_ID )
                assigneeModel_.push_back( SimpleItem(assignee.user) );
            else if( assignee.group.id != NULL_ID )
                assigneeModel_.push_back( SimpleItem(assignee.group) );
        }

        DEBUG()(assigneeModel_)(currentIndex);

        qml("assignee")->setProperty( "currentIndex", -1 );
        qml("assignee")->setProperty( "currentIndex", currentIndex );

        CBRETURN();
    },
    projectId_,
    QString("limit=100") );
}

void
IssueSelector::loadIssues()
{
    ENTER()(projectId_);

    if( projectId_ == NULL_ID )
        RETURN();

    QString parameters = QString("project_id=%1").arg(projectId_);
    if( assigneeId_ != NULL_ID )
        parameters.append( QString("&assigned_to_id=%1").arg(assigneeId_) );
    if( versionId_ != NULL_ID )
        parameters.append( QString("&fixed_version_id=%1").arg(versionId_) );

    if( !connected() )
        RETURN();

    ++callbackCounter_;
    redmine_->retrieveIssues( [&]( Issues issues, RedmineError redmineError, QStringList errors )
    {
        CBENTER();

        if( redmineError != RedmineError::NO_ERR )
        {
            QString errorMsg = tr("Could not load issues.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        issuesModel_.clear();

        for( const auto& issue : issues )
            issuesModel_.push_back( issue );

        DEBUG()(issuesModel_);

        CBRETURN();
    },
    RedmineOptions( parameters, true ) );

    RETURN();
}

void
IssueSelector::loadProjects()
{
    ENTER();

    if( !connected() )
        RETURN();

    ++callbackCounter_;
    redmine_->retrieveProjects( [=]( Projects projects, RedmineError redmineError, QStringList errors )
    {
        CBENTER();

        if( redmineError != RedmineError::NO_ERR )
        {
            QString errorMsg = tr("Could not load projects.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        int currentIndex = 0;

        // Reset in case this has changed since calling loadProjects()
        projectModel_.clear();
        projectModel_.push_back( SimpleItem(NULL_ID, "Choose project") );

        for( const auto& project : projects )
        {
            if( project.id == projectId_ )
                currentIndex = projectModel_.rowCount();

            QString name = project.name;
            if( project.parent.id != NULL_ID )
                name.prepend( "- " );

            projectModel_.push_back( SimpleItem(project.id, name) );
        }

        DEBUG()(projectModel_)(currentIndex);

        qml("project")->setProperty( "currentIndex", -1 );
        qml("project")->setProperty( "currentIndex", currentIndex );

        CBRETURN();
    },
    QString("limit=100") );
}

void
IssueSelector::loadVersions()
{
    ENTER();

    if( projectId_ == NULL_ID )
    {
        // Clear and set first item at once and not wait for callback
        versionId_ = NULL_ID;
        versionModel_.clear();
        versionModel_.push_back( SimpleItem(NULL_ID, "Choose version") );
        qml("version")->setProperty( "currentIndex", -1 );
        qml("version")->setProperty( "currentIndex", 0 );
        RETURN();
    }

    if( !connected() )
        RETURN();

    ++callbackCounter_;
    redmine_->retrieveVersions( [=]( Versions versions, RedmineError redmineError, QStringList errors )
    {
        CBENTER();

        if( redmineError != RedmineError::NO_ERR )
        {
            QString errorMsg = tr("Could not load versions.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        int currentIndex = 0;

        // Reset in case this has changed since calling loadVersions()
        versionModel_.clear();
        versionModel_.push_back( SimpleItem(NULL_ID, "Choose version") );

        // Sort versions by due date
        sort( versions.begin(), versions.end(),
              [](const Version& l, const Version& r){ return l.dueDate < r.dueDate; } );

        for( const auto& version : versions )
        {
            // @todo Control the date check with a switch
            //if( version.dueDate < QDate::currentDate() )
            //    continue;

            // @todo Control the status check with a switch
            if( version.status != VersionStatus::open )
                continue;

            if( version.id == versionId_ )
                currentIndex = versionModel_.rowCount();

            versionModel_.push_back( SimpleItem(version) );
        }

        DEBUG()(versionModel_)(currentIndex);

        qml("version")->setProperty( "currentIndex", -1 );
        qml("version")->setProperty( "currentIndex", currentIndex );

        CBRETURN();
    },
    projectId_,
    QString("limit=100") );
}

int
IssueSelector::getProjectId() const
{
    ENTER();
    RETURN( projectId_ );
}

void
IssueSelector::setProjectId( int id, bool fixed )
{
    ENTER();

    projectId_ = id;

    loadProjects();
    loadAssignees();
    loadVersions();

    if( fixed )
        qml("project")->setProperty( "enabled", false );

    loadIssues();

    RETURN();
}

} // redtimer
