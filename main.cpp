#include "RedTimer.h"

#include <QApplication>
#include <QCommandLineParser>

#include <memory>

using namespace redtimer;
using namespace std;

int main(int argc, char* argv[])
{
    QApplication app( argc, argv );
    QApplication::setApplicationName( "RedTimer" );
    QApplication::setApplicationVersion( "0.0.4" );

    // Command line options
    QCommandLineParser parser;
    parser.setApplicationDescription( "Redmine Time Tracker" );
    parser.addHelpOption();
    parser.addVersionOption();

    // Disable tray icon
    QCommandLineOption noTrayOption( QStringList() << "no-tray-icon",
                                     QApplication::translate("main", "Do not provide a tray icon.") );
    parser.addOption( noTrayOption );

    // Process command line options
    parser.process( app );

    app.setWindowIcon( QIcon(":/icons/clock_red.svg") );

    new RedTimer( &app, !parser.isSet(noTrayOption) );

    return app.exec();
}
