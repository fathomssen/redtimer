#include "MainWindow.h"
#include "ProfileSelector.h"

#include <QApplication>
#include <QCommandLineParser>

using namespace redtimer;
using namespace std;

int main(int argc, char* argv[])
{
    QApplication app( argc, argv );
    app.setApplicationName( "RedTimer" );

    app.setQuitOnLastWindowClosed( false );

    // Command line options
    QCommandLineParser parser;
    parser.setApplicationDescription( "Redmine Time Tracker" );
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption( {{"p", "profile"}, "Load settings for <profile>", "profile"} );

    // Process command line options
    parser.process( app );

    QString profileId = parser.value( "profile" );

    // Use default profile or show profile selector
    if( profileId.isEmpty() )
    {
        QStringList profileIds = ProfileSelector::profileIds();

        if( profileIds.count() == 0 )
            profileId = "Default";
        else if( profileIds.count() == 1 )
            profileId = profileIds[0];
        else
        {
            // Display a profile selection dialog
            ProfileSelector* profileSelector = new ProfileSelector( profileIds );

            QObject::connect( profileSelector, &ProfileSelector::selected,
                              [&](const QString& id){profileId = id;} );

            QEventLoop blocker;
            QObject::connect( profileSelector, &ProfileSelector::applied, [&](){blocker.exit(0);} );
            QObject::connect( profileSelector, &ProfileSelector::closed,  [&](){blocker.exit(1);} );
            profileSelector->display();

            int ret = blocker.exec();

            if( ret != 0 || profileId.isEmpty() )
            {
                profileSelector->deleteLater();
                return 1;
            }
        }
    }

    app.setWindowIcon( QIcon(":/icons/clock_red.svg") );

    new MainWindow( &app, profileId );

    return app.exec();
}
