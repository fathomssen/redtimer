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
    win_->setResizeMode( QQuickView::SizeRootObjectToView );
    win_->setSource( QUrl(QStringLiteral("qrc:/IssueSelector.qml")) );
    win_->setModality( Qt::ApplicationModal );
    win_->setFlags( Qt::Tool );

    // Issue selector window access members
    ctx_ = win_->rootContext();
    item_ = qobject_cast<QQuickItem*>( win_->rootObject() );

    // Set the models
    issuesProxyModel_.setSourceModel( &issuesModel_ );
    issuesProxyModel_.setFilterCaseSensitivity( Qt::CaseInsensitive );
    issuesProxyModel_.setFilterRole( IssueModel::IssueRoles::SubjectRole );
    ctx_->setContextProperty( "issuesModel", &issuesProxyModel_ );
    ctx_->setContextProperty( "projectModel", &projectModel_ );

    // Connect the project selected signal to the projectSelected slot
    connect( qml("project"), SIGNAL(activated(int)), this, SLOT(projectSelected(int)) );

    // Connect the issue selected signal to the issueSelected slot
    connect( qml("issues"), SIGNAL(activated(int)), this, SLOT(issueSelected(int)) );

    // Connect the search accepted signal to the filterIssues slot
    connect( qml("search"), SIGNAL(textChanged()), this, SLOT(filterIssues()) );

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
        qml("search")->setProperty( "text", "" );
        updateProjects();
    }

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
    ENTER()(projectId_);

    redmine_->retrieveIssues( [=]( Issues issues )
    {
        ENTER();

        // Sort issues descending by ID
        qSort( issues.begin(), issues.end(),
               []( const Issue& a, const Issue& b ){ return a.id > b.id; } );

        issuesModel_.clear();
        for( const auto& issue : issues )
            issuesModel_.push_back( issue );

        DEBUG()(issuesModel_);
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
        projectModel_.push_back( SimpleItem("Choose project") );
        for( const auto& project : projects )
        {
            if( project.id == projectId_ )
                currentIndex = projectModel_.rowCount();

            projectModel_.push_back( SimpleItem(project) );
        }

        DEBUG()(projectModel_)(currentIndex);

        if( currentIndex != 0 )
        {
            qml("project")->setProperty( "currentIndex", currentIndex );
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

QQuickItem*
IssueSelector::qml( QString qmlItem )
{
    ENTER()(qmlItem);
    RETURN( item_->findChild<QQuickItem*>(qmlItem) );
}

QQuickView*
IssueSelector::window() const
{
    ENTER();
    RETURN( win_ );
}
