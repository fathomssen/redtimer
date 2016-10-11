#include "qtredmine/Logging.h"

#include "IssueCreator.h"
#include "IssueSelector.h"
#include "Settings.h"

#include <QSortFilterProxyModel>
#include <QTime>

using namespace qtredmine;
using namespace redtimer;
using namespace std;

IssueCreator::IssueCreator( SimpleRedmineClient* redmine, MainWindow* mainWindow )
    : Window( "IssueCreator", mainWindow ),
      redmine_( redmine )
{
    ENTER();

    // Issue selector window initialisation
    setModality( Qt::ApplicationModal );
    setFlags( Qt::Dialog );
    setTitle( "Issue Creator" );
    initHeight_ = height();

    // Set models
    QSortFilterProxyModel* categoryProxyModel = new QSortFilterProxyModel( this );
    categoryProxyModel->setSourceModel( &categoryModel_ );
    categoryProxyModel->setSortRole( SimpleModel::SimpleRoles::IdRole );
    ctx_->setContextProperty( "categoryModel", categoryProxyModel );

    QSortFilterProxyModel* projectProxyModel = new QSortFilterProxyModel( this );
    projectProxyModel->setSourceModel( &projectModel_ );
    projectProxyModel->setSortRole( SimpleModel::SimpleRoles::IdRole );
    ctx_->setContextProperty( "projectModel", projectProxyModel );

    QSortFilterProxyModel* trackerProxyModel = new QSortFilterProxyModel( this );
    trackerProxyModel->setSourceModel( &trackerModel_ );
    trackerProxyModel->setSortRole( SimpleModel::SimpleRoles::IdRole );
    ctx_->setContextProperty( "trackerModel", trackerProxyModel );

    // Load current user
    loadCurrentUser();

    // Connect the project selected signal to the projectSelected slot
    connect( qml("project"), SIGNAL(activated(int)), this, SLOT(projectSelected(int)) );

    // Connect the tracker selected signal to the trackerSelected slot
    connect( qml("tracker"), SIGNAL(activated(int)), this, SLOT(trackerSelected(int)) );

    // Connect the category selected signal to the categorySelected slot
    connect( qml("category"), SIGNAL(activated(int)), this, SLOT(categorySelected(int)) );

    // Connect the use current issue as parent button
    connect( qml("useCurrentIssue"), SIGNAL(clicked()), this, SLOT(useCurrentIssue()) );

    // Connect the use parent issue as parent button
    connect( qml("useCurrentIssueParent"), SIGNAL(clicked()), this, SLOT(useCurrentIssueParent()) );

    // Connect the issue selector button
    connect( qml("selectParentIssue"), SIGNAL(clicked()), this, SLOT(selectParentIssue()) );

    // Connect the create button clicked signal to the save slot
    connect( qml("create"), SIGNAL(clicked()), this, SLOT(save()) );

    // Connect the cancel button clicked signal to the close slot
    connect( qml("cancel"), SIGNAL(clicked()), this, SLOT(close()) );

    // Connect the closed signal to the close slot
    connect( this, &Window::closed, [=](){ close(); } );

    RETURN();
}

IssueCreator::~IssueCreator()
{
    ENTER();
    RETURN();
}

void
IssueCreator::categorySelected( int index )
{
    ENTER();

    categoryId_ = categoryModel_.at(index).id();
    DEBUG()(index)(categoryId_);

    RETURN();
}

void
IssueCreator::close()
{
    ENTER();

    if( cancelOnClose_ )
        cancelled();

    mainWindow_->settings_->win_.issueCreator = getWindowData();
    mainWindow_->settings_->save();

    Window::close();
    this->deleteLater();

    RETURN();
}

void
IssueCreator::display()
{
    ENTER();

    setWindowData( mainWindow_->settings_->win_.issueCreator );

    show();
    refreshGui();

    RETURN();
}

int
IssueCreator::getProjectId() const
{
    ENTER();
    RETURN( projectId_ );
}

void
IssueCreator::loadCategories()
{
    ENTER()(projectId_);

    if( projectId_ == NULL_ID )
        RETURN();

    ++callbackCounter_;
    redmine_->retrieveProject( [&]( Project project, RedmineError redmineError, QStringList errors )
    {
        CBENTER();

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load projects.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        categoryModel_.clear();
        qml("category")->setProperty( "currentIndex", -1 );

        if( project.categories.size() == 0 )
        {
            qml("category")->setProperty( "enabled", false );
        }
        else
        {
            categoryModel_.push_back( SimpleItem(NULL_ID, "") );
            for( const auto& category : project.categories )
                categoryModel_.push_back( SimpleItem(category) );

            qml("category")->setProperty( "currentIndex", 0 );
            qml("category")->setProperty( "enabled", true );
        }

        CBRETURN();
    },
    projectId_ );

    RETURN();
}

void
IssueCreator::loadCurrentUser()
{
    ENTER();

    ++callbackCounter_;
    redmine_->retrieveCurrentUser( [&]( User user, RedmineError redmineError, QStringList errors )
    {
        CBENTER();

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load current user.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        currentUserId_ = user.id;
        CBRETURN();
    } );

    RETURN();
}

void
IssueCreator::loadCustomFields()
{
    ENTER()(useCustomFields_)(projectId_);

    if( !useCustomFields_ )
        RETURN();

    if( projectId_ == NULL_ID )
        RETURN();

    CustomFieldFilter filter;
    filter.projectId = projectId_;
    filter.type = "issue";

    ++callbackCounter_;
    redmine_->retrieveCustomFields( [&]( CustomFields customFields, RedmineError redmineError,
                                         QStringList errors )
    {
        CBENTER();

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load custom fields.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        // Delete previously loaded custom fields
        for( const auto& customField : customFields_ )
        {
            customFieldItems_[customField.id].first->deleteLater();
            customFieldItems_[customField.id].second->deleteLater();
            customFieldModels_[customField.id]->deleteLater();
        }
        customFields_.clear();
        customFieldItems_.clear();
        customFieldModels_.clear();

        // Create loaded custom fields
        for( const auto& customField : customFields )
        {
            customFields_.push_back( customField );

            QString labelData;
            {
                QTextStream s( &labelData );
                s << "import QtQuick 2.5" << endl;
                s << "import QtQuick.Controls 1.5" << endl;
                s << "import QtQuick.Layouts 1.3" << endl;
                s <<   "Label {" << endl;
                s <<     "id: labelCustomField" << customField.id << endl;
                s <<     "text: '" << customField.name << "'" << endl;
                s <<   "}" << endl;
            }

            QString entryFieldData;
            {
                QTextStream s( &entryFieldData );
                s << "import QtQuick 2.5" << endl;
                s << "import QtQuick.Controls 1.5" << endl;
                s << "import QtQuick.Layouts 1.3" << endl;

                // Distinguish custom field type
                QString entryFieldId = QString("entryField%1").arg(customField.id);
                QString entryFieldIds = QString("id: %1\nobjectName: \"%1\"").arg(entryFieldId);
                QString layout = "Layout.fillWidth: true";


                if( customField.format == "list" )
                {
                    QString entryFieldModel = entryFieldId.append("Model");

                    s << "ComboBox {" << endl;
                    s <<   entryFieldIds << endl;
                    s <<   "model: " << entryFieldModel << endl;
                    s <<   "textRole: \"name\"" << endl;
                    s <<   layout << endl;
                    s << "}" << endl;

                    customFieldModels_[customField.id] = new SimpleModel( this );

                    for( const auto& value : customField.possibleValues )
                        customFieldModels_[customField.id]->push_back( SimpleItem(NULL_ID, value) );

                    ctx_->setContextProperty( entryFieldModel, customFieldModels_[customField.id] );
                }
                else
                {
                    s << "TextField {" << endl;
                    s <<   entryFieldIds << endl;
                    s <<   layout << endl;
                    s << "}" << endl;
                }
            }

            DEBUG()(labelData)(entryFieldData);

            QQuickItem* labelItem = nullptr;
            QQuickItem* entryFieldItem = nullptr;

            auto createItem = [&]( QQuickItem*& item, QString data )
            {
                ENTER()(data)(engine());

                QQmlComponent component( engine() );
                component.setData( data.toUtf8(), QUrl() );

                DEBUG()(component.status());

                if( component.status() != QQmlComponent::Ready )
                {
                    DEBUG()(component.errorString());
                    RETURN();
                }

                item = qobject_cast<QQuickItem*>( component.create() );
                item->setParentItem( qml("maingrid") );

                RETURN();
            };

            createItem( labelItem, labelData );
            createItem( entryFieldItem, entryFieldData );

            DEBUG()(labelItem)(entryFieldItem);

            if( !labelItem || !entryFieldItem )
            {
                DEBUG("Skipping custom field") << customField.name;
                continue;
            }

            labelItem->stackAfter( qml("customFields") );
            entryFieldItem->stackAfter( labelItem );

            customFieldItems_[customField.id] = qMakePair( labelItem, entryFieldItem );
        }

        // @todo Rough estimation that a custom field is 30 pixels in height and 40 pixel on OS X
        int itemHeight = 30;
#ifdef Q_OS_OSX
        itemHeight = 40;
#endif
        setHeight( initHeight_ + customFieldItems_.size() * itemHeight);
        setMinimumHeight( height() );

        CBRETURN();
    },
    filter );

    RETURN();
}

void
IssueCreator::loadProjects()
{
    ENTER();

    ++callbackCounter_;
    redmine_->retrieveProjects( [&]( Projects projects, RedmineError redmineError, QStringList errors )
    {
        CBENTER();

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load projects.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        int currentIndex = 0;

        projectModel_.clear();
        projectModel_.push_back( SimpleItem(NULL_ID, "Choose project") );
        for( const auto& project : projects )
        {
            if( projectId_ == project.id )
                currentIndex = projectModel_.rowCount();

            projectModel_.push_back( SimpleItem(project) );
        }

        qml("project")->setProperty( "currentIndex", -1 );
        qml("project")->setProperty( "currentIndex", currentIndex );

        trackerModel_.clear();

        if( projectId_ )
            refreshGui();

        CBRETURN();
    },
    QString("limit=100") );

    RETURN();
}

void
IssueCreator::loadTrackers()
{
    ENTER()(projectId_);

    if( projectId_ == NULL_ID )
        RETURN();

    ++callbackCounter_;
    redmine_->retrieveProject( [&]( Project project, RedmineError redmineError, QStringList errors )
    {
        CBENTER();

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load projects.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        int currentIndex = 0;

        trackerModel_.clear();
        trackerModel_.push_back( SimpleItem(NULL_ID, "Choose tracker") );
        for( const auto& tracker : project.trackers )
        {
            if( mainWindow_->settings_->data_.defaultTrackerId == tracker.id )
            {
                currentIndex = trackerModel_.rowCount();
                trackerId_ = tracker.id;
            }

            trackerModel_.push_back( SimpleItem(tracker) );
        }

        qml("tracker")->setProperty( "currentIndex", -1 );
        qml("tracker")->setProperty( "currentIndex", currentIndex );
        qml("tracker")->setProperty( "enabled", true );

        if( trackerId_ )
            refreshGui();

        CBRETURN();
    },
    projectId_ );

    RETURN();
}

void
IssueCreator::projectSelected( int index )
{
    ENTER();

    projectId_ = projectModel_.at(index).id();
    DEBUG()(index)(projectId_);

    refreshGui();

    RETURN();
}

void
IssueCreator::refreshGui()
{
    ENTER();

    // 1. Load projects
    if( projectModel_.rowCount() == 0 )
    {
        loadProjects();
        RETURN();
    }

    // 2. When project has been selected, load trackers, categories and custom fields
    if( projectId_ != NULL_ID && trackerModel_.rowCount() == 0 )
        loadTrackers();

    if( projectId_ != NULL_ID && categoryModel_.rowCount() == 0 )
        loadCategories();

    if( projectId_ != NULL_ID )
        loadCustomFields();

    if( trackerId_ == NULL_ID )
        RETURN();

    // 3. When tracker has been selected, enable the other fields
    qml("subject")->setProperty( "enabled", true );
    qml("parentIssue")->setProperty( "enabled", true );
    qml("useCurrentIssue")->setProperty( "enabled", true );
    qml("useCurrentIssueParent")->setProperty( "enabled", true );
    qml("selectParentIssue")->setProperty( "enabled", true );
    qml("estimatedTime")->setProperty( "enabled", true );
    qml("description")->setProperty( "enabled", true );
    qml("create")->setProperty( "enabled", true );

    RETURN();
}

void
IssueCreator::save()
{
    ENTER();

    Issue issue;

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

    QString stime = qml("estimatedTime")->property("text").toString();
    if( !stime.isEmpty() )
    {
        // Try to find valid time string format
        // @todo: Source out to function
        QTime time = QTime::fromString( stime, "hh:mm:ss" );
        if( !time.isValid() )
            time = QTime::fromString( stime, "hh:mm:s" );
        if( !time.isValid() )
            time = QTime::fromString( stime, "hh:m:ss" );
        if( !time.isValid() )
            time = QTime::fromString( stime, "hh:m:s" );
        if( !time.isValid() )
            time = QTime::fromString( stime, "h:mm:s" );
        if( !time.isValid() )
            time = QTime::fromString( stime, "h:m:ss" );
        if( !time.isValid() )
            time = QTime::fromString( stime, "h:m:s" );
        if( !time.isValid() )
            time = QTime::fromString( stime, "hh:mm" );
        if( !time.isValid() )
            time = QTime::fromString( stime, "hh:m" );
        if( !time.isValid() )
            time = QTime::fromString( stime, "h:mm" );
        if( !time.isValid() )
            time = QTime::fromString( stime, "h:m" );
        if( !time.isValid() )
            time = QTime::fromString( stime, "hh" );
        if( !time.isValid() )
            time = QTime::fromString( stime, "h" );

        if( time.isValid() )
        {
            issue.estimatedHours = time.hour() + (double)time.minute()/60 + (double)time.second()/3600;
        }
        else
        {
            message( tr("Invalid time format, expecting hh:mm:ss "), QtCriticalMsg );
            RETURN();
        }
    }

    cancelOnClose_ = false;

    issue.project.id = projectId_;
    issue.tracker.id = trackerId_;
    issue.assignedTo.id = currentUserId_;

    if( categoryId_ != NULL_ID )
        issue.category.id = categoryId_;

    if( !qml("parentIssue")->property("text").toString().isEmpty() )
        issue.parentId = qml("parentIssue")->property("text").toInt();

    issue.subject = qml("subject")->property("text").toString();
    issue.description = qml("description")->property("text").toString();

    for( const auto& customField : customFields_ )
    {
        QString value;
        if( customField.format == "list" )
            value = customFieldItems_[customField.id].second->property("currentText").toString();
        else
            value = customFieldItems_[customField.id].second->property("text").toString();

        CustomField cf;
        cf.id = customField.id;
        cf.values.push_back( value );

        issue.customFields.push_back( cf );
    }

    ++callbackCounter_;
    redmine_->sendIssue( issue, [=](bool success, int id, RedmineError errorCode, QStringList errors)
    {
        CBENTER();

        DEBUG()(success)(id)(errorCode)(errors);

        if( !success )
        {
            cancelOnClose_ = true;

            QString errorMsg = tr( "Could not create issue." );
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        message( tr("New issue created with ID %1").arg(id) );

        created( id );
        close();

        CBRETURN();
    } );

    RETURN();
}

void
IssueCreator::selectParentIssue()
{
    // Issue selector initialisation
    IssueSelector* issueSelector = new IssueSelector( redmine_, mainWindow() );
    issueSelector->setTransientParent( this );
    if( projectId_ != NULL_ID )
        issueSelector->setProjectId( projectId_, true );
    issueSelector->display();

    // Connect the issue selected signal to the setIssue slot
    connect( issueSelector, &IssueSelector::selected,
             [=](int issueId){ qml("parentIssue")->setProperty( "text", issueId ); } );
}

void
IssueCreator::setCurrentIssue( Issue issue )
{
    ENTER();

    if( issue.id != NULL_ID )
    {
        issue_ = issue;
        useCurrentIssueParent();
    }

    RETURN();
}

void
IssueCreator::setProjectId( int id )
{
    ENTER();

    projectId_ = id;
    loadProjects();

    RETURN();
}

void
IssueCreator::setUseCustomFields( bool useCustomFields )
{
    ENTER();

    useCustomFields_ = useCustomFields;
    refreshGui();

    RETURN();
}

void
IssueCreator::trackerSelected( int index )
{
    ENTER();

    trackerId_ = trackerModel_.at(index).id();
    DEBUG()(index)(trackerId_);

    refreshGui();

    RETURN();
}

void
IssueCreator::useCurrentIssue()
{
    ENTER();

    if( issue_.id != NULL_ID )
        qml("parentIssue")->setProperty( "text", issue_.id );

    RETURN();
}

void
IssueCreator::useCurrentIssueParent()
{
    ENTER();

    if( issue_.parentId != NULL_ID )
        qml("parentIssue")->setProperty( "text", issue_.parentId );

    RETURN();
}
