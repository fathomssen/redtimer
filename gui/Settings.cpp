#include "qtredmine/Logging.h"

#include "Settings.h"

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
    : Window( "Settings", mainWindow ),
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
    setCtxProperty( "issueStatusModel", &issueStatusModel_ );
    setCtxProperty( "trackerModel", &trackerModel_ );
    setCtxProperty( "startTimeModel", &startTimeModel_ );
    setCtxProperty( "endTimeModel", &endTimeModel_ );

    profilesProxyModel_.setSourceModel( &profilesModel_ );
    profilesProxyModel_.setSortRole( SimpleModel::NameRole );
    profilesProxyModel_.setDynamicSortFilter( true );
    setCtxProperty( "profilesModel", &profilesModel_ );

    // Connect the profile selector
    connect( qml("profiles"), SIGNAL(activated(int)), this, SLOT(profileSelected(int)) );

    // Connect the create profile button
    connect( qml("createProfile"), SIGNAL(clicked()), this, SLOT(createProfile()) );

    // Connect the copy profile button
    connect( qml("copyProfile"), SIGNAL(clicked()), this, SLOT(copyProfile()) );

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

    load();

    RETURN();
}

void
Settings::apply()
{
    ENTER();

    bool reload = false;
    applyProfileData( &reload );

    QString errmsg;
    if( !profileData()->isValid( &errmsg ) )
        message( errmsg, QtWarningMsg );

    if( reload && profileId() == profileId_ )
    {
        auto cb = [=](bool success, int id, RedmineError errorCode, QStringList errors)
        {
            CBENTER()(success)(id)(errorCode)(errors);

            if( !success )
            {
                QString errorMsg = tr( "Could not save the time entry." );
                for( const auto& error : errors )
                    errorMsg.append("\n").append(error);
                message( errorMsg, QtCriticalMsg );

                CBRETURN();
            }

            save();

            DEBUG() << "Emitting applied() signal";
            applied();

            CBRETURN();
        };

        // Save current time before applying
        ++callbackCounter_;
        mainWindow()->stop( true, true, cb );
    }
    else
        applied();

    RETURN();
}

void
Settings::applyAndClose()
{
    ENTER();

    apply();
    close();

    RETURN();
}

void
Settings::applyProfileData( bool* reload )
{
    ENTER()(profileId_);

    if( profileId_ == NULL_ID )
        RETURN();

    ProfileData* data = profileData();

    QString oldApiKey = data->apiKey;
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

    bool loginUnchanged = oldApiKey == data->apiKey && oldUrl == data->url;
    if( reload )
        *reload = !loginUnchanged;

    if( loginUnchanged )
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

    redmine_->setUrl( data->url );
    redmine_->setAuthenticator( data->apiKey );
    redmine_->setCheckSsl( !data->ignoreSslErrors );

    updateIssueStatuses();
    updateTrackers();
    updateTimeEntryCustomFields();

    RETURN();
}

void
Settings::cancel()
{
    ENTER();

    close();

    // Revert edit changes
    load( false, false );

    RETURN();
}

void
Settings::close()
{
    ENTER();

    data_.windows.settings = getWindowData();
    Window::close();

    RETURN();
}

bool
Settings::copyProfile()
{
    ENTER()(profileId_);
    bool ret = createProfile( true );
    RETURN( ret );
}

bool
Settings::createProfile( bool copy, bool force )
{
    ENTER()(copy);

    int maxId = 0;
    for( const auto& profile : data_.profiles )
        if( profile.id > maxId )
            maxId = profile.id;

    QString name = "New profile";
    if( !profileNameDialog( name, tr("Create new profile"), name ) && !force )
        RETURN( false );

    // Save to profiles map and model
    ProfileData data;

    if( copy )
        data = *profileData();

    data.id = maxId + 1;
    data.name = name;

    DEBUG()(data);

    if( copy )
    {
        data.recentIssues.clear();
        data_.profiles.insert( data.id, data );
        profilesModel_.push_back( SimpleItem(data) );
    }
    else
    {
        // @improve Replace by initProfileData
        loadProfileData( data.id, &data );
    }

    // Use the newly created profile
    profileId_ = NULL_ID;
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
    data_.profiles.remove( profileId );
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
Settings::display( bool loadMainProfile )
{
    ENTER()(profileId_);

    // Load main window's profile ID
    if( loadMainProfile )
        profileId_ = mainWindow()->profileId();

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

    setWindowData( data_.windows.settings );

    showNormal();
    raise();

    RETURN();
}

bool
Settings::initialised()
{
    ENTER();
    RETURN( initialised_ );
}

void
Settings::load( const bool apply, const bool createNewProfile )
{
    ENTER()(apply);

    // Window settings
    auto loadWindowData = [&]( Window::Data WindowData::*field, QString name )
    {
        ENTER();

        QString prefix = QString("windows/%1/%2").arg(name);

        if( !settings_.value(prefix.arg("geometry")).isNull() )
            (data_.windows.*field).geometry = settings_.value(prefix.arg("geometry")).toRect();
        if( !settings_.value(prefix.arg("position")).isNull() )
            (data_.windows.*field).position = settings_.value(prefix.arg("position")).toPoint();

        DEBUG()((data_.windows.*field).position)((data_.windows.*field).geometry);

        RETURN();
    };

    loadWindowData( &WindowData::issueCreator,  "issueCreator"  );
    loadWindowData( &WindowData::issueSelector, "issueSelector" );
    loadWindowData( &WindowData::mainWindow,    "mainWindow"    );
    loadWindowData( &WindowData::settings,      "settings"      );

    // Profiles
    data_.profiles.clear();
    profilesModel_.clear();

    QStringList groups = settings_.childGroups();
    for( const auto& group : groups )
    {
        QRegularExpressionMatch match;
        if( !group.contains(QRegularExpression("profile-(\\d+)"), &match) )
            continue;

        int profileId = match.captured(1).toInt();
        loadProfileData( profileId );
    }

    // If no profile exists, ask to create a new one until a profile was successfully created
    if( data_.profiles.count() == 0 )
    {
        if( createNewProfile )
            while( !createProfile(false, true) );
    }
    else
    {
        profilesModel_.sort( SimpleModel::NameRole );

        QVariant profileId = settings_.value("profileId");
        if( !profileId.isNull() && profileId.toInt()
            && settings_.childGroups().contains(profileHash(profileId.toInt())) )
        {
            profileId_ = profileId.toInt();
            data_.profileId = profileId.toInt();
        }
    }

    if( profileId_ == NULL_ID && profilesModel_.rowCount() )
        profileId_ = profilesModel_.at(0).id();

    if( apply )
        applied();

    DEBUG()(data_.profiles);

    RETURN();
}

void
Settings::loadProfileData( const int profileId, ProfileData* initData )
{
    ENTER()(profileId)(initData);

    ProfileData data;

    if( initData )
    {
        DEBUG()(*initData);
        data = *initData;
    }

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

    profileId_ = data.id;
    data_.profiles.insert( data.id, data );
    profilesModel_.push_back( SimpleItem(data) );

    RETURN();
}

ProfileData*
Settings::profileData()
{
    ENTER()(profileId_);

    ProfileData* data = &data_.profiles[profileId_];

    RETURN( data, *data );
}

ProfileData*
Settings::profileData( int profileId )
{
    ENTER()(profileId);
    DEBUG()(data_);
    RETURN( &data_.profiles[profileId] );
}

QString
Settings::profileHash( int id )
{
    ENTER()(id)(data_.profileId);

    if( id == NULL_ID )
        id = data_.profileId;

    RETURN( QString("profile-%1").arg(id) );
}

int
Settings::profileId()
{
    ENTER()(data_.profileId)(profileId_);

    int profileId;

    if( data_.profileId == NULL_ID )
        profileId = profileId_;
    else
        profileId = data_.profileId;

    RETURN( profileId );
}

bool
Settings::profileNameDialog( QString& name, QString title, QString initText )
{
    ENTER();

    bool ok;
    QString newProfile = QInputDialog::getText( qobject_cast<QWidget*>(this), title, tr("Profile name:"),
                                                QLineEdit::Normal, initText, &ok );

    if( !ok )
        RETURN( false );

    if( newProfile.isEmpty() )
    {
        QMessageBox::critical( qobject_cast<QWidget*>(this), tr("Create new profile"),
                               tr("No profile name specified. Aborting.") );
        RETURN( false );
    }

    bool foundName = false;
    for( const auto& profile : data_.profiles )
    {
        if( profile.name == newProfile )
        {
            foundName = true;
            break;
        }
    }

    if( foundName )
    {
        QMessageBox::critical( qobject_cast<QWidget*>(this), title,
                               tr("Profile '%1' already exists. Aborting.").arg(newProfile) );
        RETURN( false );
    }

    name = newProfile;

    RETURN( true );
}

QMap<int, ProfileData>
Settings::profiles()
{
    ENTER();
    RETURN( data_.profiles );
}

void
Settings::profileSelected( int profileIndex )
{
    ENTER();

    applyProfileData();

    QModelIndex proxyIndex = profilesProxyModel_.index( profileIndex, 0 );
    profileId_ = proxyIndex.data(SimpleModel::IdRole).toInt();
    display( false );

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
    if( !profileNameDialog( newProfileName, tr("Rename profile"), profileName ) )
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

    // General settings
    {
        auto saveWindowData = [&]( Window::Data WindowData::*field, QString name )
        {
            ENTER();

            QString prefix = QString("windows/%1/%2").arg(name);

            settings_.setValue( prefix.arg("geometry"), (data_.windows.*field).geometry );
            settings_.setValue( prefix.arg("position"), (data_.windows.*field).position );

            RETURN();
        };

        saveWindowData( &WindowData::issueCreator,  "issueCreator"  );
        saveWindowData( &WindowData::issueSelector, "issueSelector" );
        saveWindowData( &WindowData::mainWindow,    "mainWindow"    );
        saveWindowData( &WindowData::settings,      "settings"      );

        settings_.setValue( "profileId", data_.profileId );
    }

    DEBUG()(data_.profiles);

    for( const auto& profile: data_.profiles )
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

    ProfileData* data = &data_.profiles[profileId];

    settings_.beginGroup( profileHash(profileId) );

    settings_.setValue( "id", data->id );
    settings_.setValue( "name", data->name );

    // Connection
    {
        settings_.setValue( "apikey",            data->apiKey );
        settings_.setValue( "checkConnection",   data->checkConnection );
        settings_.setValue( "ignoreSslErrors",   data->ignoreSslErrors );
        settings_.setValue( "numRecentIssues",   data->numRecentIssues );
        settings_.setValue( "url",               data->url );
        settings_.setValue( "useCustomFields",   data->useCustomFields );
        settings_.setValue( "workedOnId",        data->workedOnId );
        settings_.setValue( "defaultTrackerId",  data->defaultTrackerId );
        settings_.setValue( "startTimeFieldId",  data->startTimeFieldId );
        settings_.setValue( "endTimeFieldId",    data->endTimeFieldId );

        settings_.setValue( "activity", data->activityId );
        settings_.setValue( "issue",    data->issueId );
        settings_.setValue( "project",  data->projectId );
    }

    // Shortcuts
    settings_.setValue("shortcutCreateIssue", data->shortcutCreateIssue );
    settings_.setValue("shortcutSelectIssue", data->shortcutSelectIssue );
    settings_.setValue("shortcutStartStop",   data->shortcutStartStop );
    settings_.setValue("shortcutToggle",      data->shortcutToggle );

    // Interface
    settings_.setValue( "useSystemTrayIcon", data->useSystemTrayIcon );
    settings_.setValue( "closeToTray", data->closeToTray );

    // Recently used issues for the data
    {
        settings_.beginWriteArray( "recentIssues" );
        for( int i = 0; i < data->recentIssues.size(); ++i )
        {
            settings_.setArrayIndex( i );
            settings_.setValue( "id",      data->recentIssues.at(i).id );
            settings_.setValue( "subject", data->recentIssues.at(i).subject );
        }
        settings_.endArray();
    }

    settings_.endGroup();

    RETURN();
}

void
Settings::setProfileId( int profileId )
{
    ENTER()(profileId);

    data_.profileId = profileId;
    profileId_ = profileId;

    RETURN();
}

void
Settings::setProfileId( QString profileName )
{
    ENTER()(profileName);

    for( const auto& profile : data_.profiles )
    {
        if( profile.name == profileName )
        {
            data_.profileId = profile.id;
            profileId_ = profile.id;
        }
    }

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

    if( !profileData()->isValid() )
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

    if( !connected() )
        RETURN();

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

    if( !profileData()->isValid() || !useCustomFields )
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

    if( !connected() )
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

    if( !profileData()->isValid() )
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

    if( !connected() )
        RETURN();

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

WindowData*
Settings::windowData()
{
    ENTER();
    RETURN( &data_.windows, data_.windows );
}
