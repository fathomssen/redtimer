#pragma once

#include "redtimer/CliOptions.h"

#include <QMap>
#include <QObject>

class QLocalSocket;

class CommandSender : public QObject
{
    Q_OBJECT

private:
    /// All commands have been sent
    bool finished_ = false;

    /// Just one profile receives a command
    bool singleProfile_ = true;

    /// Sockets
    QMap<QLocalSocket*, bool> sockets_;

public:
    /**
     * @brief Constructor
     *
     * @param parent Parent QObject
     */
    explicit CommandSender( QObject* parent = nullptr );

signals:
    /**
     * @brief Sending has finished
     */
    void finished();

public slots:
    /**
     * @brief Delete the local socket and emit finished() signal if all sockets are deleted
     *
     * @param socket Socket to delete
     */
    void deleteSocket( QLocalSocket* socket );

    /**
     * @brief Send commands to all running RedTimer instances
     *
     * @param options Options
     */
    void sendToAll( const redtimer::CliOptions& options );

    /**
     * @brief Send commands to the running RedTimer instance with the specified profile selected
     *
     * @param profileId Profile ID
     * @param options Options
     */
    void sendToProfile( int profileId, const redtimer::CliOptions& options );

    /**
     * @brief Send commands to the running RedTimer instance connected to by the specified socket
     *
     * @param socket Socket connected to the RedTimer instance
     * @param options Options
     */
    void sendToSocket( QLocalSocket* socket, const redtimer::CliOptions& options );
};
