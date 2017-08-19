#include "qtredmine/Logging.h"
#include "redtimer/CliOptions.h"

namespace redtimer {

QByteArray
CliOptions::serialise( const CliOptions& options )
{
    ENTER()(options);

    QByteArray byteArray;

    QDataStream stream( &byteArray, QIODevice::WriteOnly );
    stream.setVersion( QDataStream::Qt_5_5 );
    stream << options;

    RETURN( byteArray );
}

CliOptions
CliOptions::deserialise( const QByteArray& byteArray )
{
    ENTER()(byteArray);

    CliOptions options;

    QDataStream stream( byteArray );
    stream.setVersion( QDataStream::Qt_5_5 );
    stream >> options;

   RETURN( options );
}

CliOptions
CliOptions::deserialise( QIODevice* device )
{
    ENTER()(device);

    CliOptions options;

    QDataStream stream( device );
    stream.setVersion( QDataStream::Qt_5_5 );
    stream >> options;

   RETURN( options );
}

} // redtimer
