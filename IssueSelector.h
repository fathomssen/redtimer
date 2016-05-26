#ifndef ISSUESELECTOR_H
#define ISSUESELECTOR_H

#include "MainWindow.h"
#include "Models.h"
#include "Window.h"
#include "qtredmine/SimpleRedmineClient.h"

#include <QSortFilterProxyModel>

namespace redtimer {

/**
 * @brief An issue selector for RedTimer
 */
class IssueSelector : public Window
{
    Q_OBJECT

private:
    /// Redmine connection object
    qtredmine::SimpleRedmineClient* redmine_;

    /// List of issues in the GUI
    IssueModel issuesModel_;
    QSortFilterProxyModel issuesProxyModel_;

    /// Current project
    int projectId_ = NULL_ID;

    /// List of projects in the GUI
    SimpleModel projectModel_;

public:
    /**
     * @brief Constructor for an IssueSelector object
     *
     * @param redmine Redmine connection object
     * @param mainWindow Main window object
     */
    explicit IssueSelector( qtredmine::SimpleRedmineClient* redmine, MainWindow* mainWindow );

    /// @name Getters
    /// @{

    /**
     * @brief Get the currently selected project ID
     *
     * @return Project ID
     */
    int getProjectId() const;

    /// @}

    /// @name Setters
    /// @{

    /**
     * @brief Set the currently selected project ID
     *
     * @param id Project ID
     * @param fixed The project may not be changed
     */
    void setProjectId( int id, bool fixed = false );

    /// @}

public slots:
    /**
     * @brief Close the issue selector dialog
     */
    void close();

    /**
     * @brief Display the issue selector dialog
     */
    void display();

private slots:    
    /**
     * @brief Filter issues using the filter text field
     */
    void filterIssues();

    /**
     * @brief Slot to a selected issue
     */
    void issueSelected( int index );

    /**
     * @brief Slot to a selected project
     */
    void projectSelected( int index );

    /**
     * @brief Update issues and refresh issues list
     */
    void loadIssues();

    /**
     * @brief Update projects and refresh project list
     */
    void loadProjects();

signals:
    /**
     * @brief Emitted when an issue has been selected
     */
    void selected( int issueId );
};

} // redtimer

#endif // ISSUESELECTOR_H
