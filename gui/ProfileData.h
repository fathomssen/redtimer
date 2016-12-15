#pragma once

#include "qtredmine/Logging.h"
#include "qtredmine/SimpleRedmineTypes.h"

namespace redtimer {

/**
 * @brief Profile data
 */
struct ProfileData
{
    /// @name Profile settings
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

} // redtimer

inline QDebug
operator<<( QDebug debug, const redtimer::ProfileData& data )
{
    QDebugStateSaver saver( debug );
    DEBUGFIELDS(id)(name)(apiKey)(ignoreSslErrors)(numRecentIssues)(shortcutCreateIssue)
            (shortcutSelectIssue)(shortcutStartStop)(shortcutToggle)(url)(useCustomFields)
            (useSystemTrayIcon)(closeToTray)(workedOnId)(defaultTrackerId)(startTimeFieldId)(endTimeFieldId)
            (activityId)(issueId)(projectId)(recentIssues);
    return debug;
}
