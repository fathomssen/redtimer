#ifndef REDTIMER_H
#define REDTIMER_H

#include "Models.h"
#include "Settings.h"

#include "qtredmine/Redmine.h"

#include <QList>
#include <QMap>
#include <QObject>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>

#include <memory>

namespace redtimer {

class RedTimer : public QObject
{
    Q_OBJECT

private:
    /// Redmine connection object
    qtredmine::Redmine* redmine_;

    /// Redmine connection dialog object
    Settings* settings_;

    /// Main window object
    QQuickView* win_;

    /// Main window context
    QQmlContext* ctx_;

    /// Main item object
    QQuickItem* item_;

    /// Current activity
    qtredmine::Redmine::Enumeration activity_;

    /// Cached activities
    qtredmine::Redmine::Enumerations activities_;

    /// Current issue
    qtredmine::Redmine::Issue issue_;

    /// Cached issues
    qtredmine::Redmine::Issues issues_;

    /// Current issue status
    qtredmine::Redmine::IssueStatus issueStatus_;

    /// Cached issue statuses
    qtredmine::Redmine::IssueStatuses issueStatuses_;

    /// Current project
    qtredmine::Redmine::Project project_;

    /// List of projects in the GUI
    SimpleModel projectModel_;

    /// Cached projects
    qtredmine::Redmine::Projects projects_;

    /// Current tracker
    qtredmine::Redmine::Tracker tracker_;

    /// Cached trackers
    qtredmine::Redmine::Trackers trackers_;

public:
    explicit RedTimer( QObject* parent = nullptr );

    /**
     * @brief Run the application
     *
     * @return Status code
     */
    int display();

    /**
     * @brief Update the issue list in the GUI
     */
    void refreshIssues();

    /**
     * @brief Update the issue statuse list in the GUI
     */
    void refreshIssueStatuses();

    /**
     * @brief Update the project list in the GUI
     */
    void refreshProjects();

    /**
     * @brief Update the activities list in the GUI
     */
    void refreshActivities();

    /**
     * @brief Update the tracker list in the GUI
     */
    void refreshTrackers();

public slots:
    /**
     * @brief Update Redmine entities
     */
    void update();

    /**
     * @brief Update issues
     */
    void updateIssues();

    /**
     * @brief Update issues
     *
     * @param projectId Project ID to get issues for
     */
    void updateIssues( int projectId );

    /**
     * @brief Update issue statuses
     */
    void updateIssueStatuses();

    /**
     * @brief Update projects
     */
    void updateProjects();

    /**
     * @brief Update activities
     */
    void updateActivities();

    /**
     * @brief Update trackers
     */
    void updateTrackers();

private slots:
    /**
     * @brief Slot to reconnect to Redmine
     */
    void reconnect();

    /**
     * @brief Slot to a selected project
     */
    void projectSelected( int index );
};

} // redtimer

#endif // REDTIMER_H
