#ifndef ISSUESELECTOR_H
#define ISSUESELECTOR_H

#include "Models.h"

#include "qtredmine/SimpleRedmineClient.h"

#include <QObject>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QSortFilterProxyModel>

namespace redtimer {

class IssueSelector : public QObject
{
    Q_OBJECT

private:
    /// Redmine connection object
    qtredmine::SimpleRedmineClient* redmine_;

    /// Issue selector window object
    QQuickView* win_;

    /// Issue selector window context
    QQmlContext* ctx_;

    /// Issue selector item object
    QQuickItem* item_;

    /// List of issues in the GUI
    IssueModel issuesModel_;
    QSortFilterProxyModel issuesProxyModel_;

    /// Current project
    int projectId_;

    /// List of projects in the GUI
    SimpleModel projectModel_;

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
     * @brief Constructor for a IssueSelector object
     *
     * @param redmine Redmine connection object
     * @param parent Parent QObject
     */
    explicit IssueSelector( qtredmine::SimpleRedmineClient* redmine, QObject* parent = nullptr );

    /// @name Getters
    /// @{

    /**
     * @brief Get the currently selected project ID
     *
     * @return Project ID
     */
    int getProjectId() const;

    /**
     * @brief Get the issue selector window
     *
     * @return Issue selector window
     */
    QQuickView* window() const;

    /// @}

    /// @name Setters
    /// @{

    /**
     * @brief Set the currently selected project ID
     *
     * @param id Project ID
     */
    void setProjectId( int id );

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
    void updateIssues();

    /**
     * @brief Update projects and refresh project list
     */
    void updateProjects();

signals:
    /**
     * @brief Emitted when an issue has been selected
     */
    void selected( int issueId );
};

} // redtimer

#endif // ISSUESELECTOR_H
