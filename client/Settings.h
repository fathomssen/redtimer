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
 * @brief A settings window and IO access for RedTimer
 */
class Settings : public Window
{
    Q_OBJECT

public:
    /// Settings data structure for the currently loaded profile
    struct ProfileData
    {
        /// @name Profile settings
        /// @{

        /// Profile ID
        int id = NULL_ID;

        /// Profile name
        QString name;

        /// @}

        /// @name GUI settings
        /// @{

        /// Redmine API key
        QString apiKey;

        /// Manually check the network connection
        bool checkConnection;

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

        /// Use system tray icon
        bool useSystemTrayIcon;

        /// Close to tray
        bool closeToTray;

        /// Issue status to switch after tracking time
        int workedOnId;

        /// Default tracker to use in the Issue Creator
        int defaultTrackerId;

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

        /// @}
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

    /// Currently applied settings
    ProfileData data_;

    /// Window data
    WindowData win_;

private:
    /// Redmine connection object
    qtredmine::SimpleRedmineClient* redmine_;

    /// Application settings
    QSettings settings_;

    /// Cached issue statuses
    SimpleModel issueStatusModel_;

    /// Cached trackers
    SimpleModel trackerModel_;

    /// Time entry custom fields for the start time
    SimpleModel startTimeModel_;

    /// Time entry custom fields for the end time
    SimpleModel endTimeModel_;

    /// GUI profiles
    SimpleModel profilesModel_;
    QSortFilterProxyModel profilesProxyModel_;

    /// Current profile ID
    int profileId_ = NULL_ID;

    /// Internal representation of the profiles
    QMap<int, ProfileData> profiles_;

private:
    /**
     * @brief Display a message box to get a profile name
     *
     * @param[out] name Profile name
     * @param[in]  title Title
     * @param[in]  initText Initial text
     *
     * @return true if valid profile name was specified, false otherwise
     */
    bool profileNameDialog( QString& name, QString title, QString initText );

    /**
     * @brief Get the current profile data
     *
     * @return Pointer to the profile data
     */
    ProfileData* profileData();

    /**
     * @brief Get the current profile hash
     *
     * @param id Profile ID to get the hash for
     *
     * @return Profile hash
     */
    QString profileHash( int id = NULL_ID );

public:
    /**
     * @brief Constructor for a Settings object
     *
     * @param mainWindow Main window object
     */
    explicit Settings( MainWindow* mainWindow );

    /**
     * @brief Determines whether the currently loaded settings are valid
     *
     * To be valid, a set of settings must at least contain a URL and an API key.
     *
     * @return true if settings are valid, false otherwise
     */
    bool isValid( bool displayError = false );

    /**
     * @brief Load settings from settings file
     *
     * @param profile Load this profile instead of the last loaded
     * @param apply Apply the loaded settings
     */
    void load( const QString data_, const bool apply = true );

    /**
     * @brief Load settings from settings file for current profile
     *
     * @param apply Apply the loaded settings
     */
    void load( const bool apply );

    /**
     * @brief Load settings from settings file
     */
    void load();

    /**
     * @brief Load profile-dependent settings from settings file
     *
     * @param profileId Profile to load data for
     * @param initData Inital data
     */
    void loadProfileData( const int profileId, const ProfileData* initData = nullptr );

    /**
     * @brief Save settings to settings file
     */
    void save();

    /**
     * @brief Save profile-dependent settings to settings file
     *
     * @param profileId Profile to save data for
     */
    void saveProfileData( int profileId );

public slots:
    /**
     * @brief Store the settings from the settings dialog in this class
     */
    void apply();

    /**
     * @brief Store the profile data from the settings dialog in this class
     */
    void applyProfileData();

    /**
     * @brief Store the settings and close
     */
    void applyAndClose();

    /**
     * @brief Close the settings dialog
     */
    void close();

    /**
     * @brief Create a new profile
     *
     * @return true if profile was created successfully, false otherwise
     */
    bool createProfile();

    /**
     * @brief Cancel and close
     */
    void cancel();

    /**
     * @brief Delete the currently selected profile
     */
    void deleteProfile();

    /**
     * @brief Display the settings dialog
     */
    void display();

    /**
     * @brief A profile has been selected
     *
     * @param profileIndex Selected profile index
     */
    void profileSelected( int profileIndex );

    /**
     * @brief Rename the currently selected profile
     */
    void renameProfile();

    /**
     * @brief Update issue statuses
     */
    void updateIssueStatuses();

    /**
     * @brief Update trackers
     */
    void updateTrackers();

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
operator<<( QDebug debug, const redtimer::Settings::ProfileData& data )
{
    QDebugStateSaver saver( debug );

    DEBUGFIELDS(id)(name)(apiKey)(checkConnection)(ignoreSslErrors)(numRecentIssues)(shortcutCreateIssue)
    (shortcutSelectIssue)(shortcutStartStop)(shortcutToggle)(url)(useCustomFields)(useSystemTrayIcon)
    (closeToTray)(workedOnId)(defaultTrackerId)(startTimeFieldId)(endTimeFieldId)(activityId)
    (issueId)(projectId)(recentIssues);

    return debug;
}
