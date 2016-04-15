#include "IssueCreator.h"
#include "IssueSelector.h"
#include "logging.h"

#include <QSortFilterProxyModel>

using namespace qtredmine;
using namespace redtimer;
using namespace std;

IssueCreator::IssueCreator( SimpleRedmineClient* redmine, QQuickView* parent )
    : Window( "qrc:/IssueCreator.qml", parent ),
      redmine_( redmine )
{
    ENTER();

    // Issue selector window initialisation
    setResizeMode( QQuickView::SizeRootObjectToView );
    setModality( Qt::ApplicationModal );
    setFlags( Qt::Dialog );
    setTitle( "Issue Creator" );

    // Load projects
    QSortFilterProxyModel* projectProxyModel = new QSortFilterProxyModel();
    projectProxyModel->setSourceModel( &projectModel_ );
    projectProxyModel->setSortRole( SimpleModel::SimpleRoles::IdRole );
    ctx_->setContextProperty( "projectModel", projectProxyModel );

    loadProjects();

    // Load trackers
    QSortFilterProxyModel* trackerProxyModel = new QSortFilterProxyModel();
    trackerProxyModel->setSourceModel( &trackerModel_ );
    trackerProxyModel->setSortRole( SimpleModel::SimpleRoles::IdRole );
    ctx_->setContextProperty( "trackerModel", trackerProxyModel );

    loadTrackers();

    // Connect the project selected signal to the projectSelected slot
    connect( qml("project"), SIGNAL(activated(int)), this, SLOT(projectSelected(int)) );

    // Connect the tracker selected signal to the trackerSelected slot
    connect( qml("tracker"), SIGNAL(activated(int)), this, SLOT(trackerSelected(int)) );

    // Connect the issue selector button
    connect( qml("selectParentIssue"), SIGNAL(clicked()), this, SLOT(selectParentIssue()) );

    // Connect the create button clicked signal to the save slot
    connect( qml("create"), SIGNAL(clicked()), this, SLOT(save()) );

    // Connect the cancel button clicked signal to the close slot
    connect( qml("cancel"), SIGNAL(clicked()), this, SLOT(closeWin()) );

    // Connect the closed signal to the close slot
    connect( this, &Window::closed, [=](){ closeWin(); } );

    RETURN();
}

IssueCreator::~IssueCreator()
{
    ENTER();
    RETURN();
}

void
IssueCreator::closeWin()
{
    ENTER();

    if( isVisible() )
    {
        DEBUG() << "Closing issue creator window";
        if( cancelOnClose_ )
            cancelled();
        close();
    }

    RETURN();
}

void
IssueCreator::display()
{
    ENTER();

    if( !isVisible() )
    {
        DEBUG() << "Displaying issue creator window";
        show();
    }

    RETURN();
}

void
IssueCreator::loadProjects()
{
    ENTER();

    redmine_->retrieveProjects( [&]( Projects projects )
    {
        ENTER();

        projectModel_.clear();
        projectModel_.push_back( SimpleItem(NULL_ID, "Choose project") );
        for( const auto& project : projects )
            projectModel_.push_back( SimpleItem(project) );

        qml("project")->setProperty( "currentIndex", -1 );
        qml("project")->setProperty( "currentIndex", 0 );

        RETURN();
    } );

    RETURN();
}

void
IssueCreator::loadTrackers()
{
    ENTER();

    redmine_->retrieveTrackers( [&]( Trackers trackers )
    {
        ENTER();

        trackerModel_.clear();
        trackerModel_.push_back( SimpleItem(NULL_ID, "Choose tracker") );
        for( const auto& tracker : trackers )
            trackerModel_.push_back( SimpleItem(tracker) );

        qml("tracker")->setProperty( "currentIndex", -1 );
        qml("tracker")->setProperty( "currentIndex", 0 );

        RETURN();
    } );

    RETURN();
}

void
IssueCreator::projectSelected( int index )
{
    ENTER();

    projectId_ = projectModel_.at(index).id();
    DEBUG()(index)(projectId_);

    RETURN();
}

void
IssueCreator::save()
{
    ENTER();

    if( projectId_ == NULL_ID )
    {
        message( "Please select a project", QtCriticalMsg );
        RETURN();
    }

    if( trackerId_ == NULL_ID )
    {
        message( "Please select a tracker", QtCriticalMsg );
        RETURN();
    }

    if( qml("subject")->property("text").toString().isEmpty() )
    {
        message( "Please specify a subject", QtCriticalMsg );
        RETURN();
    }

    cancelOnClose_ = false;

    Issue issue;
    issue.project.id = projectId_;
    issue.tracker.id = trackerId_;

    if( !qml("parentIssue")->property("text").toString().isEmpty() )
        issue.parentId = qml("parentIssue")->property("text").toInt();

    if( !qml("parentIssue")->property("estimatedTime").toString().isEmpty() )
        issue.estimatedHours = qml("estimatedTime")->property("text").toDouble();

    issue.subject = qml("subject")->property("text").toString();
    issue.description = qml("description")->property("text").toString();

    redmine_->sendIssue( issue, [=](bool success, int id, RedmineError errorCode, QStringList errors)
    {
        ENTER()(success)(id)(errorCode)(errors);

        if( !success )
        {
            cancelOnClose_ = true;

            QString errorMsg = tr( "Could not create issue." );
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            RETURN();
        }

        message( tr("New issue created with ID %1").arg(id) );

        created( id );
        closeWin();

        RETURN();
    } );

    RETURN();
}

void
IssueCreator::selectParentIssue()
{
    // Issue selector initialisation
    IssueSelector* issueSelector = new IssueSelector( redmine_ );
    issueSelector->setTransientParent( this );
    if( projectId_ != NULL_ID )
        issueSelector->setProjectId( projectId_ );
    issueSelector->display();

    // Connect the issue selected signal to the setIssue slot
    connect( issueSelector, &IssueSelector::selected, [=](int issueId)
    {
        qml("parentIssue")->setProperty( "text", issueId );
        issueSelector->close();
    } );
}

void
IssueCreator::setParentIssueId( int id )
{
    ENTER();

    if( id != NULL_ID )
        qml("parentIssue")->setProperty( "text", id );

    RETURN();
}

void
IssueCreator::trackerSelected( int index )
{
    ENTER();

    trackerId_ = trackerModel_.at(index).id();
    DEBUG()(index)(trackerId_);

    RETURN();
}
