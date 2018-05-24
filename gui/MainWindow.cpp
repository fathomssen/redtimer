#include "qtredmine/Logging.h"
#include "qtredmine/SimpleRedmineTypes.h"
#include "redtimer/CliOptions.h"

#include "IssueCreator.h"
#include "IssueSelector.h"
#include "MainWindow.h"
#include "Settings.h"

#include <QEventLoop>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QMenu>
#include <QNetworkInterface>
#include <QObject>
#include <QSystemSemaphore>
#include <QTime>

using namespace qtredmine;
using namespace std;

namespace redtimer {

#define MSG_CANNOT_PROCEDE "Cannot procede without a connection"

MainWindow::MainWindow( QApplication* parent, QString profileId )
    : Window( "MainWindow", this ),
      app_( parent )
{
    ENTER();

    // Connect to Redmine
    redmine_ = new SimpleRedmineClient( this );

    // Settings initialisation
    settings_ = new Settings( this, profileId );

    // Main window initialisation
    installEventFilter( this );
    setTitle( "RedTimer" );

    setWindowData( settings_->windowData()->mainWindow );

    if( profileData()->hidden )
        hide();
    else
        display();

    if( !profileData()->isValid() )
    {
        connected_ = false;
        settings_->display();
    }

    // Main window access members
    qmlCounter_ = qml( "counter" );

    // Shortcuts
    shortcutCreateIssue_ = new QxtGlobalShortcut( this );
    shortcutSelectIssue_ = new QxtGlobalShortcut( this );
    shortcutStartStop_   = new QxtGlobalShortcut( this );
    shortcutToggle_      = new QxtGlobalShortcut( this );
    connect( shortcutCreateIssue_, &QxtGlobalShortcut::activated, this, &MainWindow::createIssue );
    connect( shortcutSelectIssue_, &QxtGlobalShortcut::activated, this, &MainWindow::selectIssue );
    connect( shortcutStartStop_,   &QxtGlobalShortcut::activated, this, &MainWindow::startStop );
    connect( shortcutToggle_,      &QxtGlobalShortcut::activated, this, &MainWindow::toggle );

    // Timer initialisation
    timer_ = new QTimer( this );
    timer_->setTimerType( Qt::VeryCoarseTimer );
    timer_->setInterval( 1000 );

    // Placeholders sets
    qml("entryComment")->setProperty( "placeholderText", quickComment_ );

    // Initially connect and update the GUI
    reconnect();
    refreshGui();

    // Notify upon connection status change
    connect( redmine_, &SimpleRedmineClient::connectionChanged, this, &MainWindow::notifyConnectionStatus );

    setCtxProperty( "activityModel",     &activityModel_ );
    setCtxProperty( "issueStatusModel",  &issueStatusModel_ );
    setCtxProperty( "recentIssuesModel", &recentIssues_ );

    // Set transient window parent
    settings_->setTransientParent( this );

    // Connect the create issue button
    connect( qml("createIssue"), SIGNAL(clicked()), this, SLOT(createIssue()) );

    // Connect the settings button
    connect( qml("settings"), SIGNAL(clicked()), settings_, SLOT(display()) );

    // Connect the reload button
    connect( qml("reload"), SIGNAL(clicked()), this, SLOT(reconnectAndRefreshGui()) );

    // Connect the issue selector button
    connect( qml("selectIssue"), SIGNAL(clicked()), this, SLOT(selectIssue()) );

    // Connect the combobox text field
    connect( qml("quickPick"), SIGNAL(accepted()), this, SLOT(loadIssueFromTextField()) );

    // Connect the combobox list
    connect( qml("quickPick"), SIGNAL(activated(int)), this, SLOT(loadIssueFromList(int)) );

    // Connect the start/stop button
    connect( qml("startStop"), SIGNAL(clicked()), this, SLOT(startStop()) );

    // Connect the activity selected signal to the activitySelected slot
    connect( qml("activity"), SIGNAL(activated(int)), this, SLOT(activitySelected(int)) );

    // Connect the issueStatus selected signal to the issueStatusSelected slot
    connect( qml("issueStatus"), SIGNAL(activated(int)), this, SLOT(issueStatusSelected(int)) );

    // Connect the counter text field signals to pause or resume the displayed counted time
    connect( qml(), SIGNAL(counterEntered()), this, SLOT(pauseCounterGui()) );
    connect( qml("counter"), SIGNAL(editingFinished()), this, SLOT(resumeCounterGui()) );

    // Connect the settings saved signal to the settingsApplied (including reconnect) slot
    connect( settings_, &Settings::applied, this, &MainWindow::settingsApplied );

    // Connect the timer to the tracking counter
    connect( timer_, &QTimer::timeout, this, &MainWindow::refreshCounter );

    // Initially check the Redmine connection
    redmine_->checkConnectionStatus();

    qml("quickPick")->setProperty( "editText", quickPick_ );

    // Issue selector initialisation
    issueSelector_ = new IssueSelector( redmine_, this );
    issueSelector_->setTransientParent( this );

    // Connect the issue selected signal to the setIssue slot
    connect( issueSelector_, &IssueSelector::selected, [=](int issueId)
    {
        ENTER()(issueId);

        profileData()->projectId = issueSelector_->getProjectId();
        loadIssue( issueId );

        RETURN();
    } );

    server = new QLocalServer( this );
    server->setSocketOptions( QLocalServer::UserAccessOption );
    connect( server, &QLocalServer::newConnection, this, &MainWindow::receiveCommand );
    initServer();

    initialised_ = true;

    RETURN();
}

void
MainWindow::activitySelected( int index )
{
    ENTER()(index);

    activityId_ = activityModel_.at(index).id();
    DEBUG()(activityId_);

    RETURN();
}

void
MainWindow::addRecentIssue( qtredmine::Issue issue )
{
    ENTER()(issue);

    // If found, remove the new issue from the list
    for( int i = 0; i < recentIssues_.rowCount(); ++i )
        if( recentIssues_.at(i).id == issue.id )
            recentIssues_.removeRow( i );

    // Add the issue to the top of the list
    recentIssues_.push_front( issue );

    // Crop the list after numRecentIssues entries
    int numRecentIssues = profileData()->numRecentIssues;
    if( numRecentIssues != -1 )
        recentIssues_.removeRowsFrom( numRecentIssues );

    // Reset the quickPick text field
    qml("quickPick")->setProperty( "currentIndex", -1 );
    qml("quickPick")->setProperty( "editText", quickPick_ );

    saveSettings();

    RETURN();
}

bool
MainWindow::connected()
{
    ENTER()(connected_);
    RETURN( connected_ );
}

double
MainWindow::counter()
{
    ENTER()(counterDiff_);

    int value = counterNoDiff() + counterDiff_;

    RETURN( value );
}

double
MainWindow::counterGui()
{
    ENTER();

    if( !qml("counter") )
        RETURN( 0 );

    int value = 0;
    QTime time = SimpleRedmineClient::getTime( qml("counter")->property("text").toString() );
    if( time.isValid() )
        value = time.hour()*3600 + time.minute()*60 + time.second();

    RETURN( value );
}

double
MainWindow::counterNoDiff()
{
    ENTER();

    int value = lastCounterUpdated_.secsTo(QDateTime::currentDateTimeUtc());

    RETURN( value );
}

void
MainWindow::createIssue()
{
    ENTER();

    // Save time on current issue if timer is running
    if( timer_->isActive() && issue_.id != NULL_ID )
        stop( true, false );

    if( !timer_->isActive() )
        startTimer();

    const ProfileData* data = profileData();

    // Make sure that the main window is visible
    display();

    // Display the issue creator with the current issue as parent
    IssueCreator* issueCreator = new IssueCreator( redmine_, this );
    issueCreator->setTransientParent( this );
    issueCreator->setCurrentIssue( issue_ );
    issueCreator->setProjectId( data->projectId );
    issueCreator->setUseCustomFields( data->useCustomFields );
    issueCreator->display();

    // Empty the issue information and set ID to NULL_ID
    resetGui( "<New issue>" );
    issue_.id = NULL_ID;

    // Connect the issue selected signal to the setIssue slot
    connect( issueCreator, &IssueCreator::cancelled, [=]()
    {
        if( recentIssues_.rowCount() )
            loadIssue( recentIssues_.at(0).id );
        else
            resetGui();
    } );

    // Connect the issue selected signal to the setIssue slot
    connect( issueCreator, &IssueCreator::created, [=]( int issueId )
    {
        loadIssue( issueId, true, true );
        profileData()->projectId = issueCreator->getProjectId();
    } );

    RETURN();
}

void
MainWindow::display()
{
    ENTER();

#ifdef Q_OS_OSX
    TransformProcessType( &psn_, kProcessTransformToForegroundApplication );
    ShowHideProcess( &psn_, true );
#endif

    raise();
    requestActivate();
    showNormal();

    hidden_ = false;

    RETURN();
}

void
MainWindow::hide()
{
    ENTER();

    hidden_ = true;

#ifdef Q_OS_OSX
    TransformProcessType( &psn_, kProcessTransformToUIElementApplication );
    ShowHideProcess( &psn_, true );
#else
    Window::hide();
#endif

    RETURN();
}

bool
MainWindow::hidden()
{
    ENTER();
    RETURN( hidden_ );
}

bool
MainWindow::event( QEvent* event )
{
    // Try to reconnect upon window focus
    if( event->type() == QEvent::FocusIn )
    {
        DEBUG() << "Received window activated signal";

        if( initialised_ )
            reconnect();
    }

    // Control closing behaviour depending on tray icon usage
    if( event->type() == QEvent::Close )
    {
        DEBUG() << "Received close signal";

        if( trayIcon_ && profileData()->closeToTray )
            hide();
        else
            exit();

        event->ignore();
        return true;
    }

    return Window::event( event );
}

void
MainWindow::exit()
{
    ENTER()(initialised_);

    if( !initialised_ )
    {
        if( server )
            server->close();

        DEBUG() << "Quitting app";
        app_->quit();

        RETURN();
    }

    // Show warning on close and if timer is running
    if( timer_->isActive() || counterGui() != 0 )
    {
        DEBUG() << "Received close event while timer is running";

        int ret = QMessageBox::question( qobject_cast<QWidget*>(this), tr("RedTimer"),
                                         tr("The timer is currently running.\n"
                                            "Do you want to save the logged time?"),
                                         QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                         QMessageBox::Cancel );

        switch( ret )
        {
            case QMessageBox::Cancel:
                // Ignore the event
                DEBUG() << "Ignoring the close event";
                return;

            case QMessageBox::Save:
            {
                DEBUG() << "Saving time entry before closing the application";

                // Only go on with closing the window if saving was successful
                // If saving was successful before the blocker has been started, do not start it at all
                QEventLoop* blocker = new QEventLoop();
                bool startBlocker = true;
                connect( this, &MainWindow::timeEntrySaved, [&]()
                {
                    startBlocker = false;
                    blocker->exit();
                } );

                stop();

                if( startBlocker )
                    blocker->exec();
            }

            default:
                DEBUG() << "Closing the application";
                break;
        }
    }

    if( trayIcon_ )
        trayIcon_->hide();

    // Save settings
    saveSettings();

    server->close();

    app_->quit();

    RETURN();
}

QString
MainWindow::getServerName( QString suffix )
{
    ENTER()(suffix);

    QString uname = qgetenv( "USER" ); // UNIX
    if( uname.isEmpty() )
        uname = qgetenv( "USERNAME" ); // Windows

    QString serverName = QString("redtimer-%1").arg(uname);

    if( !suffix.isEmpty() )
        serverName = QString("%1-%2").arg(serverName).arg(suffix);

    RETURN( serverName );
}

void
MainWindow::initServer()
{
    ENTER();

    server->close();

    if( profileData()->startLocalServer )
    {
        QString serverName = getServerName( QString::number(profileData()->id) );

        if( server->listen(serverName) )
            DEBUG() << "Listening on socket";
        else
            DEBUG() << server->errorString();
    }

    RETURN();
}

void
MainWindow::initTrayIcon()
{
    ENTER();

    const bool useSystemTrayIcon = profileData()->useSystemTrayIcon;

    // Create tray icon if desired and not yet available
    if( !trayIcon_ && useSystemTrayIcon && QSystemTrayIcon::isSystemTrayAvailable() )
    {
        trayIcon_ = new QSystemTrayIcon( this );
        trayIcon_->setIcon( QIcon(ICON_CLOCK_STOP) );
        trayIcon_->show();

        QMenu* trayMenu = new QMenu( "RedTimer", qobject_cast<QWidget*>(this) );
        trayMenu->addAction( QIcon(":/icons/clock.svg"), tr("S&how/hide"), this, SLOT(toggle()) );
        trayMenu->addAction( QIcon(":/open-iconic/svg/media-play.svg"), tr("S&tart/stop"),
                             this, SLOT(startStop()) );
        trayMenu->addAction( QIcon(":/open-iconic/svg/plus.svg"), tr("&New issue"),
                             this, SLOT(createIssue()) );
        trayMenu->addAction( QIcon(":/open-iconic/svg/cog.svg"), tr("S&ettings"),
                             [this](){display(); settings_->display();} );
        trayMenu->addAction( QIcon(":/open-iconic/svg/power-standby.svg"), tr("E&xit"), this, SLOT(exit()) );
        trayIcon_->setContextMenu( trayMenu );

        // Connect the tray icon to the window show slot
        connect( trayIcon_, &QSystemTrayIcon::activated, this, &MainWindow::trayEvent );
    }
    // Hide tray icon if desired and currently shown
    else if( trayIcon_ && !useSystemTrayIcon )
    {
        trayIcon_->hide();
        delete trayIcon_;
        trayIcon_ = nullptr;
    }

    RETURN();
}

void
MainWindow::issueStatusSelected( int index )
{
    ENTER();

    issue_.status.id = issueStatusModel_.at(index).id();
    DEBUG()(index)(issue_.status.id);

    updateIssueStatus( issue_.status.id );

    RETURN();
}

void
MainWindow::loadActivities()
{
    ENTER();

    if( !connected() )
        RETURN();

    ++callbackCounter_;
    redmine_->retrieveTimeEntryActivities( [&]( Enumerations activities, RedmineError redmineError,
                                                QStringList errors )
    {
        CBENTER()(redmineError)(errors);

        if( !connected() )
            CBRETURN();

        if( redmineError != RedmineError::NO_ERR )
        {
            QString errorMsg = tr("Could not load activities.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        int currentIndex = 0;

        activityModel_.clear();
        activityModel_.push_back( SimpleItem(NULL_ID, "Choose activity") );
        for( const auto& activity : activities )
        {
            if( activity.id == activityId_ )
                currentIndex = activityModel_.rowCount();

            activityModel_.push_back( SimpleItem(activity) );
        }

        DEBUG()(activityModel_)(activityId_)(currentIndex);

        qml("activity")->setProperty( "currentIndex", -1 );
        qml("activity")->setProperty( "currentIndex", currentIndex );

        CBRETURN();
    } );

    RETURN();
}

void
MainWindow::loadIssueFromList( int index )
{
    ENTER()(index);

    loadIssue( recentIssues_.at(index).id );

    RETURN();
}

void
MainWindow::loadIssueFromTextField()
{
    ENTER();

    bool ok;
    int issueId = qml("quickPick")->property("editText").toInt( &ok );
    if( !ok )
    {
        message( "Issue ID may consist of digits only", QtWarningMsg );
        RETURN();
    }

    qml("startStop")->setProperty( "focus", true );

    if( !issueId )
    {
        qml("quickPick")->setProperty( "editText", quickPick_ );
        RETURN();
    }

    loadIssue( issueId );

    RETURN();
}

void
MainWindow::loadIssue( int issueId, bool startTimer, bool saveNewIssue )
{
    ENTER()(issueId)(startTimer)(saveNewIssue);

    if( !connected() )
        RETURN();

    // If the timer is currently active, save the currently logged time first
    // If there will be no new issue selected, stop the timer
    if( startTimer && timer_->isActive() )
        stop( true, issueId == NULL_ID );

    if( saveNewIssue )
        issue_.id = issueId;

    if( issueId == NULL_ID )
    {
        resetGui();
        RETURN();
    }

    ++callbackCounter_;
    redmine_->retrieveIssue( [=]( Issue issue, RedmineError redmineError, QStringList errors )
    {
        CBENTER()(issue)(redmineError)(errors);

        if( !connected() )
            CBRETURN();

        if( redmineError != RedmineError::NO_ERR )
        {
            QString errorMsg = tr("Could not load issue.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        issue_ = issue;

        addRecentIssue( issue );

        qml("issueId")->setProperty( "text", QString("Issue ID: %1").arg(issue.id) );
        qml("issueId")->setProperty( "cursorPosition", 0 );
        qml("subject")->setProperty( "text", issue.subject );
        qml("subject")->setProperty( "cursorPosition", 0 );
        qml("description")->setProperty( "text", issue.description );

        QString more;
        if( issue.tracker.id != NULL_ID )
            more.append( QString("<b>Tracker:</b> %1<br>").arg(issue.tracker.name) );
        if( issue.category.id != NULL_ID )
            more.append( QString("<b>Category:</b> %1<br>").arg(issue.category.name) );
        if( issue.version.id != NULL_ID )
            more.append( QString("<b>Target version:</b> %1<br>").arg(issue.version.name) );
        if( issue.parentId != NULL_ID )
            more.append( QString("<b>Parent issue ID:</b> %1<br>").arg(issue.parentId) );

        QString customFields;
        bool displayCustomFields = false;
        for( const auto& customField : issue.customFields )
        {
            if( !customField.values.size()
                || (customField.values.size() == 1 && customField.values[0].isEmpty()) )
                continue;

            displayCustomFields = true;

            QString value;

            for( const auto& val : customField.values )
            {
                if( !value.isEmpty() )
                    value.append( ", " );

                value.append( val );
            }

            customFields.append( QString("<b>%1:</b> %2<br>").arg(customField.name).arg(value) );
        }
        if( displayCustomFields )
            more.append( customFields );

        if( more.isEmpty() )
        {
            qml("more")->setProperty( "visible", false );
        }
        else
        {
            // Remove the last <br>
            more.chop(4);
            qml("more")->setProperty( "text", more );
            qml("more")->setProperty( "visible", true );
        }

        loadLatestActivity();
        loadIssueStatuses();

        updateTitle();

        if( startTimer )
            start();

        saveSettings();

        CBRETURN();
    },
    issueId );

    RETURN();
}

void
MainWindow::loadIssueStatuses()
{
    ENTER();

    if( !connected() )
        RETURN();

    ++callbackCounter_;
    redmine_->retrieveIssueStatuses( [&]( IssueStatuses issueStatuses, RedmineError redmineError,
                                          QStringList errors )
    {
        CBENTER()(redmineError)(errors);

        if( !connected() )
            CBRETURN();

        if( redmineError != RedmineError::NO_ERR )
        {
            QString errorMsg = tr( "Could not load issue statuses." );
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        int currentIndex = 0;

        issueStatusModel_.clear();
        issueStatusModel_.push_back( SimpleItem(NULL_ID, "Choose issue status") );
        for( const auto& issueStatus : issueStatuses )
        {
            if( issueStatus.id == issue_.status.id )
                currentIndex = issueStatusModel_.rowCount();

            issueStatusModel_.push_back( SimpleItem(issueStatus) );
        }

        DEBUG()(issueStatusModel_)(issue_.status.id)(currentIndex);

        qml("issueStatus")->setProperty( "currentIndex", -1 );
        qml("issueStatus")->setProperty( "currentIndex", currentIndex );

        CBRETURN();
    } );

    RETURN();
}

void
MainWindow::loadLatestActivity()
{
    ENTER();

    if( !connected() )
        RETURN();

    if( issue_.id == NULL_ID )
    {
        loadActivities();
        RETURN();
    }

    ++callbackCounter_;
    redmine_->retrieveTimeEntries( [&]( TimeEntries timeEntries, RedmineError redmineError,
                                        QStringList errors )
    {
        CBENTER()(redmineError)(errors);

        if( !connected() )
            CBRETURN();

        if( redmineError != RedmineError::NO_ERR )
        {
            QString errorMsg = tr( "Could not load time entries." );
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        if( timeEntries.size() == 1 && timeEntries[0].activity.id != NULL_ID )
            activityId_ = timeEntries[0].activity.id;

        loadActivities();

        CBRETURN();
    },
    QString("issue_id=%1&limit=1").arg(issue_.id) );

    RETURN();
}

void
MainWindow::loadOrCreateIssue( CliOptions options )
{
    ENTER()(options);

    QString suffix = QString("create-%1").arg(options.externalId);
    QSystemSemaphore* semaphore = new QSystemSemaphore( getServerName(suffix), 1 );

    if( !semaphore )
        RETURN();

    // If another process is already creating an issue, wait until it has finished
    bool ok;
    ok = semaphore->acquire();

    if( !ok )
        RETURN();

    auto create = [=]( int parentId = NULL_ID )
    {
        ENTER()(parentId);

        Issue issue;
        issue.subject = options.subject;
        issue.project.id = options.projectId;

        if( options.assigneeId != NULL_ID )
            issue.assignedTo.id = options.assigneeId;

        if( parentId != NULL_ID )
            issue.parentId = parentId;

        if( options.trackerId != NULL_ID )
            issue.tracker.id = options.trackerId;

        if( options.versionId != NULL_ID )
            issue.version.id = options.versionId;

        if( !options.description.isEmpty() )
            issue.description = options.description;

        issue.startDate = QDate::currentDate();

        // external ID
        {
            CustomField externalId;
            externalId.id = profileData()->externalIdFieldId;
            externalId.values.push_back( options.externalId );

            issue.customFields.push_back( externalId );
        }

        ++callbackCounter_;
        redmine_->sendIssue( issue, [=](bool success, int id, RedmineError errorCode, QStringList errors)
        {
            CBENTER();

            DEBUG()(issue)(success)(id)(errorCode)(errors);

            if( !success )
            {
                QString errorMsg = tr( "CLI: Could not create issue." );
                for( const auto& error : errors )
                    errorMsg.append("\n").append(error);

                message( errorMsg, QtCriticalMsg );

                semaphore->release();
                delete semaphore;

                CBRETURN();
            }

            message( tr("CLI: New issue created with ID %1").arg(id) );

            loadIssue( id );

            semaphore->release();
            delete semaphore;

            CBRETURN();
        } );

        RETURN();
    };

    auto createAndFindParent = [=]()
    {
        ENTER();

        // Try to load an existing issue first
        if( options.parentId != NULL_ID || !options.externalParentId.isEmpty() )
        {
            if( options.parentId != NULL_ID )
            {
                // Search by issue ID
                ++callbackCounter_;
                redmine_->retrieveIssue( [=]( Issue issue, RedmineError redmineError, QStringList errors )
                {
                    CBENTER()(issue)(redmineError)(errors);

                    // Exactly one issue found, loading it
                    create( issue.id );

                    CBRETURN();
                },
                options.parentId );
            }
            else if( !options.externalParentId.isEmpty() )
            {
                if( profileData()->externalIdFieldId == NULL_ID )
                {
                    message( "Cannot load existing parent issue: No external ID field specified.",
                             QtCriticalMsg );

                    semaphore->release();
                    delete semaphore;

                    CBRETURN();
                }

                // Search by external ID
                RedmineOptions redmineOptions;
                redmineOptions.parameters = QString("cf_%1=%2").arg(profileData()->externalIdFieldId)
                                                               .arg(options.externalParentId);

                ++callbackCounter_;
                redmine_->retrieveIssues( [=]( Issues issues, RedmineError redmineError, QStringList errors )
                {
                    CBENTER()(issues)(redmineError)(errors);

                    if( issues.count() != 1 )
                    {
                        // No exact match found for parent
                        create();

                        CBRETURN();
                    }

                    // Exactly one parent found
                    create( issues[0].id );

                    CBRETURN();
                },
                redmineOptions );
            }
        }
        else
        {
            // If no issue has been found, create a new one
            create();
        }

        RETURN();
    };

    // Try to load an existing issue first
    if( !options.externalId.isEmpty() )
    {
        if( profileData()->externalIdFieldId == NULL_ID )
        {
            message( "Cannot load existing issue: No external ID field specified.", QtCriticalMsg );

            semaphore->release();
            delete semaphore;

            RETURN();
        }

        // Search by external ID
        RedmineOptions redmineOptions;
        redmineOptions.parameters = QString("cf_%1=%2").arg(profileData()->externalIdFieldId)
                                                       .arg(options.externalParentId);

        ++callbackCounter_;
        redmine_->retrieveIssues( [=]( Issues issues, RedmineError redmineError, QStringList errors )
        {
            CBENTER()(issues)(redmineError)(errors);

            if( issues.count() > 1 )
            {
                // Multiple issues found, doing nothing
                semaphore->release();
                delete semaphore;

                CBRETURN();
            }

            if( issues.count() == 0 )
            {
                // No issue found, creating one
                createAndFindParent();

                CBRETURN();
            }

            // Exactly one issue found, loading it
            loadIssue( issues[0].id );

            semaphore->release();
            delete semaphore;

            CBRETURN();
        },
        redmineOptions );
    }
    else
    {
        // Create a new issue
        createAndFindParent();
    }

    RETURN();
}

void
MainWindow::notifyConnectionStatus( QNetworkAccessManager::NetworkAccessibility connected )
{
    ENTER()(connected);

    if( connected == QNetworkAccessManager::Accessible )
    {
        if( !connected_ )
        {
            connected_ = true;
            refreshGui();
        }

        qml("connectionStatus")->setProperty("tooltip", "Connection established" );
        qml("connectionStatusStyle")->setProperty("color", "lightgreen" );

        if( !timer_->isActive() && counterGui() != 0 )
            stop( false );
    }
    else
    {
        connected_ = false;

        qml("connectionStatus")->setProperty("tooltip", "Connection not available" );
        qml("connectionStatusStyle")->setProperty("color", "red" );
    }

    RETURN();
}

void
MainWindow::pauseCounterGui()
{
    ENTER();

    updateCounterGui_ = false;

    // Save the currently displayed time
    QTime time = SimpleRedmineClient::getTime( qml("counter")->property("text").toString() );
    if( time.isValid() )
        counterBeforeEdit_ = time.hour()*3600 + time.minute()*60 + time.second();

    RETURN();
}

void
MainWindow::receiveCommand()
{
    ENTER();

    QLocalSocket* socket = server->nextPendingConnection();

    ++callbackCounter_;
    auto cb = [=]()
    {
        CBENTER();

        CliOptions options = CliOptions::deserialise( socket );

        DEBUG()(options);

        if( options.command == "start" )
            loadIssue( options.issueId );
        else if( options.command == "stop")
            stop();
        else if( options.command == "create" )
            loadOrCreateIssue( options );
        else if( options.command == "issue" )
            options.issueId = issue_.id;

        QByteArray block = CliOptions::serialise( options );

        socket->write( block );
        socket->flush();

        CBRETURN();
    };

    connect( socket, &QLocalSocket::readyRead, cb );

    RETURN();
}

void
MainWindow::resumeCounterGui()
{
    ENTER();

    QTime time = SimpleRedmineClient::getTime( qml("counter")->property("text").toString() );

    if( time.isValid() )
    {
        if( !timer_->isActive() )
            startTimer();

        int secs = time.hour()*3600 + time.minute()*60 + time.second();

        if( secs != counterBeforeEdit_ )
            counterDiff_ = secs - counterNoDiff();
    }
    else
        message( tr("Invalid time format, expecting hh:mm:ss "), QtCriticalMsg );

    updateCounterGui_ = true;

    RETURN();
}

void
MainWindow::reconnect()
{
    ENTER();

    const ProfileData* data = profileData();

    if( data->isValid() && !connected() )
    {
        redmine_->setUrl( data->url );
        redmine_->setAuthenticator( data->apiKey );
        redmine_->reconnect();
    }

    RETURN();
}

void
MainWindow::reconnectAndRefreshGui()
{
    ENTER();

    reconnect();
    refreshGui();

    RETURN();
}

void
MainWindow::refreshGui()
{
    ENTER();

    const ProfileData* data = profileData();

    redmine_->setCheckSsl( !data->ignoreSslErrors );

    shortcutCreateIssue_->setShortcut( QKeySequence(data->shortcutCreateIssue) );
    shortcutSelectIssue_->setShortcut( QKeySequence(data->shortcutSelectIssue) );
    shortcutStartStop_->setShortcut( QKeySequence(data->shortcutStartStop) );
    shortcutToggle_->setShortcut(  QKeySequence(data->shortcutToggle) );

    initTrayIcon();

    if( data->activityId != NULL_ID)
        activityId_ = data->activityId;

    loadIssue( data->issueId, false, true );

    recentIssues_.clear();
    for( const auto& issue : data->recentIssues )
        recentIssues_.push_back( issue );

    loadLatestActivity();
    loadIssueStatuses();

    updateTitle();

    if( !timer_->isActive() && counter() != 0 )
        stop();

    RETURN();
}

void
MainWindow::refreshCounter()
{
    // GUI counter update
    if( updateCounterGui_ )
        qmlCounter_->setProperty( "text", QTime(0, 0, 0).addSecs(counter()).toString("HH:mm:ss") );
}

void
MainWindow::resetGui( const QString value )
{
    ENTER();

    qml("description")->setProperty( "text", value );
    qml("issueId")->setProperty( "text", value );
    qml("more")->setProperty( "text", value );
    qml("subject")->setProperty( "text", value );

    qml("more")->setProperty( "visible", false );

    RETURN();
}

void
MainWindow::saveSettings()
{
    ENTER();

    ProfileData* data = profileData();
    settings_->windowData()->mainWindow = getWindowData();
    data->recentIssues = recentIssues_.data().toVector();

    // If currently there is no issue selected, use the first one from the recently opened issues list
    if( issue_.id == NULL_ID && recentIssues_.rowCount() )
        data->issueId = recentIssues_.at(0).id;
    else
        data->issueId = issue_.id;

    settings_->save();

    RETURN();
}

void
MainWindow::selectIssue()
{
    ENTER();

    issueSelector_->setProjectId( profileData()->projectId );
    issueSelector_->display();

    RETURN();
}

void
MainWindow::start()
{
    ENTER();

    // If no issue is selected, show issue selector
    if( issue_.id == NULL_ID )
    {
        // Make sure that the main window is visible
        display();

        selectIssue();

        RETURN();
    }

    // Afterwards, start the timer
    startTimer();

    RETURN();
}

void
MainWindow::settingsApplied()
{
    ENTER();

    if( !profileData()->isValid() )
    {
        connected_ = false;
        settings_->display();
        RETURN();
    }

    reconnect();
    refreshGui();

    RETURN();
}

void
MainWindow::startStop()
{
    ENTER();

    // If the timer is currently active, stop it; otherwise, start it
    if( timer_->isActive() )
        stop( false );
    else
        start();

    RETURN();
}

void
MainWindow::startTimer()
{
    ENTER();

    timer_->start();

    lastCounterUpdated_ = QDateTime::currentDateTimeUtc();
    lastStarted_ = QDateTime::currentDateTimeUtc();

    // Set the start/stop button icon to stop
    qml("startStop")->setProperty( "iconSource", "qrc:/open-iconic/svg/media-stop.svg" );
    qml("startStop")->setProperty( "tooltip", tr("Stop time tracking") );

    if( trayIcon_ )
        trayIcon_->setIcon( QIcon(ICON_CLOCK_PLAY) );

    // Set the issue status ID to the worked on ID if not already done
    int workedOnId = profileData()->workedOnId;
    if( workedOnId != NULL_ID && workedOnId != issue_.status.id )
        updateIssueStatus( workedOnId );

    RETURN();
}

void
MainWindow::stop( bool resetTimerOnError, bool stopTimerAfterSaving, SuccessCb cb )
{
    ENTER();

    QDateTime cur = QDateTime::currentDateTimeUtc();
    counterDiff_ += lastCounterUpdated_.secsTo( cur );
    lastCounterUpdated_ = cur;

    if( !timer_->isActive() && counterGui() == 0 )
    {
        if( cb )
            cb( true, NULL_ID, (RedmineError)RedmineError::NO_ERR, QStringList() );

        RETURN();
    }

    // Check that an activity has been selected
    if( activityId_ == NULL_ID )
    {
        QStringList errors;
        errors.append( tr("Select an activity before saving the time entry.") );
        message( errors.at(0), QtCriticalMsg );

        if( cb )
            cb( false, NULL_ID, RedmineError::ERR_NOT_SAVED, errors );

        RETURN();
    }

    // Save the tracked time
    TimeEntry timeEntry;
    timeEntry.activity.id = activityId_;
    timeEntry.hours       = counter() / 3600; // Seconds to hours conversion
    timeEntry.issue.id    = issue_.id;
    timeEntry.comment     = qml("entryComment")->property("text").toString();  // Time entry comment

    // Possibly save start and end time as well
    const ProfileData* data = profileData();
    if( data->useCustomFields )
    {
        auto addCustomField = [&timeEntry]( int fieldId, QString stime )
        {
            ENTER();

            if( fieldId == NULL_ID )
                RETURN();

            CustomField cf;
            cf.id = fieldId;
            cf.values.push_back( stime );

            timeEntry.customFields.push_back( cf );

            RETURN();
        };

        QString timeFormat = "yyyy-MM-ddTHH:mm:ss";
        addCustomField( data->startTimeFieldId, lastStarted_.toString(timeFormat) );
        addCustomField( data->endTimeFieldId,
                        QDateTime::currentDateTimeUtc().toString(timeFormat) );
    }

    // Stop the timer for now - might be started again later
    stopTimer();

    ++callbackCounter_;
    redmine_->sendTimeEntry( timeEntry, [=](bool success, int id, RedmineError errorCode, QStringList errors)
    {
        CBENTER()(success)(id)(errorCode)(errors);

        if( !success && errorCode != RedmineError::ERR_TIME_ENTRY_TOO_SHORT )
        {
            QString errorMsg = tr( "Could not save the time entry." );
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);
            message( errorMsg, QtCriticalMsg );

            if( cb )
                cb( success, id, errorCode, errors );

            CBRETURN();
        }

        if( errorCode == RedmineError::ERR_TIME_ENTRY_TOO_SHORT && stopTimerAfterSaving )
            message( tr("Not saving too short time entries."), QtWarningMsg );

        if( !stopTimerAfterSaving )
            startTimer();

        if( success )
            message( tr("Saved time %1").arg(QTime(0, 0, 0).addSecs(counter()).toString("HH:mm:ss")) );

        if( success || (resetTimerOnError && errorCode != RedmineError::ERR_TIME_ENTRY_TOO_SHORT) )
        {
            counterDiff_ = 0;
            qmlCounter_->setProperty( "text", "00:00:00" );
        }

        DEBUG() << "Emitting signal timeEntrySaved()";
        emit timeEntrySaved();

        if( cb )
            cb( true, id, errorCode, errors );

        CBRETURN();
    });

    RETURN();
}

void
MainWindow::stopTimer()
{
    ENTER();

    timer_->stop();

    // Set the start/stop button icon to start
    qml("startStop")->setProperty( "iconSource", "qrc:///open-iconic/svg/media-play.svg" );
    qml("startStop")->setProperty( "tooltip", tr("Start time tracking") );

    if( trayIcon_ )
        trayIcon_->setIcon( QIcon(ICON_CLOCK_STOP) );

    RETURN();
}

void
MainWindow::toggle()
{
    ENTER();

    if( isExposed() )
        hide();
    else
        display();

    RETURN();
}

void
MainWindow::trayEvent( QSystemTrayIcon::ActivationReason reason )
{
    ENTER()(reason);

#ifndef Q_OS_MAC
    if( reason == QSystemTrayIcon::ActivationReason::Trigger )
        toggle();
#endif

    RETURN();
}

QSystemTrayIcon*
MainWindow::trayIcon()
{
    ENTER();
    RETURN( trayIcon_ );
}

void
MainWindow::updateIssueStatus( int statusId )
{
    ENTER();

    if( statusId == NULL_ID || issue_.id == NULL_ID )
        RETURN();

    Issue issue;
    issue.status.id = statusId;

    ++callbackCounter_;
    redmine_->sendIssue( issue, [=](bool success, int id, RedmineError errorCode, QStringList errors)
    {
        CBENTER();

        DEBUG()(success)(id)(errorCode)(errors);

        if( !success )
        {
            QString errorMsg = tr( "Could not update the issue." );
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        message( tr("Issue updated") );

        issue_.status.id = statusId;
        loadIssueStatuses();

        CBRETURN();
    },
    issue_.id );

    RETURN();
}

void
MainWindow::updateTitle()
{
    ENTER();

    QString title = QString("RedTimer - %1").arg(profileData()->name);
    setTitle( title );

    if( trayIcon_ )
    {
        if( issue_.id != NULL_ID )
        {
            title.append( QString("\n\nIssue ID: %1").arg(issue_.id) );
            if( !issue_.subject.isEmpty() )
                title.append( QString("\n%2").arg(issue_.subject) );
        }
        trayIcon_->setToolTip( title );
    }

    RETURN();
}

} // redtimer
