#include "qtredmine/Logging.h"

#include "IssueCreator.h"
#include "IssueSelector.h"
#include "Settings.h"

#include <QMessageBox>
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

    QSortFilterProxyModel* versionProxyModel = new QSortFilterProxyModel( this );
    versionProxyModel->setSourceModel( &versionModel_ );
    versionProxyModel->setSortRole( SimpleModel::SimpleRoles::IdRole );
    ctx_->setContextProperty( "versionModel", versionProxyModel );

    // Load current user
    loadCurrentUser();

    // Connect the project selected signal to the projectSelected slot
    connect( qml("project"), SIGNAL(activated(int)), this, SLOT(projectSelected(int)) );

    // Connect the tracker selected signal to the trackerSelected slot
    connect( qml("tracker"), SIGNAL(activated(int)), this, SLOT(trackerSelected(int)) );

    // Connect the category selected signal to the categorySelected slot
    connect( qml("category"), SIGNAL(activated(int)), this, SLOT(categorySelected(int)) );

    // Connect the category selected signal to the categorySelected slot
    connect( qml("category"), SIGNAL(activated(int)), this, SLOT(categorySelected(int)) );

    // Connect the parent issue text field enter event
    connect( qml("parentIssue"), SIGNAL(accepted()), this, SLOT(loadParentIssueData()) );

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
    ENTER()(index);

    categoryId_ = categoryModel_.at(index).id();
    DEBUG()(categoryId_);

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

    if( !mainWindow_->connected() )
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
            int currentIndex = 0;

            categoryModel_.push_back( SimpleItem(NULL_ID, "") );
            for( const auto& category : project.categories )
            {
                if( category.id == categoryId_ )
                    currentIndex = categoryModel_.rowCount();

                categoryModel_.push_back( SimpleItem(category) );
            }

            qml("category")->setProperty( "currentIndex", -1 );
            qml("category")->setProperty( "currentIndex", currentIndex );
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

    if( !mainWindow_->connected() )
        RETURN();

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

    if( !mainWindow_->connected() )
        RETURN();

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

        sort( customFields.begin(), customFields.end(),
              [](const CustomField& a, const CustomField& b){return a.name > b.name;} );

        // Create loaded custom fields
        for( const auto& customField : customFields )
        {
            DEBUG()(customField);
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

            int currentIndex = 0;

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
                    QString entryFieldModel = entryFieldId;
                    entryFieldModel.append("Model");

                    s << "ComboBox {" << endl;
                    s <<   entryFieldIds << endl;
                    s <<   "model: " << entryFieldModel << endl;
                    s <<   "textRole: \"name\"" << endl;
                    s <<   layout << endl;
                    s << "}" << endl;

                    customFieldModels_[customField.id] = new SimpleModel( this );
                    customFieldModels_[customField.id]->push_back( SimpleItem(NULL_ID, "") );

                    for( const auto& possibleValue : customField.possibleValues )
                    {
                        DEBUG()(possibleValue);

                        if( customFieldValues_.count(customField.id) )
                        {
                            for( const auto& value : customFieldValues_[customField.id] )
                            {
                                DEBUG()(value);
                                if( value == possibleValue )
                                    currentIndex = customFieldModels_[customField.id]->rowCount();
                                DEBUG()(currentIndex);
                            }
                        }

                        customFieldModels_[customField.id]->push_back( SimpleItem(NULL_ID, possibleValue) );
                    }

                    ctx_->setContextProperty( entryFieldModel, customFieldModels_[customField.id] );
                }
                else
                {
                    s << "TextField {" << endl;
                    s <<   entryFieldIds << endl;
                    s <<   layout << endl;

                    if( customFieldValues_.count(customField.id) )
                        s << "  text: \"" << customFieldValues_[customField.id][0] << "\"" << endl;

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

            if( customField.format == "list" )
            {
                entryFieldItem->setProperty( "currentIndex", -1 );
                entryFieldItem->setProperty( "currentIndex", currentIndex );
            }
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
IssueCreator::loadParentIssueData()
{
    ENTER();

    if( qml("parentIssue")->property("text").toString().isEmpty() )
        RETURN();

    int issueId = qml("parentIssue")->property("text").toInt();

    if( !issueId )
        RETURN();

    bool loadData = true;

    if( parentIssueInit_ )
    {
        int ret = QMessageBox::question( qobject_cast<QWidget*>(this), tr("Load data"),
                                         tr("Do you want to load parent issue data?") );

        if( ret != QMessageBox::Yes )
            loadData = false;
    }

    ++callbackCounter_;
    redmine_->retrieveIssue( [=]( Issue issue, RedmineError redmineError, QStringList errors )
    {
        CBENTER()(issue);

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load issue.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        qml("parentIssue")->setProperty( "text", QString("%1 (%2)").arg(issue.subject).arg(issue.id) );
        qml("parentIssue")->setProperty( "cursorPosition", 0 );

        // Leave after loading the parent issue name
        if( !loadData )
            CBRETURN();

        categoryId_ = issue.category.id;
        loadCategories();

        versionId_ = issue.version.id;
        loadVersions();

        customFieldValues_.clear();
        for( const auto& customField : issue.customFields )
        {
            if( !customField.values.size()
                || (customField.values.size() == 1 && customField.values[0].isEmpty()) )
                continue;

            customFieldValues_.insert( customField.id, customField.values );
        }
        loadCustomFields();

        parentIssueInit_ = true;

        CBRETURN();
    },
    issueId );

    RETURN();
}

void
IssueCreator::loadProjects()
{
    ENTER();

    if( !mainWindow_->connected() )
        RETURN();

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

    if( !mainWindow_->connected() )
        RETURN();

    ++callbackCounter_;
    redmine_->retrieveProject( [&]( Project project, RedmineError redmineError, QStringList errors )
    {
        CBENTER()(project)(redmineError)(errors);

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load projects.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        trackerId_ = mainWindow_->settings_->data_.defaultTrackerId;
        int currentIndex = 0;

        trackerModel_.clear();
        trackerModel_.push_back( SimpleItem(NULL_ID, "Choose tracker") );
        for( const auto& tracker : project.trackers )
        {
            if( tracker.id == trackerId_ )
                currentIndex = trackerModel_.rowCount();

            trackerModel_.push_back( SimpleItem(tracker) );
        }

        qml("tracker")->setProperty( "currentIndex", -1 );
        qml("tracker")->setProperty( "currentIndex", currentIndex );
        qml("tracker")->setProperty( "enabled", true );

        CBRETURN();
    },
    projectId_ );

    RETURN();
}

void
IssueCreator::loadVersions()
{
    ENTER()(projectId_);

    if( projectId_ == NULL_ID )
        RETURN();

    if( !mainWindow_->connected() )
        RETURN();

    ++callbackCounter_;
    redmine_->retrieveVersions( [=]( Versions versions, RedmineError redmineError, QStringList errors )
    {
        CBENTER();

        if( redmineError != NO_ERROR )
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
            if( version.id == versionId_ )
                currentIndex = versionModel_.rowCount();

            versionModel_.push_back( SimpleItem(version) );
        }

        DEBUG()(versionModel_)(currentIndex);

        qml("version")->setProperty( "currentIndex", -1 );
        qml("version")->setProperty( "currentIndex", currentIndex );
        qml("version")->setProperty( "enabled", true );

        qml("subject")->setProperty( "enabled", true );
        qml("parentIssue")->setProperty( "enabled", true );
        qml("useCurrentIssue")->setProperty( "enabled", true );
        qml("useCurrentIssueParent")->setProperty( "enabled", true );
        qml("selectParentIssue")->setProperty( "enabled", true );
        qml("estimatedTime")->setProperty( "enabled", true );
        qml("description")->setProperty( "enabled", true );
        qml("create")->setProperty( "enabled", true );

        CBRETURN();
    },
    projectId_,
    QString("limit=100") );

    RETURN();
}

void
IssueCreator::projectSelected( int index )
{
    ENTER()(index);

    int oldProjectId = projectId_;

    projectId_ = projectModel_.at(index).id();
    DEBUG()(projectId_);

    if( oldProjectId != projectId_ )
        qml("parentIssue")->setProperty( "text", "" );

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
    if( projectId_ == NULL_ID )
        RETURN();

    loadTrackers();
    loadCategories();
    loadVersions();
    loadCustomFields();

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

    QTime time = SimpleRedmineClient::getTime( qml("estimatedTime")->property("text").toString() );
    if( time.isValid() )
    {
        issue.estimatedHours = time.hour() + (double)time.minute()/60 + (double)time.second()/3600;
    }
    else
    {
        message( tr("Invalid time format, expecting hh:mm:ss "), QtCriticalMsg );
        RETURN();
    }

    cancelOnClose_ = false;

    issue.project.id = projectId_;
    issue.tracker.id = trackerId_;
    issue.assignedTo.id = currentUserId_;

    if( categoryId_ != NULL_ID )
        issue.category.id = categoryId_;

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
    connect( issueSelector, &IssueSelector::selected, [=](int issueId)
    {
        qml("parentIssue")->setProperty( "text", issueId );
        loadParentIssueData();
    } );
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
    ENTER()(index);

    trackerId_ = trackerModel_.at(index).id();
    DEBUG()(trackerId_);

    refreshGui();

    RETURN();
}

void
IssueCreator::useCurrentIssue()
{
    ENTER();

    if( issue_.id == NULL_ID )
        RETURN();

    qml("parentIssue")->setProperty( "text", issue_.id );
    loadParentIssueData();

    RETURN();
}

void
IssueCreator::useCurrentIssueParent()
{
    ENTER();

    if( issue_.parentId == NULL_ID )
        RETURN();

    qml("parentIssue")->setProperty( "text", issue_.parentId );
    loadParentIssueData();

    RETURN();
}

void
IssueCreator::versionSelected( int index )
{
    ENTER()(index);

    versionId_ = versionModel_.at(index).id();
    DEBUG()(versionId_);

    RETURN();
}
