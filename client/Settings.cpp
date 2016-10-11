#include "Settings.h"
#include "logging.h"

#include <QAbstractButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QQuickItem>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

using namespace qtredmine;
using namespace redtimer;
using namespace std;

Settings::Settings( MainWindow* mainWindow )
    : Window( "Settings", mainWindow,
              [&]()
              {
                  if( isValid(true) )
                      close();
              } ),
      settings_( QSettings::IniFormat, QSettings::UserScope, "Thomssen IT", "RedTimer", this )
{
    ENTER();

    // Connect to Redmine
    redmine_ = new SimpleRedmineClient( this );

    // Settings window initialisation
    setModality( Qt::ApplicationModal );
    setFlags( Qt::Dialog );
    setTitle( "Settings" );

    // Set the models
    ctx_->setContextProperty( "issueStatusModel", &issueStatusModel_ );
    ctx_->setContextProperty( "trackerModel", &trackerModel_ );
    ctx_->setContextProperty( "startTimeModel", &startTimeModel_ );
    ctx_->setContextProperty( "endTimeModel", &endTimeModel_ );

    profilesProxyModel_.setSourceModel( &profilesModel_ );
    profilesProxyModel_.setSortRole( SimpleModel::NameRole );
    profilesProxyModel_.setDynamicSortFilter( true );
    ctx_->setContextProperty( "profilesModel", &profilesProxyModel_ );

    // Connect the profile selector
    connect( qml("profiles"), SIGNAL(activated(int)), this, SLOT(profileSelected(int)) );

    // Connect the create profile button
    connect( qml("createProfile"), SIGNAL(clicked()), this, SLOT(createProfile()) );

    // Connect the delete profile button
    connect( qml("deleteProfile"), SIGNAL(clicked()), this, SLOT(deleteProfile()) );

    // Connect the rename profile button
    connect( qml("renameProfile"), SIGNAL(clicked()), this, SLOT(renameProfile()) );

    // Connect the rename profile button
    connect( qml("useCustomFields"), SIGNAL(clicked()), this, SLOT(toggleCustomFields()) );

    // Connect the cancel button
    connect( qml("cancel"), SIGNAL(clicked()), this, SLOT(cancel()) );

    // Connect the apply button
    connect( qml("apply"), SIGNAL(clicked()), this, SLOT(apply()) );

    // Connect the save button
    connect( qml("save"), SIGNAL(clicked()), this, SLOT(applyAndClose()) );

    RETURN();
}

void
Settings::apply()
{
    ENTER();

    applyProfileData();
    if( !isValid(true) )
        RETURN();

    auto cb = [&](bool success, int id, RedmineError errorCode, QStringList errors)
    {
        CBENTER();

        DEBUG()(success)(id)(errorCode)(errors);

        if( !success )
        {
            QString errorMsg = tr( "Could not save the time entry." );
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);
            message( errorMsg, QtCriticalMsg );

            CBRETURN();
        }

        save();
        data_ = *profileData();

        DEBUG() << "Emitting applied() signal";
        applied();

        CBRETURN();
    };

    // Save current time before applying
    ++callbackCounter_;
    mainWindow_->stop( true, true, cb );

    RETURN();
}

void
Settings::applyAndClose()
{
    ENTER();

    apply();

    if( isValid() )
        close();

    RETURN();
}

void
Settings::applyProfileData()
{
    ENTER();

    if( profileId_ == NULL_ID )
        RETURN();

    ProfileData* data = profileData();

    QString oldUrl = data->url;

    // Connection
    data->apiKey            = qml("apikey")->property("text").toString();
    data->checkConnection   = qml("checkConnection")->property("checked").toBool();
    data->ignoreSslErrors   = qml("ignoreSslErrors")->property("checked").toBool();
    data->numRecentIssues   = qml("numRecentIssues")->property("text").toInt();
    data->url               = qml("url")->property("text").toString();
    data->useCustomFields   = qml("useCustomFields")->property("checked").toBool();

    // Shortcuts
    data->shortcutCreateIssue = qml("shortcutCreateIssue")->property("text").toString();
    data->shortcutSelectIssue = qml("shortcutSelectIssue")->property("text").toString();
    data->shortcutStartStop   = qml("shortcutStartStop")->property("text").toString();
    data->shortcutToggle      = qml("shortcutToggle")->property("text").toString();

    // Interface
    data->useSystemTrayIcon = qml("useSystemTrayIcon")->property("checked").toBool();
    data->closeToTray = qml("closeToTray")->property("checked").toBool();

    if( oldUrl == data->url )
    {
        if( issueStatusModel_.rowCount() )
        {
            int workedOnIndex = qml("workedOn")->property("currentIndex").toInt();
            data->workedOnId = issueStatusModel_.at(workedOnIndex).id();
        }

        if( trackerModel_.rowCount() )
        {
            int defaultTrackerIndex = qml("defaultTracker")->property("currentIndex").toInt();
            data->defaultTrackerId = trackerModel_.at(defaultTrackerIndex).id();
        }

        if( startTimeModel_.rowCount() )
        {
            int startTimeFieldId = qml("startTime")->property("currentIndex").toInt();
            data->startTimeFieldId = startTimeModel_.at(startTimeFieldId).id();
        }

        if( endTimeModel_.rowCount() )
        {
            int endTimeFieldId = qml("endTime")->property("currentIndex").toInt();
            data->endTimeFieldId = endTimeModel_.at(endTimeFieldId).id();
        }
    }
    else
    {
        data->activityId = NULL_ID;
        data->issueId    = NULL_ID;
        data->projectId  = NULL_ID;
        data->workedOnId = NULL_ID;
        data->defaultTrackerId = NULL_ID;
        data->startTimeFieldId = NULL_ID;
        data->endTimeFieldId   = NULL_ID;

        while( !data->recentIssues.isEmpty() )
            data->recentIssues.removeLast();
    }

    data_ = *data;

    RETURN();
}

void
Settings::cancel()
{
    ENTER();

    close();

    // Revert edit changes
    load();

    RETURN();
}

void
Settings::close()
{
    ENTER();

    win_.settings = getWindowData();
    Window::close();

    RETURN();
}

bool
Settings::createProfile()
{
    ENTER();

    int maxId = 0;
    for( const auto& profile : profiles_ )
    {
        if( profile.id > maxId )
            maxId = profile.id;
    }

    QString name;
    if( !getProfileName( name, tr("Create new profile"), "New profile" ) )
        RETURN( false );

    // Save to profiles map and model
    ProfileData data;
    data.id = maxId+1;
    data.name = name;

    int id = maxId + 1;
    loadProfileData( id, &data );
    profileId_ = NULL_ID;

    // Use the newly created profile
    QModelIndex modelIndex = profilesModel_.index( profilesModel_.rowCount() - 1 );
    QModelIndex proxyIndex = profilesProxyModel_.mapFromSource( modelIndex );
    profileSelected( proxyIndex.row() );

    RETURN( true );
}

void
Settings::deleteProfile()
{
    ENTER();

    int index = qml("profiles")->property("currentIndex").toInt();
    QModelIndex proxyIndex = profilesProxyModel_.index( index, 0 );

    int profileId = proxyIndex.data(SimpleModel::IdRole).toInt();
    QString profileName = proxyIndex.data(SimpleModel::NameRole).toString();

    int ret = QMessageBox::question( qobject_cast<QWidget*>(this), tr("Delete profile"),
                                     tr("Do you really want to delete profile '%1'?").arg(profileName) );

    if( ret != QMessageBox::Yes )
        RETURN();

    profileId_ = NULL_ID;
    profiles_.remove( profileId );
    QModelIndex modelIndex = profilesProxyModel_.mapToSource( proxyIndex );
    profilesModel_.removeRow( modelIndex.row() );

    // If no profile exists, ask to create a new one until a profile was successfully created
    if( !profilesModel_.rowCount() )
        while( !createProfile() );

    proxyIndex = profilesProxyModel_.index( 0, 0 );
    if( proxyIndex.isValid() )
        profileSelected( proxyIndex.row() );

    RETURN();
}

void
Settings::display()
{
    ENTER()(profileId_);

    if( profileId_ == NULL_ID )
        load( false );

    // Select current profile
    {
        QModelIndexList indices = profilesProxyModel_.match( profilesProxyModel_.index(0, 0),
                                                             SimpleModel::IdRole, profileId_ );
        qml("profiles")->setProperty( "currentIndex", -1 );
        if( indices.size() )
            qml("profiles")->setProperty( "currentIndex", indices[0].row() );
    }

    qml("apikey")->setProperty( "text", profileData()->apiKey );
    qml("apikey")->setProperty( "cursorPosition", 0 );
    qml("checkConnection")->setProperty( "checked", profileData()->checkConnection );
    qml("closeToTray")->setProperty( "checked", profileData()->closeToTray );
    qml("ignoreSslErrors")->setProperty( "checked", profileData()->ignoreSslErrors );
    qml("numRecentIssues")->setProperty( "text", profileData()->numRecentIssues );
    qml("url")->setProperty( "text", profileData()->url );
    qml("url")->setProperty( "cursorPosition", 0 );
    qml("useCustomFields")->setProperty( "checked", profileData()->useCustomFields );
    qml("useSystemTrayIcon")->setProperty( "checked", profileData()->useSystemTrayIcon );

    qml("shortcutCreateIssue")->setProperty( "text", profileData()->shortcutCreateIssue );
    qml("shortcutSelectIssue")->setProperty( "text", profileData()->shortcutSelectIssue );
    qml("shortcutStartStop")->setProperty( "text", profileData()->shortcutStartStop );
    qml("shortcutToggle")->setProperty( "text", profileData()->shortcutToggle );

    updateIssueStatuses();
    updateTrackers();
    updateTimeEntryCustomFields();

    setWindowData( win_.settings );

    showNormal();
    raise();

    RETURN();
}

bool
Settings::getProfileName( QString& name, QString title, QString initText )
{
    ENTER();

    bool ok;
    name = QInputDialog::getText( qobject_cast<QWidget*>(this), title, tr("Profile name:"),
                                  QLineEdit::Normal, initText, &ok );

    if( !ok )
        RETURN( false );

    if( name.isEmpty() )
    {
        QMessageBox::critical( qobject_cast<QWidget*>(this), tr("Create new profile"),
                               tr("No profile name specified. Aborting.") );
        RETURN( false );
    }

    bool foundName = false;
    for( const auto& profile : profiles_ )
    {
        if( profile.name == name )
        {
            foundName = true;
            break;
        }
    }

    if( foundName )
    {
        QMessageBox::critical( qobject_cast<QWidget*>(this), title,
                               tr("Profile '%1' already exists. Aborting.").arg(name) );
        RETURN( false );
    }

    RETURN( true );
}

bool
Settings::isValid( bool displayError )
{
    ENTER();

    ProfileData data = *profileData();

    bool result = !data.url.isEmpty() && !data.apiKey.isEmpty();

    if( !result && displayError )
        message( "URL and API key required", QtCriticalMsg );

    RETURN( result );
}

void
Settings::load( const QString profile, const bool apply )
{
    ENTER()(profile)(apply);

    // Window settings
    auto loadWindowData = [&]( Window::Data WindowData::*field, QString name )
    {
        ENTER();

        QString prefix = QString("windows/%1/%2").arg(name);

        if( !settings_.value(prefix.arg("geometry")).isNull() )
            (win_.*field).geometry = settings_.value(prefix.arg("geometry")).toRect();
        if( !settings_.value(prefix.arg("position")).isNull() )
            (win_.*field).position = settings_.value(prefix.arg("position")).toPoint();

        DEBUG()((win_.*field).position)((win_.*field).geometry);

        RETURN();
    };

    loadWindowData( &WindowData::issueCreator,  "issueCreator"  );
    loadWindowData( &WindowData::issueSelector, "issueSelector" );
    loadWindowData( &WindowData::mainWindow,    "mainWindow"    );
    loadWindowData( &WindowData::settings,      "settings"      );

    // Profiles
    profiles_.clear();
    profilesModel_.clear();

    QStringList groups = settings_.childGroups();
    for( const auto& group : groups )
    {
        QRegularExpressionMatch match;
        if( !group.contains(QRegularExpression("profile-(\\d+)"), &match) )
            continue;

        int profileId = match.captured(1).toInt();
        loadProfileData( profileId );

        // If a profile was specified, try to load that one
        if( !profile.isEmpty() && profiles_[profileId].name == profile )
            profileId_ = profileId;
    }

    // If no profile exists, ask to create a new one until a profile was successfully created
    if( profiles_.count() == 0 )
    {
        while( !createProfile() );
    }
    else
    {
        profilesModel_.sort( SimpleModel::NameRole );

        QVariant profileId = settings_.value("profileId");
        if( !profileId.isNull() && profileId.toInt()
            && settings_.childGroups().contains(profileHash(profileId.toInt())) )
        {
            profileId_ = profileId.toInt();
        }
    }

    if( profileId_ == NULL_ID )
        profileId_ = profilesModel_.at(0).id();

    if( apply )
    {
        data_ = *profileData();
        applied();
    }

    DEBUG()(profiles_);

    RETURN();
}

void
Settings::load( const bool apply )
{
    ENTER();

    load( QString(), apply );

    RETURN();
}

void
Settings::load()
{
    ENTER();

    load( QString() );

    RETURN();
}

void
Settings::loadProfileData( const int profileId, const ProfileData* initData )
{
    ENTER()(profileId)(initData);

    if( initData )
        DEBUG()(*initData);

    ProfileData data;

    if( initData )
        data = *initData;

    settings_.beginGroup( profileHash(profileId) );

    if( data.id == NULL_ID )
        data.id = profileId;

    if( data.name.isEmpty() )
    {
        if( settings_.value("name").isValid() )
            data.name = settings_.value("name").toString();
        else
            data.name = QString("Profile %1").arg(data.id);
    }

    // Connection
    data.apiKey = settings_.value("apikey").toString();
    data.checkConnection = settings_.value("checkConnection").toBool();
    data.ignoreSslErrors = settings_.value("ignoreSslErrors").toBool();
    data.url = settings_.value("url").toString();

    data.numRecentIssues = settings_.value("numRecentIssues").isValid()
                           ? settings_.value("numRecentIssues").toInt()
                           : 10;

    data.useCustomFields = settings_.value("useCustomFields").isValid()
                           ? settings_.value("useCustomFields").toBool()
                           : true;

    data.activityId  = settings_.value("activity").isValid()
                       ? settings_.value("activity").toInt()
                       : NULL_ID;
    data.issueId = settings_.value("issue").isValid()
                   ? settings_.value("issue").toInt()
                   : NULL_ID;
    data.projectId = settings_.value("project").isValid()
                     ? settings_.value("project").toInt()
                     : NULL_ID;
    data.workedOnId = settings_.value("workedOnId").isValid()
                      ? settings_.value("workedOnId").toInt()
                      : NULL_ID;
    data.defaultTrackerId = settings_.value("defaultTrackerId").isValid()
                            ? settings_.value("defaultTrackerId").toInt()
                            : NULL_ID;

    data.startTimeFieldId = settings_.value("startTimeFieldId").isValid()
                            ? settings_.value("startTimeFieldId").toInt()
                            : NULL_ID;
    data.endTimeFieldId = settings_.value("endTimeFieldId").isValid()
                          ? settings_.value("endTimeFieldId").toInt()
                          : NULL_ID;

    // Shortcuts
    data.shortcutCreateIssue = settings_.value("shortcutCreateIssue").isValid()
                               ? settings_.value("shortcutCreateIssue").toString()
                               : "Ctrl+Alt+C";
    data.shortcutSelectIssue = settings_.value("shortcutSelectIssue").isValid()
                               ? settings_.value("shortcutSelectIssue").toString()
                               : "Ctrl+Alt+L";
    data.shortcutStartStop = settings_.value("shortcutStartStop").isValid()
                             ? settings_.value("shortcutStartStop").toString()
                             : "Ctrl+Alt+S";
    data.shortcutToggle = settings_.value("shortcutToggle").isValid()
                          ? settings_.value("shortcutToggle").toString()
                          : "Ctrl+Alt+R";

    // Interface
    data.useSystemTrayIcon = settings_.value("useSystemTrayIcon").isValid()
                             ? settings_.value("useSystemTrayIcon").toBool()
                             : true;
    data.closeToTray = settings_.value("closeToTray").isValid()
                             ? settings_.value("closeToTray").toBool()
                             : true;

    // Recently used issues
    {
        data.recentIssues.clear();
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
    }

    settings_.endGroup();

    DEBUG()(data);

    profiles_.insert( data.id, data );
    profilesModel_.push_back( SimpleItem(data) );

    RETURN();
}

Settings::ProfileData*
Settings::profileData()
{
    ENTER()(profileId_);
    RETURN( &profiles_[profileId_] );
}

QString
Settings::profileHash( int id )
{
    ENTER()(id)(profileId_);

    if( id == NULL_ID )
        id = profileId_;

    RETURN( QString("profile-%1").arg(id) );
}

void
Settings::profileSelected( int profileIndex )
{
    ENTER();

    applyProfileData();

    QModelIndex proxyIndex = profilesProxyModel_.index( profileIndex, 0 );
    profileId_ = proxyIndex.data(SimpleModel::IdRole).toInt();
    display();

    RETURN();
}

void
Settings::renameProfile()
{
    ENTER();

    int index = qml("profiles")->property("currentIndex").toInt();
    QModelIndex proxyIndex = profilesProxyModel_.index( index, 0 );
    QString profileName = proxyIndex.data(SimpleModel::NameRole).toString();

    QString newProfileName;
    if( !getProfileName( newProfileName, tr("Rename profile"), profileName ) )
        RETURN();

    profileData()->name = newProfileName;
    QModelIndex modelIndex = profilesProxyModel_.mapToSource( proxyIndex );
    profilesModel_.setData( modelIndex, newProfileName, SimpleModel::NameRole );

    RETURN();
}

void
Settings::save()
{
    ENTER();

    settings_.clear();

    profiles_[profileId_] = data_;

    // General settings
    {
        auto saveWindowData = [&]( Window::Data WindowData::*field, QString name )
        {
            ENTER();

            QString prefix = QString("windows/%1/%2").arg(name);

            settings_.setValue( prefix.arg("geometry"), (win_.*field).geometry );
            settings_.setValue( prefix.arg("position"), (win_.*field).position );

            RETURN();
        };

        saveWindowData( &WindowData::issueCreator,  "issueCreator"  );
        saveWindowData( &WindowData::issueSelector, "issueSelector" );
        saveWindowData( &WindowData::mainWindow,    "mainWindow"    );
        saveWindowData( &WindowData::settings,      "settings"      );

        settings_.setValue( "profileId", profileId_ );
    }

    DEBUG()(profiles_);

    for( const auto& profile: profiles_ )
        saveProfileData( profile.id );

    settings_.sync();

    RETURN();
}

void
Settings::saveProfileData( int profileId )
{
    ENTER()(profileId);

    if( !profileId )
        RETURN();

    ProfileData data = profiles_[profileId];

    settings_.beginGroup( profileHash(profileId) );

    settings_.setValue( "id", data.id );
    settings_.setValue( "name", data.name );

    // Connection
    {
        settings_.setValue( "apikey",            data.apiKey );
        settings_.setValue( "checkConnection",   data.checkConnection );
        settings_.setValue( "ignoreSslErrors",   data.ignoreSslErrors );
        settings_.setValue( "numRecentIssues",   data.numRecentIssues );
        settings_.setValue( "url",               data.url );
        settings_.setValue( "useCustomFields",   data.useCustomFields );
        settings_.setValue( "workedOnId",        data.workedOnId );
        settings_.setValue( "defaultTrackerId",  data.defaultTrackerId );
        settings_.setValue( "startTimeFieldId",  data.startTimeFieldId );
        settings_.setValue( "endTimeFieldId",    data.endTimeFieldId );

        settings_.setValue( "activity", data.activityId );
        settings_.setValue( "issue",    data.issueId );
        settings_.setValue( "project",  data.projectId );
    }

    // Shortcuts
    settings_.setValue("shortcutCreateIssue", data.shortcutCreateIssue );
    settings_.setValue("shortcutSelectIssue", data.shortcutSelectIssue );
    settings_.setValue("shortcutStartStop",   data.shortcutStartStop );
    settings_.setValue("shortcutToggle",      data.shortcutToggle );

    // Interface
    settings_.setValue( "useSystemTrayIcon", data.useSystemTrayIcon );
    settings_.setValue( "closeToTray", data.closeToTray );

    // Recently used issues for the data
    {
        settings_.beginWriteArray( "recentIssues" );
        for( int i = 0; i < data.recentIssues.size(); ++i )
        {
            settings_.setArrayIndex( i );
            settings_.setValue( "id",      data.recentIssues.at(i).id );
            settings_.setValue( "subject", data.recentIssues.at(i).subject );
        }
        settings_.endArray();
    }

    settings_.endGroup();

    RETURN();
}

void
Settings::toggleCustomFields()
{
    ENTER();

    updateTimeEntryCustomFields();

    RETURN();
}

void
Settings::updateIssueStatuses()
{
    ENTER();

    if( !isValid() )
    {
        issueStatusModel_.clear();
        issueStatusModel_.push_back( SimpleItem(NULL_ID, "URL and API key required") );

        qml("workedOn")->setProperty( "enabled", false );
        qml("workedOn")->setProperty( "currentIndex", -1 );
        qml("workedOn")->setProperty( "currentIndex", 0 );

        RETURN();
    }

    redmine_->setUrl( profileData()->url );
    redmine_->setAuthenticator( profileData()->apiKey );
    if( profileData()->ignoreSslErrors )
        redmine_->setCheckSsl( false );
    else
        redmine_->setCheckSsl( true );

    ++callbackCounter_;
    redmine_->retrieveIssueStatuses( [&]( IssueStatuses issueStatuses, RedmineError redmineError,
                                          QStringList errors )
    {
        CBENTER();

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load issue statuses.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        int currentIndex = 0;

        // Sort issues ascending by ID
        qSort( issueStatuses.begin(), issueStatuses.end(),
               []( const IssueStatus& a, const IssueStatus& b ){ return a.id < b.id; } );

        issueStatusModel_.clear();
        issueStatusModel_.push_back( SimpleItem(NULL_ID, "Choose issue status") );
        for( const auto& issueStatus : issueStatuses )
        {
            if( issueStatus.id == profileData()->workedOnId )
                currentIndex = issueStatusModel_.rowCount();

            issueStatusModel_.push_back( SimpleItem(issueStatus) );
        }

        DEBUG()(issueStatusModel_)(profileData()->workedOnId)(currentIndex);

        qml("workedOn")->setProperty( "enabled", true );
        qml("workedOn")->setProperty( "currentIndex", -1 );
        qml("workedOn")->setProperty( "currentIndex", currentIndex );

        CBRETURN();
    } );

    RETURN();
}

void
Settings::updateTimeEntryCustomFields()
{
    ENTER();

    bool useCustomFields = qml("useCustomFields")->property("checked").toBool();

    if( !isValid() || !useCustomFields )
    {
        QString err;

        if( useCustomFields )
            err = "URL and API key required";
        else
            err = "Custom fields not enabled";

        startTimeModel_.clear();
        startTimeModel_.push_back( SimpleItem(NULL_ID, err) );
        qml("startTime")->setProperty( "enabled", false );
        qml("startTime")->setProperty( "currentIndex", -1 );
        qml("startTime")->setProperty( "currentIndex", 0 );

        endTimeModel_.clear();
        endTimeModel_.push_back( SimpleItem(NULL_ID, err) );
        qml("endTime")->setProperty( "enabled", false );
        qml("endTime")->setProperty( "currentIndex", -1 );
        qml("endTime")->setProperty( "currentIndex", 0 );

        RETURN();
    }

    CustomFieldFilter filter;
    filter.format = "string";
    filter.type   = "time_entry";

    redmine_->setUrl( profileData()->url );
    redmine_->setAuthenticator( profileData()->apiKey );
    if( profileData()->ignoreSslErrors )
        redmine_->setCheckSsl( false );
    else
        redmine_->setCheckSsl( true );


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

        QString firstEntry;

        if( customFields.size() )
            firstEntry = "Choose time entry field";
        else
            firstEntry = "No time entry fields found";

        startTimeModel_.clear();
        endTimeModel_.clear();

        int startTimeCurrentIndex = 0;
        int endTimeCurrentIndex = 0;

        startTimeModel_.push_back( SimpleItem(NULL_ID, firstEntry) );
        endTimeModel_.push_back( SimpleItem(NULL_ID, firstEntry) );

        sort( customFields.begin(), customFields.end(),
              [](CustomField l, CustomField r){ return l.name < r.name;} );

        // Create loaded custom fields
        for( const auto& customField : customFields )
        {
            if( customField.id == profileData()->startTimeFieldId )
                startTimeCurrentIndex = startTimeModel_.rowCount();
            startTimeModel_.push_back( SimpleItem(customField) );

            if( customField.id == profileData()->endTimeFieldId )
                endTimeCurrentIndex = endTimeModel_.rowCount();
            endTimeModel_.push_back( SimpleItem(customField) );
        }

        qml("startTime")->setProperty( "currentIndex", -1 );
        qml("startTime")->setProperty( "currentIndex", startTimeCurrentIndex );
        qml("startTime")->setProperty( "enabled", true );
        qml("endTime")->setProperty( "currentIndex", -1 );
        qml("endTime")->setProperty( "currentIndex", endTimeCurrentIndex );
        qml("endTime")->setProperty( "enabled", true );

        CBRETURN();
    },
    filter );

    RETURN();
}

void
Settings::updateTrackers()
{
    ENTER();

    if( !isValid() )
    {
        trackerModel_.clear();
        trackerModel_.push_back( SimpleItem(NULL_ID, "URL and API key required") );

        qml("defaultTracker")->setProperty( "enabled", false );
        qml("defaultTracker")->setProperty( "currentIndex", -1 );
        qml("defaultTracker")->setProperty( "currentIndex", 0 );

        RETURN();
    }

    redmine_->setUrl( profileData()->url );
    redmine_->setAuthenticator( profileData()->apiKey );
    if( profileData()->ignoreSslErrors )
        redmine_->setCheckSsl( false );
    else
        redmine_->setCheckSsl( true );

    ++callbackCounter_;
    redmine_->retrieveTrackers( [&]( Trackers trackers, RedmineError redmineError, QStringList errors )
    {
        CBENTER();

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load trackers.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        int currentIndex = 0;

        // Sort issues ascending by ID
        qSort( trackers.begin(), trackers.end(),
               []( const Tracker& a, const Tracker& b ){ return a.id < b.id; } );

        trackerModel_.clear();
        trackerModel_.push_back( SimpleItem(NULL_ID, "Choose tracker") );
        for( const auto& tracker : trackers )
        {
            if( tracker.id == profileData()->defaultTrackerId )
                currentIndex = trackerModel_.rowCount();

            trackerModel_.push_back( SimpleItem(tracker) );
        }

        DEBUG()(trackerModel_)(profileData()->defaultTrackerId)(currentIndex);

        qml("defaultTracker")->setProperty( "enabled", true );
        qml("defaultTracker")->setProperty( "currentIndex", -1 );
        qml("defaultTracker")->setProperty( "currentIndex", currentIndex );

        CBRETURN();
    } );

    RETURN();
}
