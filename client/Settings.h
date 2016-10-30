#pragma once

#include "qtredmine/Logging.h"
#include "MainWindow.h"
#include "Models.h"
#include "ProfileData.h"
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
    /// Current profile ID
    int profileId = NULL_ID;

    /// Internal representation of the profiles
    QMap<int, ProfileData> profiles;

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

    /// Time entry custom fields for the start time
    SimpleModel startTimeModel_;

    /// Time entry custom fields for the end time
    SimpleModel endTimeModel_;

    /// Currently selected profile ID
    int profileId_ = NULL_ID;

    /// GUI profiles
    SimpleModel profilesModel_;
    QSortFilterProxyModel profilesProxyModel_;

    /// Data pool
    SettingsData data_;

private:
    /**
     * @brief Load profile-dependent settings from settings file
     *
     * @param profileId Profile to load data for
     * @param initData Inital data
     */
    void loadProfileData( const int profileId, ProfileData *initData = nullptr );

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
     * @brief Get the current profile hash
     *
     * @param id Profile ID to get the hash for
     *
     * @return Profile hash
     */
    QString profileHash( int id = NULL_ID );

    /**
     * @brief Save profile-dependent settings to settings file
     *
     * @param profileId Profile to save data for
     */
    void saveProfileData( int profileId );

    /**
     * @brief Get the currently selected profile data
     *
     * @return Pointer to the profile data
     */
    ProfileData* profileData();

public:
    /**
     * @brief Constructor for a Settings object
     *
     * @param mainWindow Main window object
     */
    explicit Settings( MainWindow* mainWindow );

    /**
     * @brief Load settings from settings file
     *
     * @param apply Apply the loaded settings
     * @param createNewProfile Create new profile if none exists
     */
    void load( const bool apply = true , const bool createNewProfile = true );

    /**
     * @brief Get the specified profile data
     *
     * @param profileId Profile ID
     *
     * @return Pointer to the profile data
     */
    ProfileData* profileData( int profileId );

    /**
     * @brief Get the profile ID
     *
     * @return Profile ID
     */
    int profileId();

    /**
     * @brief Get all profiles
     *
     * @return Map of profiles
     */
    QMap<int, ProfileData> profiles();

    /**
     * @brief Save settings to settings file
     */
    void save();

    /**
     * @brief Set the profile ID by ID
     *
     * @param profileId Profile ID
     */
    void setProfileId( int profileId );

    /**
     * @brief Set the profile ID by name
     *
     * @param profileName Profile name
     */
    void setProfileId( QString profileName );

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
     * @brief Copy profile
     *
     * Create a new profile and copy data from current profile.
     *
     * @return true if profile was copied successfully, false otherwise
     */
    bool copyProfile();

    /**
     * @brief Create a new profile
     *
     * @param copy Copy current profile
     * @param force If no profile name was specified, use the default
     *
     * @return true if profile was created successfully, false otherwise
     */
    bool createProfile( bool copy = false, bool force = false);

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
     * @brief Settings have been initialised
     *
     * @return true if settings have been initialised, false otherwise
     */
    bool initialised();

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
operator<<( QDebug debug, const redtimer::WindowData& data )
{
    QDebugStateSaver saver( debug );
    DEBUGFIELDS(issueCreator)(issueSelector)(mainWindow)(settings);
    return debug;
}

inline QDebug
operator<<( QDebug debug, const redtimer::SettingsData& data )
{
    QDebugStateSaver saver( debug );
    DEBUGFIELDS(profileId)(profiles)(windows);
    return debug;
}

inline QDebug
operator<<( QDebug debug, const redtimer::Settings& data )
{
    QDebugStateSaver saver( debug );
    (void)data;
    return debug;
}
