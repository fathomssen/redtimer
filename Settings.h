#ifndef SETTINGS_H
#define SETTINGS_H

#include "Models.h"
#include "Window.h"

#include "qtredmine/SimpleRedmineClient.h"

#include <QObject>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QSettings>

namespace redtimer {

/**
 * @brief A settings window and IO access for RedTimer
 */
class Settings : public Window
{
    Q_OBJECT

public:
    /// Settings data structure
    struct Data
    {
        /// @name GUI settings
        /// @{

        /// Redmine API key
        QString apiKey;

        /// Manually check the network connection
        bool checkConnection = false;

        /// Ignore SSL errors
        bool ignoreSslErrors = false;

        /// Maximum number of recently opened issues
        int numRecentIssues = 10;

        /// Shortcuts
        QString shortcutCreateIssue = "Ctrl+Alt+C";
        QString shortcutSelectIssue = "Ctrl+Alt+L";
        QString shortcutStartStop   = "Ctrl+Alt+S";
        QString shortcutToggle      = "Ctrl+Alt+R";

        /// Redmine base URL
        QString url;

        /// Use system tray icon
        bool useSystemTrayIcon = true;

        /// Issue status to switch after tracking time
        int workedOnId = NULL_ID;

        /// @}

        /// @name Internal settings
        /// @{

        /// Last used activity
        int activityId = NULL_ID;

        /// Last opened issue
        int issueId = NULL_ID;

        /// Last window position
        QPoint position;

        /// Last opened project
        int projectId = NULL_ID;

        /// Recently opened issues
        qtredmine::Issues recentIssues;

        /// @}
    };

    /// Settings data
    Data data;

private:
    /// Redmine connection object
    qtredmine::SimpleRedmineClient* redmine_;

    /// Application settings
    QSettings settings_;

    /// Cached issue statuses
    SimpleModel issueStatusModel_;

public:
    /**
     * @brief Constructor for a Settings object
     *
     * @param redmine Redmine connection object
     * @param parent Parent QObject
     */
    explicit Settings( qtredmine::SimpleRedmineClient* redmine, QQuickView* parent = nullptr );

    /**
     * @brief Load settings from settings file
     */
    void load();

    /**
     * @brief Save settings to settings file
     */
    void save();

public slots:
    /**
     * @brief Store the settings from the settings dialog in this class
     */
    void apply();

    /**
     * @brief Close the settings dialog
     */
    void close();

    /**
     * @brief Display the settings dialog
     */
    void display();

    /**
     * @brief Update issue statuses
     */
    void updateIssueStatuses();

signals:
    /**
     * @brief Emitted when data have been applied in this GUI
     */
    void applied();
};

} // redtimer

#endif // SETTINGS_H
