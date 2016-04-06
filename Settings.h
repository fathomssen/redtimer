#ifndef SETTINGS_H
#define SETTINGS_H

#include "Models.h"

#include "qtredmine/SimpleRedmineClient.h"

#include <QObject>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QSettings>

namespace redtimer {

class Settings : public QObject
{
    Q_OBJECT

private:
    /// Redmine connection object
    qtredmine::SimpleRedmineClient* redmine_;

    /// Settings window object
    QQuickView* win_;

    /// Issue selector window context
    QQmlContext* ctx_;

    /// Settings item object
    QQuickItem* item_;

    /// Application settings
    QSettings settings_;

    /// Cached issue statuses
    SimpleModel issueStatusModel_;

    /// Redmine API key
    QString apiKey_;

    /// Redmine base URL
    QString url_;

    /// Last used activity
    int activityId_ = NULL_ID;

    /// Last opened issue
    int issueId_ = NULL_ID;

    /// Last window position
    QPoint position_;

    /// Last opened project
    int projectId_ = NULL_ID;

    /// Issue status to switch after tracking time
    int workedOnId_ = NULL_ID;

    /// Recently opened issues
    qtredmine::Issues recentIssues_;

    /**
     * @brief Get a QML GUI item
     *
     * @param qmlItem Name of the QML GUI item
     *
     * @return QML GUI item
     */
    QQuickItem* qml( QString qmlItem );

public:
    /**
     * @brief Constructor for a Settings object
     *
     * @param redmine Redmine connection object
     * @param parent Parent QObject
     */
    explicit Settings( qtredmine::SimpleRedmineClient* redmine,
                       QObject* parent = nullptr );

    /**
     * @brief Load settings from settings file
     */
    void load();

    /**
     * @brief Save settings to settings file
     */
    void save();

    /// @name Getters
    /// @{

    /**
     * @brief Get last used activity
     *
     * @return Activity ID
     */
    int getActivity();

    /**
     * @brief Get the Redmine API key
     *
     * @return Redmine API key
     */
    QString getApiKey() const;

    /**
     * @brief Get last used issue
     *
     * @return Issue ID
     */
    int getIssue();

    /**
     * @brief Get last window position
     *
     * @return Window position
     */
    QPoint getPosition();

    /**
     * @brief Get last used project
     *
     * @return Project ID
     */
    int getProject();

    /**
     * @brief Get recently opened issues
     *
     * @return List of recently opened issues
     */
    qtredmine::Issues getRecentIssues();

    /**
     * @brief Get the Redmine base URL
     *
     * @return Redmine base URL
     */
    QString getUrl() const;

    /**
     * @brief Get the worked on issue status ID
     *
     * @return Worked on issue status ID
     */
    int getWorkedOnId() const;

    /**
     * @brief Get the settings window
     *
     * @return Settings window
     */
    QQuickView* window() const;

    /// @}

    /// @name Setters
    /// @{

    /**
     * @brief Set last used activity
     *
     * @param id Activity ID
     */
    void setActivity( int id );

    /**
     * @brief Set last used issue
     *
     * @param id Issue ID
     */
    void setIssue( int id );

    /**
     * @brief Set last window position
     *
     * @param position Window position
     */
    void setPosition( QPoint position );

    /**
     * @brief Set last used project
     *
     * @param id Project ID
     */
    void setProject( int id );

    /**
     * @brief Set recently opened issues
     *
     * @paramt recentIssues List of recently opened issues
     */
    void setRecentIssues( qtredmine::Issues recentIssues );

    /// @}

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
