#include "IssueSelector.h"
#include "logging.h"

using namespace qtredmine;
using namespace redtimer;
using namespace std;

IssueSelector::IssueSelector( SimpleRedmineClient* redmine, MainWindow* mainWindow )
    : Window( "qrc:/IssueSelector.qml", mainWindow ),
      redmine_( redmine )
{
    ENTER();

    // Issue selector window initialisation
    setModality( Qt::ApplicationModal );
    setFlags( Qt::Dialog );
    setTitle( "Issue Selector" );

    // Set the models
    issuesProxyModel_.setSourceModel( &issuesModel_ );
    issuesProxyModel_.setSortRole( IssueModel::IssueRoles::IdRole );
    issuesProxyModel_.setDynamicSortFilter( true );
    issuesProxyModel_.setFilterCaseSensitivity( Qt::CaseInsensitive );
    issuesProxyModel_.setFilterRole( IssueModel::IssueRoles::SubjectRole );
    ctx_->setContextProperty( "issuesModel", &issuesProxyModel_ );

    QSortFilterProxyModel* projectProxyModel = new QSortFilterProxyModel( this );
    projectProxyModel->setSourceModel( &projectModel_ );
    projectProxyModel->setSortRole( SimpleModel::SimpleRoles::IdRole );
    projectProxyModel->setDynamicSortFilter( true );
    ctx_->setContextProperty( "projectModel", projectProxyModel );

    // Connect the project selected signal to the projectSelected slot
    connect( qml("project"), SIGNAL(activated(int)), this, SLOT(projectSelected(int)) );

    // Connect the issue selected signal to the issueSelected slot
    connect( qml("issues"), SIGNAL(activated(int)), this, SLOT(issueSelected(int)) );

    // Connect the search accepted signal to the filterIssues slot
    connect( qml("search"), SIGNAL(textChanged()), this, SLOT(filterIssues()) );

    // Connect the closed signal to the close slot
    connect( this, &Window::closed, [=](){ close(); } );

    RETURN();
}

void
IssueSelector::close()
{
    ENTER();

    Window::close();
    this->deleteLater();

    RETURN();
}

void
IssueSelector::display()
{
    ENTER();

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
IssueSelector::issueSelected( int index )
{
    ENTER();

    QModelIndex proxyIndex = issuesProxyModel_.index( index, 0 );
    int issueId = proxyIndex.data(SimpleModel::IdRole).toInt();
    DEBUG()(index)(proxyIndex)(issueId);

    selected( issueId );

    close();

    RETURN();
}

void
IssueSelector::projectSelected( int index )
{
    ENTER();

    projectId_ = projectModel_.at(index).id();
    DEBUG()(index)(projectId_);

    loadIssues();

    RETURN();
}

void
IssueSelector::loadIssues()
{
    ENTER()(projectId_);

    redmine_->retrieveIssues( [&]( Issues issues, RedmineError redmineError, QStringList errors )
    {
        ENTER();

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load issues.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            RETURN();
        }

        issuesModel_.clear();

        for( const auto& issue : issues )
            issuesModel_.push_back( issue );

        DEBUG()(issuesModel_);
    },
    RedmineOptions( QString("project_id=%1").arg(projectId_), true ) );

    RETURN();
}

void
IssueSelector::loadProjects()
{
    ENTER();

    // Clear and set first item at once and not wait for callback
    projectModel_.clear();
    projectModel_.push_back( SimpleItem(NULL_ID, "Choose project") );

    redmine_->retrieveProjects( [=]( Projects projects, RedmineError redmineError, QStringList errors )
    {
        ENTER();

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load projects.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            RETURN();
        }

        int currentIndex = 0;

        // Reset in case this has changed since calling loadProjects()
        projectModel_.clear();
        projectModel_.push_back( SimpleItem(NULL_ID, "Choose project") );

        for( const auto& project : projects )
        {
            if( project.id == projectId_ )
                currentIndex = projectModel_.rowCount();

            projectModel_.push_back( SimpleItem(project) );
        }

        DEBUG()(projectModel_)(currentIndex);

        qml("project")->setProperty( "currentIndex", -1 );
        qml("project")->setProperty( "currentIndex", currentIndex );

        RETURN();
    },
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

    if( fixed )
        qml("project")->setProperty( "enabled", false );

    loadIssues();

    RETURN();
}
