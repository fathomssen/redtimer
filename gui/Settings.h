#pragma once

#include "qtredmine/Logging.h"
#include "MainWindow.h"
#include "Models.h"
#include "Window.h"

#include "qtredmine/SimpleRedmineClient.h"

#include <QObject>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QSet>
#include <QSettings>
#include <QSortFilterProxyModel>

namespace redtimer {

/**
 * @brief Profile data
 */
struct ProfileData
{
    /// @name Profile
    /// @{

    /// Profile ID
    int id = NULL_ID;

    /// Profile name
    QString name;

    /// @}

    /// @name Settings GUI
    /// @{

    /// Redmine API key
    QString apiKey;

    /// Ignore SSL errors
    bool ignoreSslErrors;

    /// Maximum number of recently opened issues
    int numRecentIssues;

    /// Shortcuts
    QString shortcutCreateIssue;
    QString shortcutSelectIssue;
    QString shortcutStartStop;
    QString shortcutToggle;

    /// Redmine base URL
    QString url;

    /// Use custom fields
    bool useCustomFields;

    /// Start a local socket server
    bool startLocalServer;

    /// Use system tray icon
    bool useSystemTrayIcon;

    /// Close to tray
    bool closeToTray;

    /// Issue status to switch after tracking time
    int workedOnId;

    /// Default tracker to use in the Issue Creator
    int defaultTrackerId;

    /// ID of the issue custom field for the external issue ID
    int externalIdFieldId;

    /// ID of the time entry custom field for the start time
    int startTimeFieldId;

    /// ID of the time entry custom field for the end time
    int endTimeFieldId;

    /// @}

    /// @name Internal settings
    /// @{

    /// Last used activity
    int activityId;

    /// Last opened issue
    int issueId;

    /// Last opened project
    int projectId;

    /// Recently opened issues
    qtredmine::Issues recentIssues;

    /// Window was hidden on exit
    bool hidden;

    /// @}

    /**
     * @brief Determines whether this set of settings settings is valid
     *
     * To be valid, a set of settings must at least contain a URL and an API key.
     *
     * @param OUT errmsg Error message
     *
     * @return true if settings are valid, false otherwise
     */
    bool isValid( QString* errmsg = nullptr ) const;
};

/// Settings data structure for the currently loaded profile
struct WindowData
{
    /// Window data of the Issue Creator
    Window::Data issueCreator;

    /// Window data of the Issue Selector
    Window::Data issueSelector;

    /// Window data of the main window
    Window::Data mainWindow;

    /// Window data of the settings dialog
    Window::Data settings;
};

struct SettingsData
{
    /// Internal representation of the profiles
    ProfileData profileData;

    /// Window data
    WindowData windows;
};

/**
 * @brief A settings window and IO access for RedTimer
 */
class Settings : public Window
{
    Q_OBJECT

private:
    /// Redmine connection object
    qtredmine::SimpleRedmineClient* redmine_;

    /// Initialised
    bool initialised_ = false;

    /// Application settings
    QSettings settings_;

    /// Cached issue statuses
    SimpleModel issueStatusModel_;

    /// Cached trackers
    SimpleModel trackerModel_;

    /// Issue custom fields for the external ID
    SimpleModel externalIdModel_;

    /// Time entry custom fields for the start time
    SimpleModel startTimeModel_;

    /// Time entry custom fields for the end time
    SimpleModel endTimeModel_;

    /// Data pool
    SettingsData data_;

    /// Profile ID
    int profileId_ = NULL_ID;

private:
    /**
     * @brief Load profile-dependent settings from settings file
     */
    void loadProfileData();

    /**
     * @brief Save profile-dependent settings to settings file
     */
    void saveProfileData();

public:
    /**
     * @brief Constructor for a Settings object
     *
     * @param mainWindow Main window object
     * @param profile Profile to load
     */
    explicit Settings( MainWindow* mainWindow, const QString& profile );

    /**
     * @brief Load settings from settings file
     *
     * @param apply Apply the loaded settings
     */
    void load( const bool apply = true );

    /**
     * @brief Get the specified profile data
     *
     * @return Pointer to the profile data
     */
    ProfileData* profileData();

    /**
     * @brief Refresh the GUI and display updated data
     */
    void refresh();

    /**
     * @brief Save settings to settings file
     */
    void save();

    /**
     * @brief Get the window data
     *
     * @return Window data
     */
    WindowData* windowData();

public slots:
    /**
     * @brief Store the settings from the settings dialog in this class
     */
    void apply();

    /**
     * @brief Store the profile data from the settings dialog in this class
     *
     * @param reload Specifies whether the connection has to be reloaded after applying profile data
     */
    void applyProfileData( bool* reload = nullptr );

    /**
     * @brief Store the settings and close
     */
    void applyAndClose();

    /**
     * @brief Close the settings dialog
     */
    void close();

    /**
     * @brief Cancel and close
     */
    void cancel();

    /**
     * @brief Display the settings dialog
     */
    void display();

    /**
     * @brief Settings have been initialised
     *
     * @return true if settings have been initialised, false otherwise
     */
    bool initialised();

    /**
     * @brief Update issue statuses
     */
    void updateIssueStatuses();

    /**
     * @brief Update trackers
     */
    void updateTrackers();

    /**
     * @brief Update issue custom fields
     */
    void updateIssueCustomFields();

    /**
     * @brief Update time entry custom fields
     */
    void updateTimeEntryCustomFields();

    /**
     * @brief Toggle custom fields
     */
    void toggleCustomFields();

signals:
    /**
     * @brief Emitted when data have been applied in this GUI
     */
    void applied();
};

} // redtimer

inline QDebug
operator<<( QDebug debug, const redtimer::WindowData& data )
{
    QDebugStateSaver saver( debug );
    DEBUGFIELDS(issueCreator)(issueSelector)(mainWindow)(settings);
    return debug;
}

inline QDebug
operator<<( QDebug debug, const redtimer::ProfileData& data )
{
    QDebugStateSaver saver( debug );
    DEBUGFIELDS(apiKey)(ignoreSslErrors)(numRecentIssues)(shortcutCreateIssue)
            (shortcutSelectIssue)(shortcutStartStop)(shortcutToggle)(url)(useCustomFields)
            (useSystemTrayIcon)(closeToTray)(workedOnId)(defaultTrackerId)(startTimeFieldId)(endTimeFieldId)
            (activityId)(issueId)(projectId)(recentIssues);
    return debug;
}

inline QDebug
operator<<( QDebug debug, const redtimer::SettingsData& data )
{
    QDebugStateSaver saver( debug );
    DEBUGFIELDS(profileData)(windows);
    return debug;
}

inline QDebug
operator<<( QDebug debug, const redtimer::Settings& data )
{
    QDebugStateSaver saver( debug );
    (void)data;
    return debug;
}
