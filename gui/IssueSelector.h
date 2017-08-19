#pragma once

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

    /// Current assignee
    int assigneeId_ = NULL_ID;

    /// List of assignees in the GUI
    SimpleModel assigneeModel_;

    /// Current version
    int versionId_ = NULL_ID;

    /// List of versions in the GUI
    SimpleModel versionModel_;

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
     * @brief Get the currently selected assignee ID
     *
     * @return Assignee ID
     */
    int getAssigneeId() const;

    /**
     * @brief Get the currently selected project ID
     *
     * @return Project ID
     */
    int getProjectId() const;

    /**
     * @brief Get the currently selected version ID
     *
     * @return Version ID
     */
    int getVersionId() const;

    /// @}

    /// @name Setters
    /// @{

    /**
     * @brief Set the currently selected assignee ID
     *
     * @param id Assignee ID
     * @param fixed The assignee may not be changed
     */
    void setAssigneeId( int id, bool fixed = false );

    /**
     * @brief Set the currently selected project ID
     *
     * @param id Project ID
     * @param fixed The project may not be changed
     */
    void setProjectId( int id, bool fixed = false );

    /**
     * @brief Set the currently selected version ID
     *
     * @param id Version ID
     * @param fixed The version may not be changed
     */
    void setVersionId( int id, bool fixed = false );

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
     * @brief Slot to a selected assignee
     */
    void assigneeSelected( int index );

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
     * @brief Slot to a selected version
     */
    void versionSelected( int index );

    /**
     * @brief Update assignees and refresh assignee list
     */
    void loadAssignees();

    /**
     * @brief Update issues and refresh issues list
     */
    void loadIssues();

    /**
     * @brief Update projects and refresh project list
     */
    void loadProjects();

    /**
     * @brief Update versions and refresh version list
     */
    void loadVersions();

signals:
    /**
     * @brief Emitted when an issue has been selected
     */
    void selected( int issueId );
};

} // redtimer
