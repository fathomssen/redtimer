#include "qtredmine/Logging.h"
#include "CommandSender.h"

#include <QDataStream>
#include <QLocalSocket>
#include <QRegularExpression>
#include <QSettings>
#include <QVector>

#include <iostream>

using namespace redtimer;
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

    if( (singleProfile_ || finished_) && sockets_.count() == 0 )
        emit finished();

    RETURN();
}

void
CommandSender::readFromSocket( QLocalSocket* socket, const CliOptions& options )
{
    ENTER()(socket)(options);

    auto cb = [=]()
    {
        ENTER();

        CliOptions optionsIn = CliOptions::deserialise( socket );

        DEBUG()(optionsIn);

        std::string serverName = socket->serverName().toStdString();

        if( options.command == optionsIn.command )
        {
            if( optionsIn.command == "issue" )
            {
                if( optionsIn.issueId != NULL_ID )
                    cout << serverName << ": Current issue ID: " << optionsIn.issueId << endl;
                else
                    cout << serverName << ": No issue currently selected" << endl;
            }
            else
            {
                cout << serverName << ": Successfully sent command " << options.command.toStdString() << endl;
            }
        }
        else
        {
            cout << serverName << ": Could not send command " << options.command.toStdString() << endl;
        }

        socket->disconnectFromServer();

        RETURN();
    };

    connect( socket, &QLocalSocket::readyRead, cb );

    RETURN();
}

void
CommandSender::sendToAll( const CliOptions& options )
{
    ENTER()(options);

    singleProfile_ = false;

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
            sendToProfile( profileId, options );
    }

    finished_ = true;
    if( sockets_.count() == 0 )
        emit finished();

    RETURN();
}

void
CommandSender::sendToProfile( int profileId, const CliOptions& options )
{
    ENTER()(profileId)(options);

    QString uname = qgetenv( "USER" ); // UNIX
    if( uname.isEmpty() )
        uname = qgetenv( "USERNAME" ); // Windows
    QString serverName = QString("redtimer-%1-%2").arg(uname).arg(profileId);

    DEBUG()(serverName);

    QLocalSocket* socket = new QLocalSocket( this );
    sockets_.insert( socket, true );

    connect( socket, &QLocalSocket::connected,    [=](){ sendToSocket(socket, options); } );
    connect( socket, &QLocalSocket::disconnected, [=](){ deleteSocket(socket); } );

    socket->connectToServer( serverName, QIODevice::ReadWrite );
    if( !socket->waitForConnected() )
        deleteSocket( socket );

    RETURN();
}

void
CommandSender::sendToSocket( QLocalSocket* socket, const CliOptions& options )
{
    ENTER()(socket)(options);

    cout << socket->serverName().toStdString() << ": Sending command " << options.command.toStdString()
         << endl;

    QByteArray block = CliOptions::serialise( options );

    socket->write( block );

    if( socket->flush() )
        readFromSocket( socket, options );
    else
        socket->disconnectFromServer();

    RETURN();
}
