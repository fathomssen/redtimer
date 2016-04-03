#include "logging.h"

#include "Models.h"

using namespace redtimer;
using namespace qtredmine;

SimpleItem::SimpleItem( const QString& name )
    : id_( NULL_ID ),
      name_( name )
{}

SimpleItem::SimpleItem( int id, const QString& name )
    : id_( id ),
      name_( name )
{}

SimpleItem::SimpleItem( const Enumeration& item )
    : id_( item.id ),
      name_( item.name )
{}

SimpleItem::SimpleItem( const IssueStatus& item )
    : id_( item.id ),
      name_( item.name )
{}

SimpleItem::SimpleItem( const Project& item )
    : id_( item.id ),
      name_( item.name )
{}

int
SimpleItem::id() const
{
    ENTER();
    RETURN( id_ );
}

QString
SimpleItem::name() const
{
    ENTER();
    RETURN( name_ );
}

IssueModel::IssueModel( QObject* parent )
    : QAbstractListModel( parent )
{}

void
IssueModel::insert( const Issue& item )
{
    ENTER()(item);

    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    items_ << item;
    endInsertRows();

    RETURN();
}

Issue
IssueModel::at( const int index ) const
{
    ENTER()(index);
    RETURN( items_.at(index) );
}

void
IssueModel::clear()
{
    ENTER();

    if( rowCount() == 0 )
        RETURN();

    beginRemoveRows( QModelIndex(), 0, rowCount()NULL_ID );
    items_.clear();
    endRemoveRows();

    RETURN();
}

int
IssueModel::rowCount( const QModelIndex& parent ) const {
    Q_UNUSED( parent );
    return items_.count();
}

QVariant
IssueModel::data( const QModelIndex& index, int role ) const
{
    ENTER()(index)(role);

    if( index.row() < 0 || index.row() >= items_.count() )
        RETURN( QVariant() );

    const Issue& item = items_[index.row()];

    if( role == IdRole )
        RETURN( item.id );
    else if( role == DescriptionRole )
        RETURN( item.description );
    else if( role == DoneRatioRole )
        RETURN( item.doneRatio );
    else if( role == SubjectRole )
        RETURN( item.subject );
    else if( role == AuthorRole )
        RETURN( QVariant::fromValue(item.author) );
    else if( role == CategoryRole )
        RETURN( QVariant::fromValue(item.category) );
    else if( role == PriorityRole )
        RETURN( QVariant::fromValue(item.priority) );
    else if( role == ProjectRole )
        RETURN( QVariant::fromValue(item.project) );
    else if( role == StatusRole )
        RETURN( QVariant::fromValue(item.status) );
    else if( role == TrackerRole )
        RETURN( QVariant::fromValue(item.tracker) );
    else if( role == CreatedOnRole )
        RETURN( item.createdOn );
    else if( role == DueDateRole )
        RETURN( item.dueDate );
    else if( role == EstimatedHoursRole )
        RETURN( item.estimatedHours );
    else if( role == StartDateRole )
        RETURN( item.startDate );
    else if( role == UpdatedOnRole )
        RETURN( item.updatedOn );
    else if( role == CustomFieldsRole )
        RETURN( QVariant::fromValue(item.customFields) );
    else
        RETURN( QVariant() );
}

QList<Issue>
IssueModel::data() const
{
    ENTER();
    RETURN( items_ );
}


QHash<int, QByteArray>
IssueModel::roleNames() const
{
    ENTER();

    QHash<int, QByteArray> roles;
    roles[IdRole]             = "id";
    roles[DescriptionRole]    = "description";
    roles[DoneRatioRole]      = "doneRatio";
    roles[SubjectRole]        = "subject";
    roles[AuthorRole]         = "author";
    roles[CategoryRole]       = "category";
    roles[PriorityRole]       = "priority";
    roles[ProjectRole]        = "project";
    roles[StatusRole]         = "status";
    roles[TrackerRole]        = "tracker";
    roles[CreatedOnRole]      = "createdOn";
    roles[DueDateRole]        = "dueDate";
    roles[EstimatedHoursRole] = "estimatedHours";
    roles[StartDateRole]      = "startDate";
    roles[UpdatedOnRole]      = "updatedOn";
    roles[CustomFieldsRole]   = "customFields";

    RETURN( roles );
}

SimpleModel::SimpleModel( QObject* parent )
    : QAbstractListModel( parent )
{}

void
SimpleModel::insert( const SimpleItem& item )
{
    ENTER()(item);

    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    items_ << item;
    endInsertRows();

    RETURN();
}

SimpleItem
SimpleModel::at( const int index ) const
{
    ENTER()(index);
    RETURN( items_.at(index) );
}

void
SimpleModel::clear()
{
    ENTER();

    if( rowCount() == 0 )
        RETURN();

    beginRemoveRows( QModelIndex(), 0, rowCount()NULL_ID );
    items_.clear();
    endRemoveRows();

    RETURN();
}

int
SimpleModel::rowCount( const QModelIndex& parent ) const {
    Q_UNUSED( parent );
    return items_.count();
}

QVariant
SimpleModel::data( const QModelIndex& index, int role ) const
{
    ENTER()(index)(role);

    if( index.row() < 0 || index.row() >= items_.count() )
        RETURN( QVariant() );

    const SimpleItem& item = items_[index.row()];

    if( role == IdRole )
        RETURN( item.id() );
    else if( role == NameRole )
        RETURN( item.name() );
    else
        RETURN( QVariant() );
}

QList<SimpleItem>
SimpleModel::data() const
{
    ENTER();
    RETURN( items_ );
}


QHash<int, QByteArray>
SimpleModel::roleNames() const
{
    ENTER();

    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";

    RETURN( roles );
}
