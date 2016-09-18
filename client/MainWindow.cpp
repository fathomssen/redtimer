#include "IssueCreator.h"
#include "IssueSelector.h"
#include "MainWindow.h"
#include "Settings.h"
#include "logging.h"

#include <QEventLoop>
#include <QMessageBox>
#include <QMenu>
#include <QNetworkInterface>
#include <QObject>
#include <QTime>

using namespace qtredmine;
using namespace redtimer;
using namespace std;

MainWindow::MainWindow( QApplication* parent )
    : Window( "MainWindow", this ),
      app_( parent )
{
    ENTER();

    // Connect to Redmine
    redmine_ = new SimpleRedmineClient( this );

    // Settings initialisation
    settings_ = new Settings( this );
    settings_->load();

    if( settings_->data.apiKey.isEmpty() || settings_->data.url.isEmpty() )
        settings_->display( false );

    // Main window initialisation
    installEventFilter( this );
    setTitle( "RedTimer" );

    QPoint position = settings_->data.position;
    if( !position.isNull() )
        setPosition( position );

    display();

    // Notify upon connection status change
    connect( redmine_, &SimpleRedmineClient::connectionChanged, this, &MainWindow::notifyConnectionStatus );

    // Main window access members
    ctx_ = rootContext();
    item_ = qobject_cast<QQuickItem*>( rootObject() );
    qmlCounter_ = qml( "counter" );

    // Additional connection check in case that VirtualBox or similar is installed
    checkConnectionTimer_ = new QTimer( this );
    connect( checkConnectionTimer_, &QTimer::timeout, this, &MainWindow::checkNetworkConnection );
    checkConnectionTimer_->setTimerType( Qt::VeryCoarseTimer );
    checkConnectionTimer_->setInterval( 5000 );

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

    // Initially connect and update the GUI
    reconnect();

    ctx_->setContextProperty( "activityModel", &activityModel_ );
    ctx_->setContextProperty( "issueStatusModel", &issueStatusModel_ );
    ctx_->setContextProperty( "recentIssuesModel", &recentIssues_ );

    // Set transient window parent
    settings_->setTransientParent( this );

    // Connect the create issue button
    connect( qml("createIssue"), SIGNAL(clicked()), this, SLOT(createIssue()) );

    // Connect the settings button
    connect( qml("settings"), SIGNAL(clicked()), settings_, SLOT(display()) );

    // Connect the reload button
    connect( qml("reload"), SIGNAL(clicked()), this, SLOT(reconnect()) );

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

    // Connect the settings saved signal to the reconnect slot
    connect( settings_, &Settings::applied, this, &MainWindow::reconnect );

    // Connect the timer to the tracking counter
    connect( timer_, &QTimer::timeout, this, &MainWindow::refreshCounter );

    // Initially check the internet connection
    redmine_->checkConnectionStatus();

    RETURN();
}

void
MainWindow::activitySelected( int index )
{
    ENTER();

    activityId_ = activityModel_.at(index).id();
    DEBUG()(index)(activityId_);

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
    int numRecentIssues = settings_->data.numRecentIssues;
    if( numRecentIssues != -1 )
        recentIssues_.removeRowsFrom( numRecentIssues );

    // Reset the quickPick text field
    qml("quickPick")->setProperty( "currentIndex", -1 );
    qml("quickPick")->setProperty( "editText", "" );

    RETURN();
}

void
MainWindow::checkNetworkConnection()
{
    ENTER();

    QNetworkAccessManager::NetworkAccessibility accessible = QNetworkAccessManager::NotAccessible;

    for( const auto& ifc : QNetworkInterface::allInterfaces() )
    {
        // Check whether interface is up, running and no loopback
        if(    !ifc.isValid()
            || !ifc.flags().testFlag(QNetworkInterface::IsUp)
            || !ifc.flags().testFlag(QNetworkInterface::IsRunning)
            || ifc.flags().testFlag(QNetworkInterface::IsLoopBack) )
            continue;

        // Ignore virtual interfaces
        if(    ifc.humanReadableName().startsWith("VirtualBox")
            || ifc.humanReadableName().startsWith("vbox")
            || ifc.humanReadableName().startsWith("VMware") )
            continue;

        for( const auto& addr : ifc.addressEntries() )
        {
            const auto ip = addr.ip();

            // Not a valid IPv4 or IPv6 address
            if( ip.isNull() || ip.isLoopback() || ip == QHostAddress::Any )
                continue;

            DEBUG()(ip.toString());

            accessible = QNetworkAccessManager::Accessible;
            break;
        }

        if( accessible == QNetworkAccessManager::Accessible )
        {
            DEBUG()(ifc.humanReadableName());
            break;
        }
    }

    redmine_->checkConnectionStatus( accessible );

    RETURN();
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

    // Display the issue creator with the current issue as parent
    IssueCreator* issueCreator = new IssueCreator( redmine_, this );
    issueCreator->setTransientParent( this );
    issueCreator->setParentIssueId( issue_.id );
    issueCreator->setProjectId( settings_->data.projectId );
    issueCreator->setUseCustomFields( settings_->data.useCustomFields );
    issueCreator->display();

    // Empty the issue information and set ID to NULL_ID
    qml("description")->setProperty( "text", "<new issue>" );
    issue_.id = NULL_ID;

    // Connect the issue selected signal to the setIssue slot
    connect( issueCreator, &IssueCreator::cancelled, [=]()
    {
        if( recentIssues_.rowCount() )
            loadIssue( recentIssues_.at(0).id );
        else
            qml("description")->setProperty( "text", "" );
    } );

    // Connect the issue selected signal to the setIssue slot
    connect( issueCreator, &IssueCreator::created, [=]( int issueId )
    {
        loadIssue( issueId, true, true );
        settings_->data.projectId = issueCreator->getProjectId();
    } );

    RETURN();
}

void
MainWindow::display()
{
    ENTER();

    raise();
    requestActivate();
    showNormal();

    RETURN();
}

void
MainWindow::hide()
{
    ENTER();

#ifdef Q_OS_OSX
    showMinimized();
#else
    Window::hide();
#endif

    RETURN();
}

bool
MainWindow::eventFilter( QObject* obj, QEvent* event )
{
    // Control closing behaviour depending on tray icon usage
    if( event->type() == QEvent::Close )
    {
        DEBUG("Received close signal");
        if( trayIcon_ )
            hide();
        else
            exit();

        event->ignore();
        return true;
    }

    return QObject::eventFilter( obj, event );
}

void
MainWindow::exit()
{
    ENTER();

    // Show warning on close and if timer is running
    if( timer_->isActive() )
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

    // Save settings
    saveSettings();

    if( trayIcon_ )
        trayIcon_->hide();

    app_->quit();

    RETURN();
}

void
MainWindow::initTrayIcon()
{
    ENTER();

    // Create tray icon if desired and not yet available
    if( !trayIcon_ && settings_->data.useSystemTrayIcon
            && QSystemTrayIcon::isSystemTrayAvailable() )
    {
        trayIcon_ = new QSystemTrayIcon( this );
        trayIcon_->setIcon( QIcon(":/icons/clock_red_stop.svg") );
        trayIcon_->show();

        QMenu* trayMenu = new QMenu( "RedTimer", qobject_cast<QWidget*>(this) );
        trayMenu->addAction( QIcon(":/icons/clock_red.svg"), tr("S&how/hide"), this, SLOT(toggle()) );
        trayMenu->addAction( QIcon(":/open-iconic/svg/media-play.svg"), tr("S&tart/stop"),
                             this, SLOT(startStop()) );
        trayMenu->addAction( QIcon(":/open-iconic/svg/x.svg"), tr("E&xit"), this, SLOT(exit()) );
        trayIcon_->setContextMenu( trayMenu );

        // Connect the tray icon to the window show slot
        connect( trayIcon_, &QSystemTrayIcon::activated, this, &MainWindow::trayEvent );
    }

    // Hide tray icon if desired and currently shown
    if( trayIcon_ && !settings_->data.useSystemTrayIcon )
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

    ++callbackCounter_;
    redmine_->retrieveTimeEntryActivities( [&]( Enumerations activities, RedmineError redmineError,
                                                QStringList errors )
    {
        CBENTER();

        if( redmineError != NO_ERROR )
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

    if( !qml("quickPick")->property("editText").toInt() )
        RETURN();

    int issueId = qml("quickPick")->property("editText").toInt();

    loadIssue( issueId );

    RETURN();
}

void
MainWindow::loadIssue( int issueId, bool startTimer, bool saveNewIssue )
{
    ENTER()(issueId)(startTimer)(saveNewIssue);

    // If the timer is currently active, save the currently logged time first
    // If there will be no new issue selected, stop the timer
    if( timer_->isActive() )
        stop( true, issueId == NULL_ID );

    if( saveNewIssue )
        issue_.id = issueId;

    if( issueId == NULL_ID )
    {
        qml("issueId")->setProperty( "text", "" );
        qml("subject")->setProperty( "text", "" );
        qml("description")->setProperty( "text", "" );
        RETURN();
    }

    ++callbackCounter_;
    redmine_->retrieveIssue( [=]( Issue issue, RedmineError redmineError, QStringList errors )
    {
        CBENTER();

        DEBUG()(issue);

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load issue.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        issue_ = issue;

        addRecentIssue( issue );

        qml("issueId")->setProperty( "text", QString("Issue #%1").arg(issue.id) );
        qml("subject")->setProperty( "text", issue.subject );
        qml("description")->setProperty( "text", issue.description );

        loadLatestActivity();
        loadIssueStatuses();

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

    ++callbackCounter_;
    redmine_->retrieveIssueStatuses( [&]( IssueStatuses issueStatuses, RedmineError redmineError,
                                          QStringList errors )
    {
        CBENTER();

        if( redmineError != NO_ERROR )
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

    if( issue_.id == NULL_ID )
    {
        loadActivities();
        RETURN();
    }

    ++callbackCounter_;
    redmine_->retrieveTimeEntries( [&]( TimeEntries timeEntries, RedmineError redmineError,
                                        QStringList errors )
    {
        CBENTER();

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr( "Could not load time entries." );
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            CBRETURN();
        }

        if( timeEntries.size() == 1 )
            activityId_ = timeEntries[0].activity.id;

        loadActivities();

        CBRETURN();
    },
    QString("issue_id=%1&limit=1").arg(issue_.id) );

    RETURN();
}

void
MainWindow::notifyConnectionStatus( QNetworkAccessManager::NetworkAccessibility connected )
{
    ENTER();

    if( connectionError_ )
    {
        connectionError_->deleteLater();
        connectionError_ = nullptr;
    }

    if( connected == QNetworkAccessManager::Accessible )
        message( tr("Connection to Redmine established") );
    else if( connected == QNetworkAccessManager::NotAccessible )
        connectionError_ = message( tr("Connection to Redmine currently not available"), QtCriticalMsg );

    RETURN();
}

void
MainWindow::pauseCounterGui()
{
    ENTER();
    updateCounterGui_ = false;
    RETURN();
}

void
MainWindow::resumeCounterGui()
{
    ENTER();

    QString stime = qml("counter")->property( "text" ).toString();
    // Try to find valid time string format
    QTime time = QTime::fromString( stime, "hh:mm:ss" );
    if( !time.isValid() )
        time = QTime::fromString( stime, "hh:mm:s" );
    if( !time.isValid() )
        time = QTime::fromString( stime, "hh:m:ss" );
    if( !time.isValid() )
        time = QTime::fromString( stime, "hh:m:s" );
    if( !time.isValid() )
        time = QTime::fromString( stime, "h:mm:s" );
    if( !time.isValid() )
        time = QTime::fromString( stime, "h:m:ss" );
    if( !time.isValid() )
        time = QTime::fromString( stime, "h:m:s" );
    if( !time.isValid() )
        time = QTime::fromString( stime, "hh:mm" );
    if( !time.isValid() )
        time = QTime::fromString( stime, "hh:m" );
    if( !time.isValid() )
        time = QTime::fromString( stime, "h:mm" );
    if( !time.isValid() )
        time = QTime::fromString( stime, "h:m" );
    if( !time.isValid() )
        time = QTime::fromString( stime, "hh" );
    if( !time.isValid() )
        time = QTime::fromString( stime, "h" );

    if( time.isValid() )
        counter_ = time.hour()*3600 + time.minute()*60 + time.second();
    else
        message( tr("Invalid time format, expecting hh:mm:ss "), QtCriticalMsg );

    updateCounterGui_ = true;

    RETURN();
}

void
MainWindow::reconnect()
{
    ENTER();

    redmine_->setUrl( settings_->data.url );
    redmine_->setAuthenticator( settings_->data.apiKey );

    refreshGui();

    if( !timer_->isActive() && counter_ )
        stop();

    RETURN();
}

void
MainWindow::refreshGui()
{
    ENTER();

    if( settings_->data.ignoreSslErrors )
        redmine_->setCheckSsl( false );
    else
        redmine_->setCheckSsl( true );

    if( settings_->data.checkConnection )
        checkConnectionTimer_->start();
    else
        checkConnectionTimer_->stop();

    shortcutCreateIssue_->setShortcut( QKeySequence(settings_->data.shortcutCreateIssue) );
    shortcutSelectIssue_->setShortcut( QKeySequence(settings_->data.shortcutSelectIssue) );
    shortcutStartStop_->setShortcut( QKeySequence(settings_->data.shortcutStartStop) );
    shortcutToggle_->setShortcut(  QKeySequence(settings_->data.shortcutToggle) );

    initTrayIcon();

    activityId_ = settings_->data.activityId;
    loadIssue( settings_->data.issueId, false, true);

    recentIssues_.clear();
    for( const auto& issue : settings_->data.recentIssues )
        recentIssues_.push_back( issue );

    loadLatestActivity();
    loadIssueStatuses();

    QString title = "RedTimer";
    QString url = redmine_->getUrl();

    if( !url.isEmpty() )
        title.append(" - ").append( url );

    setTitle( title );
    if( trayIcon_ )
        trayIcon_->setToolTip( title );

    RETURN();
}

void
MainWindow::refreshCounter()
{
    // Internal counter update
    ++counter_;

    // GUI counter update
    if( updateCounterGui_ )
        qmlCounter_->setProperty( "text", QTime(0, 0, 0).addSecs(counter_).toString("HH:mm:ss") );

    // Time difference check
    int minDiff = 60; // minimum difference in seconds
    QDateTime curTime = QDateTime::currentDateTimeUtc();

    int diff = lastTime_.secsTo(curTime);
    lastTime_ = curTime;

    if( diff > minDiff )
    {
        int ret = QMessageBox::question( qobject_cast<QWidget*>(this), tr("RedTimer"),
                  tr("The timer was paused the last %n minute(s).\n"
                     "Do you want to add this time to the currently tracked time?",
                     "", diff/60) );

        if( ret == QMessageBox::Yes )
            counter_ += diff;
    }
}

void
MainWindow::saveSettings()
{
    ENTER();

    settings_->data.position = position();
    settings_->data.recentIssues = recentIssues_.data().toVector();

    // If currently there is no issue selected, use the first one from the recently opened issues list
    if( issue_.id == NULL_ID && recentIssues_.rowCount() )
        settings_->data.issueId = recentIssues_.at(0).id;
    else
        settings_->data.issueId = issue_.id;

    settings_->save();

    RETURN();
}

void
MainWindow::selectIssue()
{
    // Issue selector initialisation
    IssueSelector* issueSelector = new IssueSelector( redmine_, this );
    issueSelector->setTransientParent( this );
    issueSelector->setProjectId( settings_->data.projectId );
    issueSelector->display();

    // Connect the issue selected signal to the setIssue slot
    connect( issueSelector, &IssueSelector::selected, [=](int issueId)
    {
        settings_->data.projectId = issueSelector->getProjectId();
        loadIssue( issueId );
    } );
}

void
MainWindow::start()
{
    ENTER();

    // If no issue is selected, show issue selector
    if( issue_.id == NULL_ID )
    {
        selectIssue();
        RETURN();
    }

    // Afterwards, start the timer
    startTimer();

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

    lastTime_ = QDateTime::currentDateTimeUtc();

    timer_->start();

    // Set the start/stop button icon to stop
    qml("startStop")->setProperty( "iconSource", "qrc:///open-iconic/svg/media-stop.svg" );
    qml("startStop")->setProperty( "tooltip", tr("Stop time tracking") );

    if( trayIcon_ )
        trayIcon_->setIcon( QIcon(":/icons/clock_red_play.svg") );

    // Set the issue status ID to the worked on ID if not already done
    int workedOnId = settings_->data.workedOnId;
    if( workedOnId != NULL_ID && workedOnId != issue_.status.id )
        updateIssueStatus( workedOnId );

    RETURN();
}

void
MainWindow::stop( bool resetTimerOnError, bool stopTimerAfterSaving, SuccessCb cb )
{
    ENTER();

    if( !timer_->isActive() && counter_ == 0 )
    {
        if( cb )
            cb( true, NULL_ID, (RedmineError)0, QStringList() );

        RETURN();
    }

    // Check that an activity has been selected
    if( activityId_ == NULL_ID )
    {
        QStringList errors;
        errors.append( tr("Select an activity before saving the time entry.") );
        message( errors.at(0), QtCriticalMsg );

        if( cb )
            cb( false, NULL_ID, ERR_NOT_SAVED, errors );

        RETURN();
    }

    // Save the tracked time
    TimeEntry timeEntry;
    timeEntry.activity.id = activityId_;
    timeEntry.hours       = (double)counter_ / 3600; // Seconds to hours conversion
    timeEntry.issue.id    = issue_.id;

    // Stop the timer for now - might be started again later
    stopTimer();

    ++callbackCounter_;
    redmine_->sendTimeEntry( timeEntry, [=](bool success, int id, RedmineError errorCode, QStringList errors)
    {
        CBENTER();

        DEBUG()(success)(id)(errorCode)(errors);

        if( !success && errorCode != ERR_TIME_ENTRY_TOO_SHORT )
        {
            QString errorMsg = tr( "Could not save the time entry." );
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);
            message( errorMsg, QtCriticalMsg );

            if( cb )
                cb( success, id, errorCode, errors );

            CBRETURN();
        }

        if( errorCode == ERR_TIME_ENTRY_TOO_SHORT )
            message( tr("Not saving time entries shorter than one minute."), QtWarningMsg );

        if( !stopTimerAfterSaving )
            startTimer();

        if( success )
            message( tr("Saved time %1").arg(QTime(0, 0, 0).addSecs(counter_).toString("HH:mm:ss")) );

        if( success || (resetTimerOnError && errorCode != ERR_TIME_ENTRY_TOO_SHORT) )
        {
            counter_ = 0;
            qmlCounter_->setProperty( "text", "00:00:00" );
        }

        DEBUG() << "Emitting signal timeEntrySaved()";
        timeEntrySaved();

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
        trayIcon_->setIcon( QIcon(":/icons/clock_red_stop.svg") );

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

#ifndef Q_OS_OSX
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
