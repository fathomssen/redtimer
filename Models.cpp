#include "logging.h"

#include "Models.h"

using namespace redtimer;
using namespace qtredmine;

SimpleItem::SimpleItem( const QString& name )
    : id_( -1 ),
      name_( name )
{
    ENTER()(id_)(name_);
    RETURN();
}

SimpleItem::SimpleItem( int id, const QString& name )
    : id_( id ),
      name_( name )
{
    ENTER()(id_)(name_);
    RETURN();
}

SimpleItem::SimpleItem( Redmine::Project project )
    : id_( project.id ),
      name_( project.name )
{
    ENTER()(id_)(name_);
    RETURN();
}

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

SimpleModel::SimpleModel( QObject* parent )
    : QAbstractListModel( parent )
{}

void
SimpleModel::insert( const SimpleItem& item )
{
    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    items_ << item;
    endInsertRows();
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
SimpleModel::data( const QModelIndex& index, int role ) const {
    if( index.row() < 0 || index.row() >= items_.count() )
        return QVariant();

    const SimpleItem& item = items_[index.row()];
    if( role == IdRole )
        return item.id();
    else if( role == NameRole )
        return item.name();
    return QVariant();
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
