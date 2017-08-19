#pragma once

#include "qtredmine/Logging.h"

#include <QByteArray>
#include <QDataStream>
#include <QString>
#include <QtGlobal>

namespace redtimer {

#define NULL_ID -1

struct CliOptions
{
    QString command;

    qint32 assigneeId = NULL_ID;
    qint32 issueId    = NULL_ID;
    qint32 parentId   = NULL_ID;
    qint32 projectId  = NULL_ID;
    qint32 trackerId  = NULL_ID;
    qint32 versionId  = NULL_ID;

    QString externalId;
    QString externalParentId;

    QString subject;
    QString description;

    /**
     * @brief Serialise a CliOptions object
     *
     * @param options CliOptions object
     *
     * @return Serialised CliOptions object
     */
    static QByteArray serialise( const CliOptions& options );

    /**
     * @brief Deserialise a CliOptions object
     *
     * @param byteArray Serialised CliOptions object
     *
     * @return Deserialised CliOptions object
     */
    static CliOptions deserialise( const QByteArray& byteArray );

    /**
     * @brief Deserialise a CliOptions object
     *
     * @param device Device containing the serialised CliOptions object
     *
     * @return Deserialised CliOptions object
     */
    static CliOptions deserialise( QIODevice* device );
};

} // redtimer

inline QDebug
operator<<( QDebug debug, const redtimer::CliOptions& data )
{
    QDebugStateSaver saver( debug );
    DEBUGFIELDS(command)(assigneeId)(issueId)(parentId)(projectId)(trackerId)(versionId)(externalId)
               (externalParentId)(subject)(description);
    return debug;
}

inline QDataStream&
operator<<( QDataStream& out, const redtimer::CliOptions& options )
{
    out << options.command
           << options.assigneeId
           << options.issueId
           << options.parentId
           << options.projectId
           << options.trackerId
           << options.versionId
           << options.externalId
           << options.externalParentId
           << options.subject
           << options.description;

    return out;
}

inline QDataStream&
operator>>( QDataStream& in, redtimer::CliOptions& options )
{
    in >> options.command
       >> options.assigneeId
       >> options.issueId
       >> options.parentId
       >> options.projectId
       >> options.trackerId
       >> options.versionId
       >> options.externalId
       >> options.externalParentId
       >> options.subject
       >> options.description;

    return in;
}
