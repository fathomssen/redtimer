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

    // Command line options
    QCommandLineParser parser;
    parser.setApplicationDescription( "Redmine Time Tracker" );
    parser.addHelpOption();
    parser.addVersionOption();

    // Process command line options
    parser.process( app );

    app.setWindowIcon( QIcon(":/icons/clock_red.svg") );

    new RedTimer( &app );

    return app.exec();
}
