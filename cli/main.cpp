#include "qtredmine/Logging.h"
#include "CommandSender.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QObject>
#include <QTimer>

#include <iostream>

using namespace std;

int main( int argc, char *argv[] )
{
    QCoreApplication app( argc, argv );

    app.setApplicationName( "RedTimerCLI" );

    // Command line options
    QCommandLineParser parser;
    parser.setApplicationDescription( "RedTimer Command Line Interface" );
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption( {{"i", "issue"}, "Switch to the specified issue", "issue ID"} );

    // Process command line options
    parser.process( app );

    // Build commands string
    QString cmds;

    if( parser.isSet("issue") )
        cmds.append( QString("%1:%2|").arg("issue").arg(parser.value("issue")) );

    if( cmds.isEmpty() )
    {
        cout << "No commands specified" << endl;
        return 1;
    }

    // Remove last pipe sign from cmds
    cmds.remove( -1, 1 );

    CommandSender* sender = new CommandSender( &app );
    QObject::connect( sender, &CommandSender::finished, &app, &QCoreApplication::quit );

    QTimer::singleShot( 0, [&](){ sender->send(cmds); } );

    return app.exec();
}
