#include "IssueSelector.h"

#include "logging.h"

using namespace qtredmine;
using namespace redtimer;
using namespace std;

IssueSelector::IssueSelector( SimpleRedmineClient* redmine, QObject* parent )
    : QObject( parent ),
      redmine_( redmine )
{
    ENTER();

    // Issue selector window initialisation
    win_ = new QQuickView();
    win_->setSource( QUrl(QStringLiteral("qrc:/IssueSelector.qml")) );
    win_->setModality( Qt::ApplicationModal );

    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::CustomizeWindowHint  | Qt::WindowTitleHint;
    flags |= Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint;
    win_->setFlags( flags );

    // Issue selector window access members
    ctx_ = win_->rootContext();
    item_ = qobject_cast<QQuickItem*>( win_->rootObject() );

    // Connect the project selected signal to the projectSelected slot
    connect( item_->findChild<QQuickItem*>("project"), SIGNAL(activated(int)),
             this, SLOT(projectSelected(int)) );

    // Connect the issue selected signal to the issueSelected slot
    connect( item_->findChild<QQuickItem*>("issues"), SIGNAL(activated(int)),
             this, SLOT(issueSelected(int)) );

    updateProjects();

    RETURN();
}

void
IssueSelector::close()
{
    ENTER();

    if( win_->isVisible() )
    {
        DEBUG() << "Closing issue selector window";
        win_->close();
    }

    RETURN();
}

void
IssueSelector::display()
{
    ENTER();

    if( !win_->isVisible() )
    {
        DEBUG() << "Displaying issue selector window";
        win_->show();
        updateProjects();
    }

    RETURN();
}

void
IssueSelector::issueSelected( int index )
{
    ENTER();

    int issueId = issuesModel_.at(index).id;
    DEBUG()(index)(issueId);

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

    updateIssues();

    RETURN();
}

void
IssueSelector::updateIssues()
{
    ENTER();

    DEBUG()(projectId_);

    redmine_->retrieveIssues( [=]( Issues issues )
    {
        ENTER();

        // Sort issues descending by ID
        qSort( issues.begin(), issues.end(),
               []( const Issue& a, const Issue& b ){ return a.id > b.id; } );

        issuesModel_.clear();
        for( const auto& issue : issues )
            issuesModel_.insert( issue );

        DEBUG()(issuesModel_);

        ctx_->setContextProperty( "issuesModel", &issuesModel_ );

    },
    QString("project_id=%1").arg(projectId_) );
}

void
IssueSelector::updateProjects()
{
    ENTER();

    redmine_->retrieveProjects( [&]( Projects projects )
    {
        ENTER();

        int currentIndex = 0;

        // Sort issues ascending by name
        qSort( projects.begin(), projects.end(),
               []( const Project& a, const Project& b ){ return a.name < b.name; } );

        projectModel_.clear();
        projectModel_.insert( SimpleItem("Choose project") );
        for( const auto& project : projects )
        {
            if( project.id == projectId_ )
                currentIndex = projectModel_.rowCount();

            projectModel_.insert( SimpleItem(project) );
        }

        DEBUG()(projectModel_)(currentIndex);

        ctx_->setContextProperty( "projectModel", &projectModel_ );

        if( currentIndex != 0 )
        {
            item_->findChild<QQuickItem*>("project")->setProperty( "currentIndex", currentIndex );
            projectSelected( currentIndex );
        }

        RETURN();
    } );
}

int
IssueSelector::getProjectId() const
{
    ENTER();
    RETURN( projectId_ );
}

void
IssueSelector::setProjectId( int id )
{
    ENTER();
    projectId_ = id;
    RETURN();
}
