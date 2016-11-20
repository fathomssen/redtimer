#include "qtredmine/Logging.h"
#include "CommandSender.h"

#include <QDataStream>
#include <QLocalSocket>
#include <QRegularExpression>
#include <QSettings>
#include <QVector>

#include <iostream>

using namespace std;

CommandSender::CommandSender( QObject*parent )
    : QObject( parent )
{}

void
CommandSender::deleteSocket( QLocalSocket* socket )
{
    ENTER()(socket)(sockets_)(finished_);

    if( sockets_.find(socket) == sockets_.end() )
        RETURN();

    sockets_.remove( socket );
    socket->abort();
    socket->deleteLater();

    if( finished_ && sockets_.count() == 0 )
        emit finished();

    RETURN();
}

void
CommandSender::send( QString cmds )
{
    ENTER()(cmds);

    QSettings settings( QSettings::IniFormat, QSettings::UserScope, "Thomssen IT", "RedTimer", this );

    QStringList groups = settings.childGroups();
    for( const auto& group : groups )
    {
        QRegularExpressionMatch match;
        if( !group.contains(QRegularExpression("profile-(\\d+)"), &match) )
            continue;

        bool ok;
        int profileId = match.captured(1).toInt( &ok );

        if( ok )
            send( profileId, cmds );
    }

    finished_ = true;
    if( sockets_.count() == 0 )
        emit finished();

    RETURN();
}

void
CommandSender::send( int profileId, QString cmds )
{
    ENTER()(profileId)(cmds);

    QString uname = qgetenv( "USER" ); // UNIX
    if( uname.isEmpty() )
        uname = qgetenv( "USERNAME" ); // Windows
    QString serverName = QString("redtimer-%1-%2").arg(uname).arg(profileId);

    DEBUG()(serverName);

    QLocalSocket* socket = new QLocalSocket( this );
    sockets_.insert( socket, true );

    connect( socket, &QLocalSocket::connected,    [=](){ send(socket, cmds); } );
    connect( socket, &QLocalSocket::disconnected, [=](){ deleteSocket(socket); } );

    socket->connectToServer( serverName, QIODevice::WriteOnly );
    if( !socket->waitForConnected() )
        deleteSocket( socket );

    RETURN();
}

void
CommandSender::send( QLocalSocket* socket, const QString cmds )
{
    ENTER()(socket)(cmds);

    QByteArray block;
    QDataStream out( &block, QIODevice::WriteOnly );
    out.setVersion( QDataStream::Qt_5_5 );
    out << cmds;

    socket->write( block );
    socket->flush();
    socket->disconnectFromServer();

    RETURN();
}
