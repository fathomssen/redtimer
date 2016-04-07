#ifndef MODELITEMS_H
#define MODELITEMS_H

#include "qtredmine/SimpleRedmineClient.h"

#include <QAbstractListModel>
#include <QObject>
#include <QString>

namespace redtimer {

class SimpleItem
{
private:
    int id_;
    QString name_;

public:
    SimpleItem( const QString& name );
    SimpleItem( int id, const QString& name );

    SimpleItem( const qtredmine::Enumeration& item );
    SimpleItem( const qtredmine::IssueStatus& item );
    SimpleItem( const qtredmine::Project& item );

    int id() const;
    QString name() const;
};

class IssueModel : public QAbstractListModel
{
    Q_OBJECT

private:
    QList<qtredmine::Issue> items_;

public:
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

    IssueModel( QObject* parent = nullptr );

    qtredmine::Issue at( const int index ) const;
    void clear();
    QList<qtredmine::Issue> data() const;
    void push_back( const qtredmine::Issue& item );
    void push_front( const qtredmine::Issue& item );
    bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() );

    /**
     * @brief Remove rows starting with the specified row
     *
     * @param row Row to start delete from
     *
     * @return true upon success, false otherwise
     */
    bool removeRowsFrom( int row );

    int rowCount( const QModelIndex& parent = QModelIndex() ) const;

protected:
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

    QHash<int, QByteArray> roleNames() const;
};

class SimpleModel : public QAbstractListModel
{
    Q_OBJECT

private:
    QList<SimpleItem> items_;

public:
    enum SimpleRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TextRole
    };

    SimpleModel( QObject* parent = nullptr );

    void clear();

    QList<SimpleItem> data() const;

    void push_back( const SimpleItem& item );

    SimpleItem at( const int index ) const;

    int rowCount( const QModelIndex& parent = QModelIndex() ) const;

protected:
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

    QHash<int, QByteArray> roleNames() const;
};

inline QDebug
operator<<( QDebug debug, const SimpleItem& item )
{
    QDebugStateSaver saver( debug );
    debug.nospace() << "[" << item.id() << ", \"" << item.name() << "\"]";

    return debug;
}

inline QDebug
operator<<( QDebug debug, const IssueModel& model )
{
    QDebugStateSaver saver( debug );

    for( const auto& item : model.data() )
        debug.nospace() << item;

    return debug;
}

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
