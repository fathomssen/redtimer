#include "IssueCreator.h"
#include "IssueSelector.h"
#include "RedTimer.h"
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

RedTimer::RedTimer( QApplication* parent, bool trayIcon )
    : Window( "qrc:/RedTimer.qml" ),
      app_( parent ),
      useSystemTrayIcon_( trayIcon )
{
    ENTER();

    // Connect to Redmine
    redmine_ = new SimpleRedmineClient( this );

    // Settings initialisation
    settings_ = new Settings( redmine_ );
    settings_->load();

    // Main window initialisation
    installEventFilter( this );
    setResizeMode( QQuickView::SizeRootObjectToView );
    setModality( Qt::ApplicationModal );
    setTitle( "RedTimer" );

    QPoint position = settings_->data.position;
    if( !position.isNull() )
        setPosition( position );

    // Additional window manager properties
    Qt::WindowFlags flags = Qt::Window;
    flags |= Qt::CustomizeWindowHint  | Qt::WindowTitleHint;
    flags |= Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint;
    setFlags( flags );

    display();

    // Notify upon connection status change
    connect( redmine_, &SimpleRedmineClient::connectionChanged, this, &RedTimer::notifyConnectionStatus );

    // Main window access members
    ctx_ = rootContext();
    item_ = qobject_cast<QQuickItem*>( rootObject() );
    qmlCounter_ = qml( "counter" );

    // Additional connection check in case that VirtualBox or similar is installed
    checkConnectionTimer_ = new QTimer( this );
    connect( checkConnectionTimer_, &QTimer::timeout, this, &RedTimer::checkNetworkConnection );
    checkConnectionTimer_->setTimerType( Qt::VeryCoarseTimer );
    checkConnectionTimer_->setInterval( 5000 );

    // Shortcuts
    shortcutCreateIssue_ = new QxtGlobalShortcut( this );
    connect( shortcutCreateIssue_, &QxtGlobalShortcut::activated, this, &RedTimer::createIssue );
    shortcutSelectIssue_ = new QxtGlobalShortcut( this );
    connect( shortcutSelectIssue_, &QxtGlobalShortcut::activated, this, &RedTimer::selectIssue );
    shortcutStartStop_ = new QxtGlobalShortcut( this );
    connect( shortcutStartStop_, &QxtGlobalShortcut::activated, this, &RedTimer::startStop );
    shortcutToggle_ = new QxtGlobalShortcut( this );
    connect( shortcutToggle_, &QxtGlobalShortcut::activated, this, &RedTimer::toggle );

    // Initially connect and update the GUI
    reconnect();

    // Timer initialisation
    timer_ = new QTimer( this );
    timer_->setTimerType( Qt::VeryCoarseTimer );
    timer_->setInterval( 1000 );

    // Apply loaded settings
    activityId_ = settings_->data.activityId;
    loadIssue( settings_->data.issueId, false );

    for( const auto& issue : settings_->data.recentIssues )
        recentIssues_.push_back( issue );
    ctx_->setContextProperty( "recentIssuesModel", &recentIssues_ );

    ctx_->setContextProperty( "activityModel", &activityModel_ );
    ctx_->setContextProperty( "issueStatusModel", &issueStatusModel_ );

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
    connect( qml("redTimer"), SIGNAL(counterAnyKeyPressed()), this, SLOT(pauseCounterGui()) );
    connect( qml("counter"), SIGNAL(editingFinished()), this, SLOT(resumeCounterGui()) );

    // Connect the settings saved signal to the reconnect slot
    connect( settings_, &Settings::applied, this, &RedTimer::reconnect );

    // Connect the timer to the tracking counter
    connect( timer_, &QTimer::timeout, this, &RedTimer::refreshCounter );

    // Initially check the internet connection
    redmine_->checkConnectionStatus();

    RETURN();
}

void
RedTimer::activitySelected( int index )
{
    ENTER();

    activityId_ = activityModel_.at(index).id();
    DEBUG()(index)(activityId_);

    RETURN();
}

void
RedTimer::addRecentIssue( qtredmine::Issue issue )
{
    ENTER()(issue);

    // If found, remove the new issue from the list
    for( int i = 0; i < recentIssues_.rowCount(); ++i )
        if( recentIssues_.at(i).id == issue.id )
            recentIssues_.removeRow( i );

    // Add the issue to the top of the list
    recentIssues_.push_front( issue );

    // Crop the list after ten entries (keep 0..9)
    int numRecentIssues = settings_->data.numRecentIssues;
    if( numRecentIssues != -1 )
        recentIssues_.removeRowsFrom( numRecentIssues );

    // Reset the quickPick text field
    qml("quickPick")->setProperty( "currentIndex", -1 );
    qml("quickPick")->setProperty( "editText", "" );

    RETURN();
}

void
RedTimer::checkNetworkConnection()
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
RedTimer::createIssue()
{
    ENTER();

    // Save time on current issue if timer is running
    if( timer_->isActive() && issue_.id != NULL_ID )
        stop( true, false );

    if( !timer_->isActive() )
        startTimer();

    // Display the issue creator with the current issue as parent
    IssueCreator* issueCreator = new IssueCreator( redmine_ );
    issueCreator->setTransientParent( this );
    issueCreator->setParentIssueId( issue_.id );
    issueCreator->display();

    // Empty the issue information and set ID to NULL_ID
    qml("issueData")->setProperty( "text", "<new issue>" );
    issue_.id = NULL_ID;

    // Connect the issue selected signal to the setIssue slot
    connect( issueCreator, &IssueCreator::cancelled, [=]()
    {
        if( recentIssues_.rowCount() )
            loadIssue( recentIssues_.at(0).id );
        else
            qml("issueData")->setProperty( "text", "" );
    } );

    // Connect the issue selected signal to the setIssue slot
    connect( issueCreator, &IssueCreator::created, [=](int issueId){ loadIssue(issueId, true, true); } );

    RETURN();
}

void
RedTimer::display()
{
    ENTER();

    showNormal();
    requestActivate();
    raise();

    RETURN();
}

bool
RedTimer::eventFilter( QObject* obj, QEvent* event )
{
    // Control closing behaviour depending on tray icon usage
    if( event->type() == QEvent::Close )
    {
        if( trayIcon_ )
            hide();
        else
            exit();

        return true;
    }

    return QObject::eventFilter( obj, event );
}

void
RedTimer::exit()
{
    ENTER();

    // Show warning on close and if timer is running
    if( timer_->isActive() )
    {
        DEBUG() << "Received close event while timer is running";

        QMessageBox msgBox( QMessageBox::Warning, QString("RedTimer"),
                            tr("The timer is currently running"),
                            QMessageBox::NoButton, qobject_cast<QWidget*>(this) );
        msgBox.setInformativeText( tr("Do you want to save the logged time?") );
        msgBox.setWindowModality( Qt::ApplicationModal );
        msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
        msgBox.setDefaultButton( QMessageBox::Cancel );

        int ret = msgBox.exec();

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
            connect( this, &RedTimer::timeEntrySaved, [&]()
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

    settings_->data.position = position();
    settings_->data.recentIssues = recentIssues_.data().toVector();

    // If currently there is no issue selected, use the first one from the recently opened issues list
    if( issue_.id == NULL_ID && recentIssues_.rowCount() )
        settings_->data.issueId = recentIssues_.at(0).id;
    else
        settings_->data.issueId = issue_.id;

    settings_->save();

    if( trayIcon_ )
        trayIcon_->hide();

    app_->quit();

    RETURN();
}

void
RedTimer::initTrayIcon()
{
    ENTER();

    // Create tray icon if desired and not yet available
    if( !trayIcon_ && useSystemTrayIcon_ && settings_->data.useSystemTrayIcon
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
        connect( trayIcon_, &QSystemTrayIcon::activated, this, &RedTimer::trayEvent );
    }

    // Hide tray icon if desired and currently shown
    if( trayIcon_ && (!useSystemTrayIcon_ || !settings_->data.useSystemTrayIcon) )
    {
        trayIcon_->hide();
        delete trayIcon_;
        trayIcon_ = nullptr;
    }

    RETURN();
}

void
RedTimer::issueStatusSelected( int index )
{
    ENTER();

    issue_.status.id = issueStatusModel_.at(index).id();
    DEBUG()(index)(issue_.status.id);

    updateIssueStatus( issue_.status.id );

    RETURN();
}

void
RedTimer::loadActivities()
{
    ENTER();

    redmine_->retrieveTimeEntryActivities( [&]( Enumerations activities, RedmineError redmineError,
                                                QStringList errors )
    {
        ENTER();

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load activities.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            RETURN();
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

        RETURN();
    } );

    RETURN();
}

void
RedTimer::loadIssueFromList( int index )
{
    ENTER()(index);

    loadIssue( recentIssues_.at(index).id );

    RETURN();
}

void
RedTimer::loadIssueFromTextField()
{
    ENTER();

    if( !qml("quickPick")->property("editText").toInt() )
        RETURN();

    int issueId = qml("quickPick")->property("editText").toInt();

    loadIssue( issueId );

    RETURN();
}

void
RedTimer::loadIssue( int issueId, bool startTimer, bool saveNewIssue )
{
    ENTER()(issueId)(startTimer);

    if( issueId == NULL_ID )
        RETURN();

    if( saveNewIssue )
        issue_.id = issueId;

    // If the timer is currently active, save the currently logged time first
    if( timer_->isActive() )
        stop( true, false );

    redmine_->retrieveIssue( [=]( Issue issue, RedmineError redmineError, QStringList errors )
    {
        ENTER()(issue);

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr("Could not load issue.");
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            RETURN();
        }

        issue_ = issue;

        addRecentIssue( issue );

        QString issueData = QString( "Issue #%1\n\nSubject: %2\n\n%3" )
                .arg(issue.id)
                .arg(issue.subject)
                .arg(issue.description);
        qml("issueData")->setProperty( "text", issueData );

        loadLatestActivity();
        loadIssueStatuses();

        if( startTimer )
            start();

        RETURN();
    },
    issueId );

    RETURN();
}

void
RedTimer::loadIssueStatuses()
{
    ENTER();

    redmine_->retrieveIssueStatuses( [&]( IssueStatuses issueStatuses, RedmineError redmineError,
                                          QStringList errors )
    {
        ENTER();

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr( "Could not load issue statuses." );
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            RETURN();
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

        RETURN();
    } );

    RETURN();
}

void
RedTimer::loadLatestActivity()
{
    ENTER();

    if( issue_.id == NULL_ID )
    {
        loadActivities();
        RETURN();
    }

    redmine_->retrieveTimeEntries( [&]( TimeEntries timeEntries, RedmineError redmineError,
                                        QStringList errors )
    {
        ENTER();

        if( redmineError != NO_ERROR )
        {
            QString errorMsg = tr( "Could not load time entries." );
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            RETURN();
        }

        if( timeEntries.size() == 1 )
            activityId_ = timeEntries[0].activity.id;

        loadActivities();

        RETURN();
    },
    QString("issue_id=%1&limit=1").arg(issue_.id) );

    RETURN();
}

void
RedTimer::notifyConnectionStatus( QNetworkAccessManager::NetworkAccessibility connected )
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
RedTimer::pauseCounterGui()
{
    ENTER();
    updateCounterGui_ = false;
    RETURN();
}

void
RedTimer::resumeCounterGui()
{
    ENTER();

    QString stime = qml("counter")->property( "text" ).toString();
    QTime time = QTime::fromString( stime );

    if( time.isValid() )
        counter_ = time.hour()*3600 + time.minute()*60 + time.second();
    else
        message( tr("Invalid time specified: ").append(stime), QtCriticalMsg );

    updateCounterGui_ = true;

    RETURN();
}

void
RedTimer::reconnect()
{
    ENTER();

    redmine_->setUrl( settings_->data.url );
    redmine_->setAuthenticator( settings_->data.apiKey );

    refreshGui();

    if( timer_ && !timer_->isActive() && counter_ )
        stop();

    RETURN();
}

void
RedTimer::refreshGui()
{
    ENTER();

    if( settings_->data.ignoreSslErrors )
        redmine_->setCheckSsl( false );

    if( settings_->data.checkConnection )
        checkConnectionTimer_->start();
    else
        checkConnectionTimer_->stop();

    shortcutCreateIssue_->setShortcut( QKeySequence(settings_->data.shortcutCreateIssue) );
    shortcutSelectIssue_->setShortcut( QKeySequence(settings_->data.shortcutSelectIssue) );
    shortcutStartStop_->setShortcut( QKeySequence(settings_->data.shortcutStartStop) );
    shortcutToggle_->setShortcut(  QKeySequence(settings_->data.shortcutToggle) );

    initTrayIcon();

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
RedTimer::refreshCounter()
{
    ++counter_;

    if( updateCounterGui_ )
        qmlCounter_->setProperty( "text", QTime(0, 0, 0).addSecs(counter_).toString("HH:mm:ss") );
}

void
RedTimer::selectIssue()
{
    // Issue selector initialisation
    IssueSelector* issueSelector = new IssueSelector( redmine_ );
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
RedTimer::start()
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
RedTimer::startStop()
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
RedTimer::startTimer()
{
    ENTER();

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
RedTimer::stop( bool resetTimerOnError, bool stopTimerAfterSaving )
{
    ENTER();

    // Check that an activity has been selected
    if( activityId_ == NULL_ID )
    {
        message( tr("Please select an activity before saving the time entry."), QtCriticalMsg );
        RETURN();
    }

    // Save the tracked time
    TimeEntry timeEntry;
    timeEntry.activity.id = activityId_;
    timeEntry.hours       = (double)counter_ / 3600; // Seconds to hours conversion
    timeEntry.issue.id    = issue_.id;

    // Stop the timer for now - might be started again later
    stopTimer();

    redmine_->sendTimeEntry( timeEntry, [=](bool success, int id, RedmineError errorCode, QStringList errors)
    {
        ENTER()(success)(id)(errorCode)(errors);

        if( !success && errorCode != ERR_TIME_ENTRY_TOO_SHORT )
        {
            QString errorMsg = tr( "Could not save the time entry." );
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            RETURN();
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

        RETURN();
    });

    RETURN();
}

void
RedTimer::stopTimer()
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
RedTimer::toggle()
{
    ENTER();

    if( isVisible() )
        hide();
    else
        display();

    RETURN();
}

void
RedTimer::trayEvent( QSystemTrayIcon::ActivationReason reason )
{
    ENTER()(reason);

    if( reason == QSystemTrayIcon::ActivationReason::Trigger )
        toggle();

    RETURN();
}

void
RedTimer::updateIssueStatus( int statusId )
{
    ENTER();

    if( statusId == NULL_ID )
        RETURN();

    Issue issue;
    issue.status.id = statusId;

    redmine_->sendIssue( issue, [=](bool success, int id, RedmineError errorCode, QStringList errors)
    {
        ENTER()(success)(id)(errorCode)(errors);

        if( !success )
        {
            QString errorMsg = tr( "Could not update the issue." );
            for( const auto& error : errors )
                errorMsg.append("\n").append(error);

            message( errorMsg, QtCriticalMsg );
            RETURN();
        }

        message( tr("Issue updated") );

        issue_.status.id = statusId;
        loadIssueStatuses();

        RETURN();
    },
    issue_.id );

    RETURN();
}
