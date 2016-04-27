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

    /// Emit the cancelled signal upon closing
    bool cancelOnClose_ = true;

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

private:
    /**
     * @brief Load the current user
     */
    void loadCurrentUser();

    /**
     * @brief Load and refresh projects in the GUI
     */
    void loadProjects();

    /**
     * @brief Load and refresh trackers in the GUI
     */
    void loadTrackers();

public:
    /**
     * @brief Constructor for an IssueCreator object
     *
     * @param redmine Redmine connection object
     * @param parent Parent QObject
     */
    explicit IssueCreator( qtredmine::SimpleRedmineClient* redmine, QQuickView* parent = nullptr );

    /**
     * @brief Destructor
     */
    ~IssueCreator();

    /// @name Setters
    /// @{

    /**
     * @brief Set the parent issue ID
     *
     * @param id Parent issue ID
     */
    void setParentIssueId( int id );

    /// @}

public slots:
    /**
     * @brief Close the issue selector dialog
     */
    void closeWin();

    /**
     * @brief Display the issue selector dialog
     */
    void display();

private slots:
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
