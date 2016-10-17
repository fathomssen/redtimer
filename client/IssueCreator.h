#pragma once

#include "MainWindow.h"
#include "Models.h"
#include "Window.h"
#include "qtredmine/SimpleRedmineClient.h"

namespace redtimer {

/**
 * @brief An issue creator for RedTimer
 */
class IssueCreator : public Window
{
    Q_OBJECT

private:
    /// Redmine connection object
    qtredmine::SimpleRedmineClient* redmine_ = nullptr;

    /// Initial height
    int initHeight_;

    /// Emit the cancelled signal upon closing
    bool cancelOnClose_ = true;

    /// Parent issue was initialised once
    bool parentIssueInit_ = false;

    /// Use custom fields
    bool useCustomFields_ = false;

    /// Currently loading parent issue data
    bool loadingParentIssueData_ = false;

    /// Current assignee
    int assigneeId_ = NULL_ID;

    /// Cached categories
    SimpleModel assigneeModel_;

    /// Current category
    int categoryId_ = NULL_ID;

    /// Cached categories
    SimpleModel categoryModel_;

    /// Custom fields
    qtredmine::CustomFields customFields_;

    /// Custom field values
    /// @param int Custom field ID
    /// @param QVector Values
    QMap<int, QVector<QString>> customFieldValues_;

    /// Custom field items
    /// @param int Custom field ID
    /// @param QPair QQuickItem GUI items (label and entry field)
    QMap<int, QPair<QQuickItem*, QQuickItem*>> customFieldItems_;

    /// Custom field models
    /// @param int Custom field ID
    /// @param SimpleModel Model
    QMap<int, SimpleModel*> customFieldModels_;

    /// Currently tracked issue
    qtredmine::Issue issue_;

    /// Parent issue ID
    int parentIssueId_ = NULL_ID;

    /// Current project
    int projectId_ = NULL_ID;

    /// Cached projects
    SimpleModel projectModel_;

    /// Current tracker
    int trackerId_ = NULL_ID;

    /// Cached trackers
    SimpleModel trackerModel_;

    /// Current version
    int versionId_ = NULL_ID;

    /// Cached categories
    SimpleModel versionModel_;

private:
    /**
     * @brief Load and refresh assignees in the GUI
     */
    void loadAssignees();

    /**
     * @brief Load and refresh categories in the GUI
     */
    void loadCategories();

    /**
     * @brief Load the current user
     */
    void loadCurrentUser();

    /**
     * @brief Load and refresh custom fields in the GUI
     */
    void loadCustomFields();

    /**
     * @brief Load and refresh projects in the GUI
     */
    void loadProjects();

    /**
     * @brief Load and refresh trackers in the GUI
     */
    void loadTrackers();

    /**
     * @brief Load and refresh versions in the GUI
     */
    void loadVersions();

    /**
     * @brief Refresh the GUI dependent on the current selections
     */
    void refreshGui();

public:
    /**
     * @brief Constructor for an IssueCreator object
     *
     * @param redmine Redmine connection object
     * @param mainWindow Main window object
     */
    explicit IssueCreator( qtredmine::SimpleRedmineClient* redmine, MainWindow* mainWindow );

    /**
     * @brief Destructor
     */
    ~IssueCreator();

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
     * @brief Set the currently tracked issue
     *
     * @param issue Currently tracked issue
     */
    void setCurrentIssue( qtredmine::Issue issue );

    /**
     * @brief Set the currently selected project ID
     *
     * @param id Project ID
     */
    void setProjectId( int id );

    /**
     * @brief Use custom fields
     *
     * @param useCustomFields true if custom fields should be used, false otherwise
     */
    void setUseCustomFields( bool useCustomFields );

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
     * @brief Slot to a selected category
     */
    void categorySelected( int index );

    /**
     * @brief Load data from parent issue
     */
    void loadParentIssueData();

    /**
     * @brief Slot to a selected project
     */
    void projectSelected( int index );

    /**
     * @brief Save the new issue
     */
    void save();

    /**
     * @brief Open the issue selector and load issue
     */
    void selectParentIssue();

    /**
     * @brief Slot to a selected tracker
     */
    void trackerSelected( int index );

    /**
     * @brief Use the currently tracked issue as parent issue
     */
    void useCurrentIssue();

    /**
     * @brief Use the currently tracked issue's parent as parent issue
     */
    void useCurrentIssueParent();

    /**
     * @brief Slot to a selected version
     */
    void versionSelected( int index );

signals:
    /**
     * @brief Emitted when the issue creator has been cancelled
     */
    void cancelled();

    /**
     * @brief Emitted when an issue has been created
     */
    void created( int issueId );

};

} // redtimer
