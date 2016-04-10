#include "RedTimer.h"
#include "logging.h"

#include <QMessageBox>
#include <QMenu>
#include <QObject>

using namespace qtredmine;
using namespace redtimer;
using namespace std;

RedTimer::RedTimer( QApplication* parent, bool trayIcon )
    : QObject( parent ),
      app_( parent ),
      showTrayIcon_( trayIcon )
{
    ENTER();
    init();
    RETURN();
}

RedTimer::~RedTimer()
{
    ENTER();

    // Save settings
    settings_->setActivity( activityId_ );
    settings_->setIssue( issue_.id );
    settings_->setProject( issueSelector_->getProjectId() );
    settings_->setPosition( win_->position() );
    settings_->setRecentIssues( recentIssues_.data().toVector() );

    settings_->save();

    if( trayIcon_ )
        trayIcon_->hide();

    RETURN();
}

void
RedTimer::init()
{
    ENTER();

    // Connect to Redmine
    redmine_ = new SimpleRedmineClient( this );

    // Settings initialisation
    settings_ = new Settings( redmine_, this );
    settings_->load();

    // Main window initialisation
    win_ = new QQuickView();
    win_->installEventFilter( this );
    win_->setResizeMode( QQuickView::SizeRootObjectToView );
    win_->setSource( QUrl(QStringLiteral("qrc:/RedTimer.qml")) );
    win_->setModality( Qt::ApplicationModal );

    QPoint position = settings_->getPosition();
    if( !position.isNull() )
        win_->setPosition( position );

    // Additional window manager properties
    Qt::WindowFlags flags = Qt::Window;
    flags |= Qt::CustomizeWindowHint  | Qt::WindowTitleHint;
    flags |= Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint;
    win_->setFlags( flags );

    initTrayIcon();

    display();

    // Main window access members
    ctx_ = win_->rootContext();
    item_ = qobject_cast<QQuickItem*>( win_->rootObject() );
    qmlCounter_ = qml( "counter" );

    reconnect();

    // Issue selector initialisation
    issueSelector_ = new IssueSelector( redmine_, this );
    issueSelector_->setProjectId( settings_->getProject() );

    // Timer initialisation
    timer_ = new QTimer( this );
    timer_->setTimerType( Qt::VeryCoarseTimer );
    timer_->setInterval( 1000 );

    // Apply loaded settings
    activityId_ = settings_->getActivity();
    issueSelector_->setProjectId( settings_->getProject() );
    loadIssue( settings_->getIssue(), false );

    for( const auto& issue : settings_->getRecentIssues() )
        recentIssues_.push_back( issue );
    ctx_->setContextProperty( "recentIssuesModel", &recentIssues_ );

    // Set transient window parent
    settings_->window()->setTransientParent( win_ );
    issueSelector_->window()->setTransientParent( win_ );

    // Connect the settings button
    connect( qml("settings"), SIGNAL(clicked()),
             settings_, SLOT(display()) );

    // Connect the reload button
    connect( qml("reload"), SIGNAL(clicked()),
             this, SLOT(reconnect()) );

    // Connect the issue selector button
    connect( qml("selectIssue"), SIGNAL(clicked()),
             issueSelector_, SLOT(display()) );

    // Connect the combobox text field
    connect( qml("quickPick"), SIGNAL(accepted()),
             this, SLOT(loadIssueFromTextField()) );

    // Connect the combobox list
    connect( qml("quickPick"), SIGNAL(activated(int)),
             this, SLOT(loadIssueFromList(int)) );

    // Connect the start/stop button
    connect( qml("startStop"), SIGNAL(clicked()),
             this, SLOT(startStop()) );

    // Connect the activity selected signal to the activitySelected slot
    connect( qml("activity"), SIGNAL(activated(int)),
             this, SLOT(activitySelected(int)) );

    // Connect the issueStatus selected signal to the issueStatusSelected slot
    connect( qml("issueStatus"), SIGNAL(activated(int)),
             this, SLOT(issueStatusSelected(int)) );

    // Connect the settings saved signal to the reconnect slot
    connect( settings_, &Settings::applied, this, &RedTimer::reconnect );

    // Connect the issue selected signal to the setIssue slot
    connect( issueSelector_, &IssueSelector::selected,
             [=](int issueId){ loadIssue(issueId); } );

    // Connect the timer to the tracking counter
    connect( timer_, &QTimer::timeout, this, &RedTimer::refreshCounter );

    // Initially update the GUI
    refresh();

    RETURN();
}

void
RedTimer::initTrayIcon()
{
    ENTER();

    if( showTrayIcon_ && QSystemTrayIcon::isSystemTrayAvailable() )
    {
        trayIcon_ = new QSystemTrayIcon( win_ );
        trayIcon_->setIcon( QIcon(":/icons/clock_red.svg") );
        trayIcon_->show();

        QMenu* trayMenu = new QMenu( "RedTimer", qobject_cast<QWidget*>(win_) );
        trayMenu->addAction( QIcon(":/icons/clock_red.svg"), tr("S&how/hide"), this, &RedTimer::toggle );
        trayMenu->addAction( QIcon(":/open-iconic/svg/x.svg"), tr("E&xit"), this, &RedTimer::exit );
        trayIcon_->setContextMenu( trayMenu );

        // Connect the tray icon to the window show slot
        connect( trayIcon_, &QSystemTrayIcon::activated, this, &RedTimer::trayEvent );
    }

    RETURN();
}

bool
RedTimer::eventFilter( QObject* obj, QEvent* event )
{
    // Show warning on close and if timer is running
    if( event->type() == QEvent::Close )
    {
        if( trayIcon_ )
            win_->hide();
        else
            exit();

        return true;
    }

    return QObject::eventFilter( obj, event );
}

void
RedTimer::trayEvent( QSystemTrayIcon::ActivationReason reason )
{
    ENTER()(reason);

    if( reason == QSystemTrayIcon::ActivationReason::DoubleClick )
        toggle();

    RETURN();
}

void
RedTimer::display()
{
    ENTER();

    win_->showNormal();

    RETURN();
}

void
RedTimer::toggle()
{
    ENTER();

    if( win_->isVisible() )
        win_->hide();
    else
        display();

    RETURN();
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
                            QMessageBox::NoButton, qobject_cast<QWidget*>(win_) );
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
            connect( this, &RedTimer::timeEntrySaved, [=](){ app_->quit(); } );
            stop();
            return;
        }

        default:
            DEBUG() << "Closing the application";
            break;
        }
    }

    app_->quit();

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
    int numRecentIssues = settings_->getNumRecentIssues();
    if( numRecentIssues != -1 )
        recentIssues_.removeRowsFrom( numRecentIssues );

    // Reset the quickPick text field
    qml("quickPick")->setProperty( "currentIndex", -1 );
    qml("quickPick")->setProperty( "editText", "" );

    RETURN();
}


void
RedTimer::updateIssueStatus( int statusId )
{
    ENTER();

    Issue issue;
    issue.status.id = statusId;

    redmine_->sendIssue( issue,
                         [=](bool success, RedmineError redmineError,
                             QNetworkReply::NetworkError networkError)
    {
        ENTER();

        if( !success )
        {
            message( tr("Could not save the time entry. Please check your internet connection."),
                     QtCriticalMsg );
            RETURN();
        }

        message( tr("Issue status updated") );

        issue_.status.id = statusId;
        refreshIssueStatuses();

        RETURN();
    },
    issue_.id );

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
RedTimer::loadIssue( int issueId, bool startTimer )
{
    ENTER()(issueId)(startTimer);

    // If the timer is currently active, save the currently logged time first
    if( timer_->isActive() )
        stop( true, false );

    redmine_->retrieveIssue( [=]( Issue issue )
    {
        ENTER()(issue);

        issue_ = issue;

        addRecentIssue( issue );

        QString issueData = QString( "Issue #%1\n\nSubject: %2\n\n%3" )
                .arg(issue.id)
                .arg(issue.subject)
                .arg(issue.description);
        qml("issueData")->setProperty( "text", issueData );

        refreshIssueStatuses();

        if( startTimer )
            start();

        RETURN();
    },
    issueId );

    RETURN();
}

void
RedTimer::message( QString text, QtMsgType type, int timeout )
{
    ENTER()(text)(type)(timeout);

    QString colour;

    switch( type )
    {
    case QtInfoMsg:
        colour = "#006400";
        break;
    case QtWarningMsg:
        colour = "#FF8C00";
        break;
    case QtCriticalMsg:
        colour = "#8B0000";
        break;
    case QtDebugMsg:
    case QtFatalMsg:
        DEBUG() << "Error: Unsupported message type";
        RETURN();
    }

    QQuickView* view = new QQuickView( QUrl(QStringLiteral("qrc:/MessageBox.qml")), win_ );
    QQuickItem* item = view->rootObject();
    item->setParentItem( qml("redTimer") );
    item->setY( 30 );

    item->findChild<QQuickItem*>("message")->setProperty( "color", colour );
    item->findChild<QQuickItem*>("message")->setProperty( "text", text );

    QTimer* errorTimer = new QTimer( this );
    errorTimer->singleShot( timeout, this, [=](){ if(item) item->deleteLater(); } );

    RETURN();
}

QQuickItem*
RedTimer::qml( QString qmlItem )
{
    ENTER()(qmlItem);
    RETURN( item_->findChild<QQuickItem*>(qmlItem) );
}

void
RedTimer::reconnect()
{
    ENTER();

    redmine_->setUrl( settings_->getUrl() );
    redmine_->setAuthenticator( settings_->getApiKey() );

    refresh();

    RETURN();
}

void
RedTimer::refreshCounter()
{
    ++counter_;
    qmlCounter_->setProperty( "text", QTime(0, 0, 0).addSecs(counter_).toString("HH:mm:ss") );
}

void
RedTimer::start()
{
    ENTER();

    // If no issue is selected, show issue selector
    if( issue_.id == NULL_ID )
    {
        issueSelector_->display();
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
    qml("startStop")->setProperty( "text", "Stop time tracking" );

    // Set the issue status ID to the worked on ID if not already done
    int workedOnId = settings_->getWorkedOnId();
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

    redmine_->sendTimeEntry( timeEntry,
                             [=](bool success, RedmineError redmineError,
                                 QNetworkReply::NetworkError networkError)
    {
        ENTER();

        if( !success && redmineError != ERR_TIME_ENTRY_TOO_SHORT )
        {
            message( tr("Could not save the time entry. Please check your internet connection."),
                     QtCriticalMsg );
            RETURN();
        }

        if( redmineError == ERR_TIME_ENTRY_TOO_SHORT )
            message( tr("Not saving time entries shorter than one minute."), QtWarningMsg );

        if( !stopTimerAfterSaving )
            startTimer();

        if( success )
            message( tr("Saved time %1").arg(QTime(0, 0, 0).addSecs(counter_).toString("HH:mm:ss")) );

        if( success ||
            (resetTimerOnError && redmineError != ERR_TIME_ENTRY_TOO_SHORT) )
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
    qml("startStop")->setProperty( "text", tr("Start time tracking") );

    RETURN();
}

void
RedTimer::refresh()
{
    ENTER();

    refreshActivities();
    refreshIssueStatuses();

    RETURN();
}

void
RedTimer::refreshActivities()
{
    ENTER();

    redmine_->retrieveTimeEntryActivities( [&]( Enumerations activities )
    {
        ENTER();

        int currentIndex = 0;

        activityModel_.clear();
        activityModel_.push_back( SimpleItem("Choose activity") );
        for( const auto& activity : activities )
        {
            if( activity.id == activityId_ )
                currentIndex = activityModel_.rowCount();

            activityModel_.push_back( SimpleItem(activity) );
        }

        DEBUG()(activityModel_)(activityId_)(currentIndex);

        ctx_->setContextProperty( "activityModel", &activityModel_ );

        if( currentIndex != 0 )
            qml("activity")->setProperty( "currentIndex", currentIndex );

        RETURN();
    } );

    RETURN();
}

void
RedTimer::refreshIssueStatuses()
{
    ENTER();

    redmine_->retrieveIssueStatuses( [&]( IssueStatuses issueStatuses )
    {
        ENTER();

        int currentIndex = 0;

        issueStatusModel_.clear();
        issueStatusModel_.push_back( SimpleItem("Choose issue status") );
        for( const auto& issueStatus : issueStatuses )
        {
            if( issueStatus.id == issue_.status.id )
                currentIndex = issueStatusModel_.rowCount();

            issueStatusModel_.push_back( SimpleItem(issueStatus) );
        }

        DEBUG()(issueStatusModel_)(issue_.status.id)(currentIndex);

        ctx_->setContextProperty( "issueStatusModel", &issueStatusModel_ );

        if( currentIndex != 0 )
            qml("issueStatus")->setProperty( "currentIndex", currentIndex );

        RETURN();
    } );

    RETURN();
}
