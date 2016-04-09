#ifndef REDTIMER_H
#define REDTIMER_H

#include "IssueSelector.h"
#include "Models.h"
#include "Settings.h"

#include "qtredmine/SimpleRedmineClient.h"

#include <QApplication>
#include <QEvent>
#include <QList>
#include <QMap>
#include <QObject>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QTime>

#include <memory>

namespace redtimer {

/**
 * @brief RedTimer, a Redmine Time Tracker
 */
class RedTimer : public QObject
{
    Q_OBJECT

private:
    /// Redmine connection object
    qtredmine::SimpleRedmineClient* redmine_;

    /// Redmine connection dialog object
    Settings* settings_;

    /// Redmine issue selector dialog object
    IssueSelector* issueSelector_;

    /// Main application
    QApplication* app_;

    /// Main window object
    QQuickView* win_;

    /// Main window context
    QQmlContext* ctx_;

    /// Main item object
    QQuickItem* item_;

    /// System tray icon
    QSystemTrayIcon* trayIcon_;

    /// Timer for stopping the worked on time
    QTimer* timer_;

    /// Currently tracked time in seconds
    int counter_ = 0;

    /// Counter QML element for quick access
    QQuickItem* qmlCounter_;

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

    /**
     * @brief Add an issue to the list of recent issues
     *
     * This methos adds the issue to the top of the recent issues list and crops the list after ten entries.
     *
     * @param issue Issue to add to the list
     */
    void addRecentIssue( qtredmine::Issue issue );

    /**
     * @brief Get a QML GUI item
     *
     * @param qmlItem Name of the QML GUI item
     *
     * @return QML GUI item
     */
    QQuickItem* qml( QString qmlItem );

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
     */
    explicit RedTimer( QApplication* parent = nullptr );

    /**
     * @brief Destructor
     */
    ~RedTimer();

    /**
     * @brief Initialise the GUI
     */
    void init();

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
     * @brief Display or hide the main window
     */
    void toggle();

    /**
     * @brief Slot to a selected issue status
     */
    void issueStatusSelected( int index );

    /**
     * @brief Display a message
     *
     * @param text Message to display
     * @param type Message type (one of \c QtInfoMsg, \c QtWarningMsg and \c QtCriticalMsg)
     * @param timeout Duration in milliseconds that the message will be displayed
     */
    void message( QString text, QtMsgType type = QtInfoMsg, int timeout = 5000 );

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
     */
    void loadIssue( int issueId, bool startTimer = true );

    /**
     * @brief Reconnect to Redmine
     */
    void reconnect();

    /**
     * @brief Refresh Redmine entities
     */
    void refresh();

    /**
     * @brief Refresh activities
     */
    void refreshActivities();

    /**
     * @brief Refresh the counter
     */
    void refreshCounter();

    /**
     * @brief Refresh issue statuses
     */
    void refreshIssueStatuses();

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
     * @brief Stop time tracking
     *
     * Stops time tracking using the timer.
     *
     * @param resetTimerOnError Reset timer even after an error occurred
     * @param stopTimerAfterSaving Stop the timer after saving the time and resetting the counter.
     *
     * \sa timer_
     */
    void stop( bool resetTimerOnError = true, bool stopTimerAfterSaving = true );

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

signals:
    /**
     * @brief Signal emitted when a time entry has been saved
     */
    void timeEntrySaved();
};

} // redtimer

#endif // REDTIMER_H
