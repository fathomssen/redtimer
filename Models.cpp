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
    return id_;
}

QString
SimpleItem::name() const
{
    return name_;
}

IssueModel::IssueModel( QObject* parent )
    : QAbstractListModel( parent )
{}

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

    beginRemoveRows( QModelIndex(), 0, rowCount()-1 );
    items_.clear();
    endRemoveRows();

    RETURN();
}

void
IssueModel::push_back( const Issue& item )
{
    ENTER()(item);

    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    items_.push_back( item );
    endInsertRows();

    RETURN();
}

void
IssueModel::push_front( const Issue& item )
{
    ENTER()(item);

    beginInsertRows( QModelIndex(), 0, 0 );
    items_.push_front( item );
    endInsertRows();

    RETURN();
}

bool
IssueModel::removeRows( int begin, int count, const QModelIndex& parent )
{
    int end = begin + count - 1;

    ENTER()(begin)(count)(end);

    if( end >= rowCount() )
        RETURN( false );

    beginRemoveRows( parent, begin, end );
    for( int i = begin; i <= end; ++i )
        items_.removeAt( i );
    endRemoveRows();

    RETURN( true );
}

bool
IssueModel::removeRowsFrom( int row )
{
    ENTER()(row);

    int end = rowCount() - 1;

    if( row > end )
        RETURN( false );

    int count = end - row + 1;

    RETURN( removeRows(row, count) );
}

int
IssueModel::rowCount( const QModelIndex& parent ) const
{
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
    else if( role == TextRole )
        RETURN( QString("#%1: %2").arg(QString::number(item.id)).arg(item.subject) );
    else if( role == FindRole )
        RETURN( QString("%1 %2").arg(item.subject).arg(item.description) );
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
    roles[TextRole]           = "text";
    roles[FindRole]           = "find";

    RETURN( roles );
}

SimpleModel::SimpleModel( QObject* parent )
    : QAbstractListModel( parent )
{}

void
SimpleModel::push_back( const SimpleItem& item )
{
    ENTER()(item);

    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    items_.push_back( item );
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

    beginRemoveRows( QModelIndex(), 0, rowCount()-1 );
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
    else if( role == TextRole )
        RETURN( QString("#%1: %2").arg(QString::number(item.id())).arg(item.name()) );
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
    roles[IdRole]   = "id";
    roles[NameRole] = "name";
    roles[TextRole] = "text";

    RETURN( roles );
}
