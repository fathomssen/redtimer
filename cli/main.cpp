#include "qtredmine/Logging.h"
#include "redtimer/CliOptions.h"

#include "CommandSender.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QObject>
#include <QRegularExpression>
#include <QTimer>

#include <iostream>

using namespace redtimer;
using namespace std;

bool
parseCommandLine( QCoreApplication& app, QCommandLineParser& parser, CliOptions& options, QString& profileId,
                  QString& errmsg )
{
    parser.setApplicationDescription( "RedTimer Command Line Interface" );

    // Commands
    QMap<QString,QString> commands;
    commands.insert( "create", "Create a new issue" );
    commands.insert( "issue",  "Get the current issue ID" );
    commands.insert( "start",  "Start issue tracking" );
    commands.insert( "stop",   "Stop issue tracking" );

    QStringList descr;
    QMapIterator<QString, QString> cmd( commands );
    while( cmd.hasNext() )
    {
        cmd.next();

        QString data;
        QTextStream out( &data );
        out.setFieldAlignment( QTextStream::AlignLeft );
        out << qSetFieldWidth(10) << cmd.key() << cmd.value();
        descr.push_back( data );
    }

    parser.addPositionalArgument( "command", descr.join("\n"), "create|issue|start|stop" );

    // Program parameters
    parser.addOption( {"profile-id", "Redmine instance to send the command to", "ID"} );

    // Command parameters
    parser.addOption( {"assignee-id",        "Redmine assignee ID",      "ID"} );
    parser.addOption( {"issue-id",           "Redmine issue ID",         "ID"} );
    parser.addOption( {"parent-id",          "Redmine parent issue ID",  "ID"} );
    parser.addOption( {"project-id",         "Redmine project ID",       "ID"} );
    parser.addOption( {"tracker-id",         "Redmine tracker ID",       "ID"} );
    parser.addOption( {"version-id",         "Redmine version ID",       "ID"} );
    parser.addOption( {"external-id",        "External issue ID",        "text"} );
    parser.addOption( {"external-parent-id", "External parent issue ID", "text"} );
    parser.addOption( {"subject",            "Issue subject",            "text"} );
    parser.addOption( {"description",        "Issue description",        "text"} );

    // General parameters
    parser.addHelpOption();
    parser.addVersionOption();

    // Process command line options
    if( !parser.parse(app.arguments()) )
    {
        errmsg = parser.errorText();
        return false;
    }

    if( parser.isSet("help") )
        return false;


    // Get the command

    const QStringList positionalArguments = parser.positionalArguments();

    if( positionalArguments.isEmpty() )
    {
        errmsg = "No command specified.";
        return false;
    }

    if( positionalArguments.size() > 1 )
    {
        errmsg = "Several commands specified.";
        return false;
    }

    options.command = positionalArguments.first();

    if( commands.find(options.command) == commands.end() )
    {
        errmsg = QString("Command '%1' not found.").arg(options.command);
        return false;
    }

    auto getNumericId = [&]( const QString& option, qint32& id )
    {
        if( !parser.isSet(option) )
            return true;

        QString idString = parser.value( option );
        bool ok;
        id = idString.toInt( &ok );

        if( !ok )
        {
            errmsg = QString("Option '--%1' expects a numeric ID.").arg(option);
            return false;
        }

        return true;
    };

    auto getString = [&]( const QString& option, QString& str, bool allowWhitespaces )
    {
        if( !parser.isSet(option) )
            return true;

        str = parser.value( option );

        if( !allowWhitespaces && str.contains( QRegularExpression("\\W")) )
        {
            errmsg = QString("Option '--%1' must not contain whitespaces.").arg(option);
            return false;
        }

        return true;
    };

    if( !getString("profile-id", profileId, false) )
        return false;
    profileId = profileId.toLower();

    if( !getNumericId("assignee-id", options.assigneeId) )
        return false;

    if( !getNumericId("issue-id", options.issueId) )
        return false;

    if( !getNumericId("parent-id", options.parentId) )
        return false;

    if( !getNumericId("project-id", options.projectId) )
        return false;

    if( !getNumericId("tracker-id", options.trackerId) )
        return false;

    if( !getNumericId("version-id", options.versionId) )
        return false;

    if( !getString("external-id", options.externalId, false) )
        return false;

    if( !getString("external-parent-id", options.externalParentId, false) )
        return false;

    if( !getString("description", options.description, true) )
        return false;

    if( !getString("subject", options.subject, true) )
        return false;

    auto isValid = [&]( const QString option, bool allowed )
    {
        if( allowed && !parser.isSet(option) )
        {
            errmsg = QString("Option '--%1' required by command '%2'.").arg(option).arg(options.command);
            return false;
        }
        else if( !allowed && parser.isSet(option) )
        {
            errmsg = QString("Option '--%1' not allowed for command '%2'.").arg(option).arg(options.command);
            return false;
        }

        return true;
    };

    // Required: issue-id
    if( options.command == "start" )
    {
        // Required
        if( !isValid("issue-id", true) )
            return false;

        // Not allowed
        if( !isValid("assignee-id", false) )
            return false;
        if( !isValid("parent-id", false) )
            return false;
        if( !isValid("project-id", false) )
            return false;
        if( !isValid("tracker-id", false) )
            return false;
        if( !isValid("version-id", false) )
            return false;
        if( !isValid("external-id", false) )
            return false;
        if( !isValid("external-parent-id", false) )
            return false;
        if( !isValid("subject", false) )
            return false;
        if( !isValid("description", false) )
            return false;
    }
    else if( options.command == "create" )
    {
        // Required
        if( !isValid("project-id", true) )
            return false;
        if( !isValid("subject", true) )
            return false;

        // Not allowed
        if( !isValid("issue-id", false) )
            return false;

        // Exclusive
        if( parser.isSet("parent-id") && parser.isSet("external-parent-id") )
        {
            errmsg = "Options '--parent-id' and '--external-parent-id' may not be combined.";
            return false;
        }
    }
    else
    {
        // Not allowed
        if( !isValid("assignee-id", false) )
            return false;
        if( !isValid("issue-id", false) )
            return false;
        if( !isValid("parent-id", false) )
            return false;
        if( !isValid("project-id", false) )
            return false;
        if( !isValid("tracker-id", false) )
            return false;
        if( !isValid("version-id", false) )
            return false;
        if( !isValid("external-id", false) )
            return false;
        if( !isValid("external-parent-id", false) )
            return false;
        if( !isValid("subject", false) )
            return false;
        if( !isValid("description", false) )
            return false;
    }

    return true;
}

int main( int argc, char *argv[] )
{
    QCoreApplication app( argc, argv );

    app.setApplicationName( "RedTimerCLI" );

    // Command line options
    QCommandLineParser parser;
    CliOptions options;
    QString profileId;
    QString errmsg;

    if( !parseCommandLine( app, parser, options, profileId, errmsg ) )
    {
        if( !errmsg.isEmpty() )
            cout << errmsg.toStdString() << "\n\n";

        parser.showHelp( errmsg.isEmpty() ? 0 : 1 );
    }

    CommandSender* sender = new CommandSender( &app );
    QObject::connect( sender, &CommandSender::finished, &app, &QCoreApplication::quit );

    if( !profileId.isEmpty() )
        QTimer::singleShot( 0, [&](){ sender->sendToServer(profileId, options); } );
    else
        QTimer::singleShot( 0, [&](){ sender->sendToAll(options); } );

    return app.exec();
}
