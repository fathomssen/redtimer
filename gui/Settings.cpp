#include "qtredmine/Logging.h"

#include "Settings.h"

#include <QAbstractButton>
#include <QInputDialog>
#include <QQuickItem>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

using namespace qtredmine;
using namespace std;

namespace redtimer {

bool
ProfileData::isValid( QString* errmsg ) const
{
    ENTER();

    bool result = !url.isEmpty() && !apiKey.isEmpty();

    if( !result && errmsg )
        *errmsg = "Redmine URL and API key required";

    RETURN( result );
}

Settings::Settings( MainWindow* mainWindow, const QString& profile )
    : Window( "Settings", mainWindow ),
      settings_( QSettings::IniFormat, QSettings::UserScope, "Thomssen IT", "RedTimer", this )
{
    ENTER();

    // Find profile ID by name
    int maxProfileId = 0;
    for( const auto& group : settings_.childGroups() )
    {
        QRegularExpressionMatch match = QRegularExpression("profile-(\\d+)").match( group );

        bool ok;
        int profileId = match.captured(1).toInt( &ok );

        // Not a profile group entry
        if( !ok )
            continue;

        if( profileId > maxProfileId )
            maxProfileId = profileId;

        if( settings_.value(group+"/name").toString().toLower() == profile.toLower() )
        {
            data_.profileData.id = profileId;
            data_.profileData.name = settings_.value(group+"/name").toString();
            break;
        }
    }

    if( data_.profileData.id == NULL_ID )
    {
        // Create new profile
        data_.profileData.id = maxProfileId + 1;
        data_.profileData.name = profile;
    }

    // Connect to Redmine
    redmine_ = new SimpleRedmineClient( this );

    // Settings window initialisation
    setModality( Qt::ApplicationModal );
    setFlags( Qt::Dialog );
    setTitle( "Settings" );

    // Set the models
    setCtxProperty( "issueStatusModel", &issueStatusModel_ );
    setCtxProperty( "trackerModel",     &trackerModel_ );
    setCtxProperty( "externalIdModel",  &externalIdModel_ );
    setCtxProperty( "startTimeModel",   &startTimeModel_ );
    setCtxProperty( "endTimeModel",     &endTimeModel_ );

    // Connect the use custom fields checkbox
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

    if( reload )
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
            emit applied();

            CBRETURN();
        };

        // Save current time before applying
        ++callbackCounter_;
        mainWindow()->stop( true, true, cb );
    }
    else
    {
        save();
        emit applied();
    }

    refresh();

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
    ENTER();

    ProfileData* data = &data_.profileData;

    QString oldApiKey = data->apiKey;
    QString oldUrl = data->url;

    // Connection
    data->apiKey            = qml("apikey")->property("text").toString();
    data->ignoreSslErrors   = qml("ignoreSslErrors")->property("checked").toBool();
    data->numRecentIssues   = qml("numRecentIssues")->property("text").toInt();
    data->startLocalServer  = qml("startLocalServer")->property("checked").toBool();
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

        if( externalIdModel_.rowCount() )
        {
            int externalIdFieldId = qml("externalId")->property("currentIndex").toInt();
            data->externalIdFieldId = externalIdModel_.at(externalIdFieldId).id();
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
        data->externalIdFieldId = NULL_ID;
        data->startTimeFieldId = NULL_ID;
        data->endTimeFieldId   = NULL_ID;

        while( !data->recentIssues.isEmpty() )
            data->recentIssues.removeLast();
    }

    redmine_->setCheckSsl( !data->ignoreSslErrors );
    redmine_->setUrl( data->url );
    redmine_->setAuthenticator( data->apiKey );

    updateIssueCustomFields();
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
    load( false );

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

void
Settings::display()
{
    ENTER();

    load( false );
    refresh();

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
Settings::load( const bool apply )
{
    ENTER();

    // Window settings
    auto loadWindowData = [&]( Window::Data WindowData::*field, QString name )
    {
        ENTER();

        QString prefix = QString("profile-%1/%2/%3").arg(data_.profileData.id).arg(name);

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

    loadProfileData();

    if( apply )
        emit applied();

    RETURN();
}

void
Settings::loadProfileData()
{
    ENTER();

    ProfileData* data = &data_.profileData;

    settings_.beginGroup( QString("profile-%1").arg(data_.profileData.id) );

    // Connection
    data->apiKey = settings_.value("apikey").toString();
    data->ignoreSslErrors = settings_.value("ignoreSslErrors").toBool();
    data->url = settings_.value("url").toString();

    data->numRecentIssues = settings_.value("numRecentIssues").isValid()
                           ? settings_.value("numRecentIssues").toInt()
                           : 10;

    data->startLocalServer = settings_.value("startLocalServer").isValid()
                           ? settings_.value("startLocalServer").toBool()
                           : true;

    data->useCustomFields = settings_.value("useCustomFields").isValid()
                           ? settings_.value("useCustomFields").toBool()
                           : true;

    data->activityId  = settings_.value("activity").isValid()
                       ? settings_.value("activity").toInt()
                       : NULL_ID;
    data->issueId = settings_.value("issue").isValid()
                   ? settings_.value("issue").toInt()
                   : NULL_ID;
    data->projectId = settings_.value("project").isValid()
                     ? settings_.value("project").toInt()
                     : NULL_ID;
    data->workedOnId = settings_.value("workedOnId").isValid()
                      ? settings_.value("workedOnId").toInt()
                      : NULL_ID;
    data->defaultTrackerId = settings_.value("defaultTrackerId").isValid()
                            ? settings_.value("defaultTrackerId").toInt()
                            : NULL_ID;

    data->externalIdFieldId = settings_.value("externalIdFieldId").isValid()
                            ? settings_.value("externalIdFieldId").toInt()
                            : NULL_ID;

    data->startTimeFieldId = settings_.value("startTimeFieldId").isValid()
                            ? settings_.value("startTimeFieldId").toInt()
                            : NULL_ID;
    data->endTimeFieldId = settings_.value("endTimeFieldId").isValid()
                          ? settings_.value("endTimeFieldId").toInt()
                          : NULL_ID;

    // Shortcuts
    data->shortcutCreateIssue = settings_.value("shortcutCreateIssue").isValid()
                               ? settings_.value("shortcutCreateIssue").toString()
                               : "Ctrl+Alt+C";
    data->shortcutSelectIssue = settings_.value("shortcutSelectIssue").isValid()
                               ? settings_.value("shortcutSelectIssue").toString()
                               : "Ctrl+Alt+L";
    data->shortcutStartStop = settings_.value("shortcutStartStop").isValid()
                             ? settings_.value("shortcutStartStop").toString()
                             : "Ctrl+Alt+S";
    data->shortcutToggle = settings_.value("shortcutToggle").isValid()
                          ? settings_.value("shortcutToggle").toString()
                          : "Ctrl+Alt+R";

    // Interface
#ifdef Q_OS_MAC
    data->useSystemTrayIcon = true;
    data->closeToTray = true;
#else
    data->useSystemTrayIcon = settings_.value("useSystemTrayIcon").isValid()
                             ? settings_.value("useSystemTrayIcon").toBool()
                             : true;
    data->closeToTray = settings_.value("closeToTray").isValid()
                             ? settings_.value("closeToTray").toBool()
                             : true;
#endif

    // Recently used issues
    data->recentIssues.clear();
    int size = settings_.beginReadArray( "recentIssues" );
    for( int i = 0; i < size; ++i )
    {
        settings_.setArrayIndex( i );

        Issue issue;
        issue.id      = settings_.value("id").toInt();
        issue.subject = settings_.value("subject").toString();
        data->recentIssues.append( issue );
    }
    settings_.endArray();

    // Internal data
    data->hidden = settings_.value("hidden").isValid()
                   ? settings_.value("hidden").toBool()
                   : false;

    settings_.endGroup();

    DEBUG()(data);

    RETURN();
}

ProfileData*
Settings::profileData()
{
    ENTER();

    ProfileData* data = &data_.profileData;

    RETURN( data, *data );
}

void
Settings::refresh()
{
    ENTER();

    qml("apikey")->setProperty( "text", profileData()->apiKey );
    qml("apikey")->setProperty( "cursorPosition", 0 );
    qml("closeToTray")->setProperty( "checked", profileData()->closeToTray );
    qml("ignoreSslErrors")->setProperty( "checked", profileData()->ignoreSslErrors );
    qml("numRecentIssues")->setProperty( "text", profileData()->numRecentIssues );
    qml("startLocalServer")->setProperty( "checked", profileData()->startLocalServer );
    qml("url")->setProperty( "text", profileData()->url );
    qml("url")->setProperty( "cursorPosition", 0 );
    qml("useCustomFields")->setProperty( "checked", profileData()->useCustomFields );
    qml("useSystemTrayIcon")->setProperty( "checked", profileData()->useSystemTrayIcon );

    qml("shortcutCreateIssue")->setProperty( "text", profileData()->shortcutCreateIssue );
    qml("shortcutSelectIssue")->setProperty( "text", profileData()->shortcutSelectIssue );
    qml("shortcutStartStop")->setProperty( "text", profileData()->shortcutStartStop );
    qml("shortcutToggle")->setProperty( "text", profileData()->shortcutToggle );

    updateIssueCustomFields();
    updateIssueStatuses();
    updateTrackers();
    updateTimeEntryCustomFields();

    RETURN();
}

void
Settings::save()
{
    ENTER();

    // General settings
    auto saveWindowData = [&]( Window::Data WindowData::*field, QString name )
    {
        ENTER();

        QString prefix = QString("profile-%1/%2/%3").arg(data_.profileData.id).arg(name);

        settings_.setValue( prefix.arg("geometry"), (data_.windows.*field).geometry );
        settings_.setValue( prefix.arg("position"), (data_.windows.*field).position );

        RETURN();
    };

    saveWindowData( &WindowData::issueCreator,  "issueCreator"  );
    saveWindowData( &WindowData::issueSelector, "issueSelector" );
    saveWindowData( &WindowData::mainWindow,    "mainWindow"    );
    saveWindowData( &WindowData::settings,      "settings"      );

    saveProfileData();

    settings_.sync();

    RETURN();
}

void
Settings::saveProfileData()
{
    ENTER();

    ProfileData* data = &data_.profileData;

    settings_.beginGroup( QString("profile-%1").arg(data_.profileData.id) );

    // Connection
    settings_.setValue( "apikey",            data->apiKey );
    settings_.setValue( "ignoreSslErrors",   data->ignoreSslErrors );
    settings_.setValue( "numRecentIssues",   data->numRecentIssues );
    settings_.setValue( "startLocalServer",  data->startLocalServer );
    settings_.setValue( "url",               data->url );
    settings_.setValue( "useCustomFields",   data->useCustomFields );
    settings_.setValue( "workedOnId",        data->workedOnId );
    settings_.setValue( "defaultTrackerId",  data->defaultTrackerId );
    settings_.setValue( "externalIdFieldId", data->externalIdFieldId );
    settings_.setValue( "startTimeFieldId",  data->startTimeFieldId );
    settings_.setValue( "endTimeFieldId",    data->endTimeFieldId );

    settings_.setValue( "activity", data->activityId );
    settings_.setValue( "issue",    data->issueId );
    settings_.setValue( "project",  data->projectId );

    // Shortcuts
    settings_.setValue("shortcutCreateIssue", data->shortcutCreateIssue );
    settings_.setValue("shortcutSelectIssue", data->shortcutSelectIssue );
    settings_.setValue("shortcutStartStop",   data->shortcutStartStop );
    settings_.setValue("shortcutToggle",      data->shortcutToggle );

    // Interface
    settings_.setValue( "useSystemTrayIcon", data->useSystemTrayIcon );
    settings_.setValue( "closeToTray",       data->closeToTray );

    // Recently used issues for the data
    settings_.beginWriteArray( "recentIssues" );
    for( int i = 0; i < data->recentIssues.size(); ++i )
    {
        settings_.setArrayIndex( i );
        settings_.setValue( "id",      data->recentIssues.at(i).id );
        settings_.setValue( "subject", data->recentIssues.at(i).subject );
    }
    settings_.endArray();

    // Internal
    settings_.setValue( "hidden", mainWindow()->hidden() );
    settings_.setValue( "name",   data->name );

    settings_.endGroup();

    RETURN();
}

void
Settings::toggleCustomFields()
{
    ENTER();

    updateIssueCustomFields();
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

    redmine_->setCheckSsl( !profileData()->ignoreSslErrors );
    redmine_->setUrl( profileData()->url );
    redmine_->setAuthenticator( profileData()->apiKey );

    ++callbackCounter_;
    redmine_->retrieveIssueStatuses( [&]( IssueStatuses issueStatuses, RedmineError redmineError,
                                          QStringList errors )
    {
        CBENTER();

        if( redmineError != RedmineError::NO_ERR )
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
Settings::updateIssueCustomFields()
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

        externalIdModel_.clear();
        externalIdModel_.push_back( SimpleItem(NULL_ID, err) );
        qml("externalId")->setProperty( "enabled", false );
        qml("externalId")->setProperty( "currentIndex", -1 );
        qml("externalId")->setProperty( "currentIndex", 0 );

        RETURN();
    }

    CustomFieldFilter filter;
    filter.format = "string";
    filter.type   = "issue";

    // @todo Remove from here
    redmine_->setCheckSsl( !profileData()->ignoreSslErrors );
    redmine_->setUrl( profileData()->url );
    redmine_->setAuthenticator( profileData()->apiKey );

    ++callbackCounter_;
    redmine_->retrieveCustomFields( [&]( CustomFields customFields, RedmineError redmineError,
                                         QStringList errors )
    {
        CBENTER();

        if( redmineError != RedmineError::NO_ERR )
        {
            QString errorMsg = tr("Could not load custom fields.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        QString firstEntry;

        if( customFields.size() )
            firstEntry = "Choose issue field";
        else
            firstEntry = "No issue fields found";

        int externalIdCurrentIndex = 0;
        externalIdModel_.clear();
        externalIdModel_.push_back( SimpleItem(NULL_ID, firstEntry) );

        sort( customFields.begin(), customFields.end(),
              [](CustomField l, CustomField r){ return l.name < r.name;} );

        // Create loaded custom fields
        for( const auto& customField : customFields )
        {
            if( customField.id == profileData()->externalIdFieldId )
                externalIdCurrentIndex = externalIdModel_.rowCount();
            externalIdModel_.push_back( SimpleItem(customField) );
        }

        qml("externalId")->setProperty( "currentIndex", -1 );
        qml("externalId")->setProperty( "currentIndex", externalIdCurrentIndex );
        qml("externalId")->setProperty( "enabled", true );

        CBRETURN();
    },
    filter );

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

    // @todo Remove from here
    redmine_->setCheckSsl( !profileData()->ignoreSslErrors );
    redmine_->setUrl( profileData()->url );
    redmine_->setAuthenticator( profileData()->apiKey );

    ++callbackCounter_;
    redmine_->retrieveCustomFields( [&]( CustomFields customFields, RedmineError redmineError,
                                         QStringList errors )
    {
        CBENTER();

        if( redmineError != RedmineError::NO_ERR )
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

    redmine_->setCheckSsl( !profileData()->ignoreSslErrors );
    redmine_->setUrl( profileData()->url );
    redmine_->setAuthenticator( profileData()->apiKey );

    ++callbackCounter_;
    redmine_->retrieveTrackers( [&]( Trackers trackers, RedmineError redmineError, QStringList errors )
    {
        CBENTER();

        if( redmineError != RedmineError::NO_ERR )
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

} // redtimer
