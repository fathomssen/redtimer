#ifndef ISSUECREATOR_H
#define ISSUECREATOR_H

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

    /// Use custom fields
    bool useCustomFields_ = false;

    /// Emit the cancelled signal upon closing
    bool cancelOnClose_ = true;

    /// Current category
    int categoryId_ = NULL_ID;

    /// Cached categories
    SimpleModel categoryModel_;

    /// User ID which was used to login into Redmine
    int currentUserId_ = NULL_ID;

    /// Current project
    int projectId_ = NULL_ID;

    /// Cached projects
    SimpleModel projectModel_;

    /// Current tracker
    int trackerId_ = NULL_ID;

    /// Cached trackers
    SimpleModel trackerModel_;

    /// Custom fields
    qtredmine::CustomFields customFields_;

    /// Custom field items
    /// @param int Custom field ID
    /// @param Pair QQuickItem GUI items (label and entry field)
    QMap<int, QPair<QQuickItem*, QQuickItem*>> customFieldItems_;

    /// Custom field models
    /// @param int Custom field ID
    /// @param SimpleModel Model
    QMap<int, SimpleModel*> customFieldModels_;

private:
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
     * @brief Refresh the GUI dependent on the current selections
     */
    void refreshGui();

public:
    /**
     * @brief Constructor for an IssueCreator object
     *
     * @param redmine Redmine connection object
     * @param parent Parent window
     */
    explicit IssueCreator( qtredmine::SimpleRedmineClient* redmine,
                           Window* parent = nullptr );

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
     * @brief Set the parent issue ID
     *
     * @param id Parent issue ID
     */
    void setParentIssueId( int id );

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
     * @brief Slot to a selected category
     */
    void categorySelected( int index );

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

#endif // ISSUECREATOR_H
