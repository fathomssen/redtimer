#ifndef MODELITEMS_H
#define MODELITEMS_H

#include "qtredmine/SimpleRedmineClient.h"

#include <QAbstractListModel>
#include <QObject>
#include <QString>

namespace redtimer {

/**
 * @brief Class that represents a simple item consisting of an ID and a name
 */
class SimpleItem
{
private:
    /// ID
    int id_;

    /// Name
    QString name_;

public:
    /**
     * @brief Constructor for a simple item by name
     *
     * This constructor will set the ID to \c NULL_ID
     *
     * @param name Name to be set
     */
    SimpleItem( const QString& name );

    /**
     * @brief Constructor for a simple item by ID and name
     *
     * @param id ID to be set
     * @param name Name to be set
     */
    SimpleItem( int id, const QString& name );

    /**
     * @brief Constructor for a simple item by an enumeration
     *
     * @param item Enumeration data to be used for the simple item
     */
    SimpleItem( const qtredmine::Enumeration& item );

    /**
     * @brief Constructor for a simple item by an issue status
     *
     * @param item Issue status data to be used for the simple item
     */
    SimpleItem( const qtredmine::IssueStatus& item );

    /**
     * @brief Constructor for a simple item by a project
     *
     * @param item Project data to be used for the simple item
     */
    SimpleItem( const qtredmine::Project& item );

    /// @name
    /// @{

    /**
     * @brief Get the ID
     *
     * @return The ID
     */
    int id() const;

    /**
     * @brief Get the name
     *
     * @return The name
     */
    QString name() const;

    /// @}
};

/**
 * @brief Class that represents the model of an issue
 */
class IssueModel : public QAbstractListModel
{
    Q_OBJECT

private:
    /// Internal issue cache
    QList<qtredmine::Issue> items_;

public:
    /// Issue model roles
    enum IssueRoles {
        IdRole = Qt::UserRole + 1,
        DescriptionRole,
        DoneRatioRole,
        SubjectRole,
        AuthorRole,
        CategoryRole,
        PriorityRole,
        ProjectRole,
        StatusRole,
        TrackerRole,
        CreatedOnRole,
        DueDateRole,
        EstimatedHoursRole,
        StartDateRole,
        UpdatedOnRole,
        CustomFieldsRole,
        TextRole,
        FindRole,
    };

    /**
     * @brief Default constructor
     *
     * @param parent Parent QObject
     */
    IssueModel( QObject* parent = nullptr );

    /// @name Getters
    /// @{

    /**
     * @brief Get the issue at the specified index
     *
     * @param index Index within the issue model
     *
     * @return The issue at the specified index
     */
    qtredmine::Issue at( const int index ) const;

    /**
     * @brief Get all issues from the model
     *
     * @return A list of all issues in the model
     */
    QList<qtredmine::Issue> data() const;

    /**
     * @brief Get the row count
     *
     * @param parent Parent model index
     *
     * @return Number of rows in the model
     */
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;

    /// @}

    /// @name Operations
    /// @{

    /**
     * @brief Clear the issue model, i.e. remove all entries
     */
    void clear();

    /**
     * @brief Append an issue to the end of the model
     *
     * @param item Issue to append
     */
    void push_back( const qtredmine::Issue& item );

    /**
     * @brief Append an issue to the front of the model
     *
     * @param item Issue to append
     */
    void push_front( const qtredmine::Issue& item );

    /**
     * @brief Remove rows from the model
     *
     * @param begin First row to remove
     * @param count Number of rows to remove
     * @param parent Parent model index
     *
     * @return True if rows have successfully been removed, false otherwise
     */
    bool removeRows( int begin, int count, const QModelIndex& parent = QModelIndex() );

    /**
     * @brief Remove rows starting with the specified row
     *
     * @param row Row to start delete from
     *
     * @return True if rows have successfully been removed, false otherwise
     */
    bool removeRowsFrom( int row );

    /// @}

protected:
    /**
     * @brief Get the issue at the specified index from the model
     *
     * @param index Index to fetch
     * @param role Role to fetch
     *
     * @return The issue at the specified index
     */
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

    /**
     * @brief Get a list of all role names in the model
     *
     * @return The list of all role names
     */
    QHash<int, QByteArray> roleNames() const;
};

/**
 * @brief Class that represents the model of a simple item
 */
class SimpleModel : public QAbstractListModel
{
    Q_OBJECT

private:
    /// Internal simple item cache
    QList<SimpleItem> items_;

public:
    /// Simple model roles
    enum SimpleRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TextRole
    };

    /**
     * @brief Default constructor
     *
     * @param parent Parent QObject
     */
    SimpleModel( QObject* parent = nullptr );

    /// @name Getters
    /// @{

    /**
     * @brief Get the simple item at the specified index
     *
     * @param index Index within the simple item model
     *
     * @return The simple item at the specified index
     */
    SimpleItem at( const int index ) const;

    /**
     * @brief Get all simple items from the model
     *
     * @return A list of all simple items in the model
     */
    QList<SimpleItem> data() const;

    /**
     * @brief Get the row count
     *
     * @param parent Parent model index
     *
     * @return Number of rows in the model
     */
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;

    /// @}

    /// @name Operations
    /// @{

    /**
     * @brief Clear the issue model, i.e. remove all entries
     */
    void clear();

    /**
     * @brief Append a simple item to the end of the model
     *
     * @param item Simple item to append
     */
    void push_back( const SimpleItem& item );

    /// @}

protected:
    /**
     * @brief Get the simple item at the specified index from the model
     *
     * @param index Index to fetch
     * @param role Role to fetch
     *
     * @return The simple item at the specified index
     */
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

    /**
     * @brief Get a list of all role names in the model
     *
     * @return The list of all role names
     */
    QHash<int, QByteArray> roleNames() const;
};

/**
 * @brief QDebug stream operator for a simple item
 *
 * @param debug QDebug stream
 * @param item Simple item
 *
 * @return QDebug stream
 */
inline QDebug
operator<<( QDebug debug, const SimpleItem& item )
{
    QDebugStateSaver saver( debug );
    debug.nospace() << "[" << item.id() << ", \"" << item.name() << "\"]";

    return debug;
}

/**
 * @brief QDebug stream operator for an issue model
 *
 * @param debug QDebug stream
 * @param model Issue model
 *
 * @return QDebug stream
 */
inline QDebug
operator<<( QDebug debug, const IssueModel& model )
{
    QDebugStateSaver saver( debug );

    for( const auto& item : model.data() )
        debug.nospace() << item;

    return debug;
}

/**
 * @brief QDebug stream operator for a simple model
 *
 * @param debug QDebug stream
 * @param model Simple model
 *
 * @return QDebug stream
 */
inline QDebug
operator<<( QDebug debug, const SimpleModel& model )
{
    QDebugStateSaver saver( debug );

    for( const auto& item : model.data() )
        debug.nospace() << item;

    return debug;
}

} // redtimer

#endif // MODELITEMS_H
