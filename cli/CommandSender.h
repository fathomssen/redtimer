#pragma once

#include <QMap>
#include <QObject>

class QLocalSocket;

class CommandSender : public QObject
{
    Q_OBJECT

private:
    /// All commands have been sent
    bool finished_ = false;

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
     * @param cmds Commands string
     */
    void send( const QString cmds );

    /**
     * @brief Send commands to the running RedTimer instance with the specified profile selected
     *
     * @param profileId Profile ID
     * @param cmds Commands string
     */
    void send( int profileId, const QString cmds );

    /**
     * @brief Send commands to the running RedTimer instance connected to by the specified socket
     *
     * @param socket Socket connected to the RedTimer instance
     * @param cmds Commands string
     */
    void send( QLocalSocket* socket, const QString cmds );
};
