#pragma once

#include "Models.h"
#include "Window.h"

#include "qtredmine/SimpleRedmineClient.h"
#include "qxtglobalshortcut.h"

#include <QApplication>
#include <QDateTime>
#include <QEvent>
#include <QList>
#include <QMap>
#include <QObject>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QSystemTrayIcon>
#include <QTime>
#include <QTimer>

#include <memory>

namespace redtimer {

// forward declarations
class Settings;

/**
 * @brief RedTimer, a Redmine Time Tracker
 */
class MainWindow : public Window
{
    Q_OBJECT

private:
    /// Redmine connection object
    qtredmine::SimpleRedmineClient* redmine_ = nullptr;

    /// Main application
    QApplication* app_ = nullptr;

    /// Shortcuts
    QxtGlobalShortcut* shortcutCreateIssue_;
    QxtGlobalShortcut* shortcutSelectIssue_;
    QxtGlobalShortcut* shortcutStartStop_;
    QxtGlobalShortcut* shortcutToggle_;

    /// System tray icon
    QSystemTrayIcon* trayIcon_ = nullptr;

    /// Timer for stopping the worked on time
    QTimer* timer_ = nullptr;

    /// Update the counter in the GUI
    bool updateCounterGui_ = true;

    /// Timer for manually checking the connection
    QTimer* checkConnectionTimer_ = nullptr;

    /// Connection error message
    QQuickItem* connectionError_ = nullptr;

    /// Currently tracked time in seconds
    int counter_ = 0;

    /// Displayed tracked time when the counter was paused to edit the time
    int counterBeforeEdit_ = 0;

    /// Counter QML element for quick access
    QQuickItem* qmlCounter_ = nullptr;

    /// Current activity
    int activityId_ = NULL_ID;

    /// Cached activities
    SimpleModel activityModel_;

    /// Current issue
    qtredmine::Issue issue_;

    /// Cached issue statuses
    SimpleModel issueStatusModel_;

    /// Recently opened issues
    IssueModel recentIssues_;

    /// Last time the counter was increased
    QDateTime lastCounterIncrease_;

    /// Last time that the timer has been started, in UTC
    QDateTime lastStarted_;

    /**
     * @brief Add an issue to the list of recent issues
     *
     * This methos adds the issue to the top of the recent issues list and crops the list after ten entries.
     *
     * @param issue Issue to add to the list
     */
    void addRecentIssue( qtredmine::Issue issue );

    /**
     * @brief Start the timer
     */
    void startTimer();

    /**
     * @brief Stop the timer
     */
    void stopTimer();

public:
    /**
     * @brief RedTimer constructor
     *
     * @param parent Parent QObject
     * @param profile Load this profile on startup
     */
    explicit MainWindow( QApplication* parent = nullptr, const QString profile = QString() );

    /**
     * @brief Initialise the tray icon
     */
    void initTrayIcon();

    /**
     * @brief Save the current configuration
     */
    void saveSettings();

    /**
     * @brief Get the system tray icon
     *
     * @return Tray icon object
     */
    QSystemTrayIcon* trayIcon();

    /// Redmine connection dialog object
    Settings* settings_ = nullptr;

protected:
    /**
     * @brief Filter Qt events
     *
     * Prevent the GUI from closing when timer is running.
     *
     * @param obj Watched object
     * @param event Received event
     *
     * @return true if event has been processed, false otherwise
     */
    bool eventFilter( QObject* obj, QEvent* event );

private slots:
    /**
     * @brief Slot to a selected activity
     */
    void activitySelected( int index );

    /**
     * @brief Proactively check the network connection, ignoring virtual interfaces
     *
     * @return \c Accessible if connection currently works, \c NotAccessible otherwise
     */
    void checkNetworkConnection();

    /**
     * @brief Display and raise the main window
     */
    void display();

    /**
     * @brief Exit the application
     *
     * If the timer is running, a warning is displayed which gives the user the opportunity to abort exiting,
     * to save the tracked time or to discard the tracked time.
     */
    void exit();

    /**
     * @brief Open the issue creator and load issue
     */
    void createIssue();

    /**
     * @brief Hide the main window
     */
    void hide();

    /**
     * @brief Slot to a selected issue status
     */
    void issueStatusSelected( int index );

    /**
     * @brief Load issue from Redmine
     *
     * Uses the issue ID from the quick pick list.
     */
    void loadIssueFromList( int index );

    /**
     * @brief Load issue from Redmine
     *
     * Uses the issue ID from the quick pick text field.
     */
    void loadIssueFromTextField();

    /**
     * @brief Load issue from Redmine
     *
     * @param issueId Issue ID
     * @param startTimer Automatically start the timer after loading the issue
     * @param saveNewIssue When saving tracked time, save time for the newly loaded issue
     */
    void loadIssue( int issueId, bool startTimer = true, bool saveNewIssue = false );

    /**
     * @brief Notify about the current connection status
     *
     * @param connected Current connection status
     */
    void notifyConnectionStatus( QNetworkAccessManager::NetworkAccessibility connected );

    /**
     * @brief Pause the update of the counter in the GUI
     */
    void pauseCounterGui();

    /**
     * @brief Resume the update of the counter in the GUI
     */
    void resumeCounterGui();

    /**
     * @brief Reconnect to Redmine
     */
    void reconnect();

    /**
     * @brief Refresh Redmine entities
     */
    void refreshGui();

    /**
     * @brief Reset all GUI fields
     *
     * @param value Value to reset GUI fields to, empty by default
     */
    void resetGui( const QString value = QString() );

    /**
     * @brief Refresh activities
     */
    void loadActivities();

    /**
     * @brief Load the latest activity on the issue and refresh activities
     */
    void loadLatestActivity();

    /**
     * @brief Refresh the counter
     */
    void refreshCounter();

    /**
     * @brief Refresh issue statuses
     */
    void loadIssueStatuses();

    /**
     * @brief Open the issue selector and load issue
     */
    void selectIssue();

    /**
     * @brief Start time tracking
     *
     * Starts time tracking using the timer. If the timer is already active, the previously tracked time will
     * be saved first and the the new time tracking will be started. Requires the current issue to be set.
     *
     * \sa timer_
     * \sa issue_
     */
    void start();

    /**
     * @brief Start or stop time tracking
     *
     * Start time tracking if the timer is currently inactive. Stop time tracking if the timer is currently
     * active.
     *
     * \sa timer_
     */
    void startStop();

    /**
     * @brief Display or hide the main window
     */
    void toggle();

    /**
     * @brief Handle a tray event
     *
     * @param reason Reason for the activation of the tray
     */
    void trayEvent( QSystemTrayIcon::ActivationReason reason );

    /**
     * @brief Update issue status for current issue
     *
     * @param statusId Issue status ID
     */
    void updateIssueStatus( int statusId );

    /**
     * @brief Update the title and tray icon tool tip
     */
    void updateTitle();

public slots:

    /**
     * @brief Stop time tracking
     *
     * Stops time tracking using the timer.
     *
     * @param resetTimerOnError Reset timer even after an error occurred
     * @param stopTimerAfterSaving Stop the timer after saving the time and resetting the counter
     * @param cb Success callback
     *
     * \sa timer_
     */
    void stop( bool resetTimerOnError = true, bool stopTimerAfterSaving = true ,
               qtredmine::SuccessCb cb = nullptr );

signals:
    /**
     * @brief Signal emitted when a time entry has been saved
     */
    void timeEntrySaved();
};

} // redtimer
