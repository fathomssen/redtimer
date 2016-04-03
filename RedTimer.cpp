#include "RedTimer.h"
#include "logging.h"

#include <QMessageBox>
#include <QObject>

using namespace qtredmine;
using namespace redtimer;
using namespace std;

RedTimer::RedTimer( QApplication* parent )
    : QObject( parent ),
      app_( parent )
{
    ENTER();

    // Settings initialisation
    settings_ = new Settings( this );
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

    win_->show();

    // Main window access members
    ctx_ = win_->rootContext();
    item_ = qobject_cast<QQuickItem*>( win_->rootObject() );
    qmlCounter_ = qml( "counter" );

    // Connect to Redmine
    redmine_ = new SimpleRedmineClient( this );

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

    // Set transient window parent
    settings_->window()->setTransientParent( win_ );
    issueSelector_->window()->setTransientParent( win_ );

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
    settings_->save();

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
RedTimer::init()
{
    ENTER();

    // Connect the settings button
    connect( qml("settings"), SIGNAL(clicked()),
             settings_, SLOT(display()) );

    // Connect the reload button
    connect( qml("reload"), SIGNAL(clicked()),
             this, SLOT(reconnect()) );

    // Connect the issue selector button
    connect( qml("selectIssue"), SIGNAL(clicked()),
             issueSelector_, SLOT(display()) );

    // Connect the text field
    connect( qml("quickPick"), SIGNAL(accepted()),
             this, SLOT(loadIssue()) );

    // Connect the start/stop button
    connect( qml("startStop"), SIGNAL(clicked()),
             this, SLOT(startStop()) );

    // Connect the project selected signal to the projectSelected slot
    connect( qml("activity"), SIGNAL(activated(int)),
             this, SLOT(activitySelected(int)) );

    // Connect the settings saved signal to the reconnect slot
    connect( settings_, &Settings::applied, this, &RedTimer::reconnect );

    // Connect the issue selected signal to the setIssue slot
    connect( issueSelector_, &IssueSelector::selected,
             [=](int issueId){ loadIssue(issueId); } );

    // Connect the timer to the tracking counter
    connect( timer_, &QTimer::timeout, this, &RedTimer::refreshCounter );

    // Initially update the GUI
    update();

    RETURN();
}

bool RedTimer::eventFilter( QObject* obj, QEvent* event )
{
    // Show warning on close and if timer is running
    if( event->type() == QEvent::Close && timer_->isActive() )
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
            return true;

        case QMessageBox::Save:
        {
            DEBUG() << "Saving time entry before closing the application";
            connect( this, &RedTimer::timeEntrySaved, [=](){ app_->quit(); } );
            stop();
            return true;
        }

        default:
            DEBUG() << "Passing the close event to QObject";
            // The call to QObject::eventFilter below will eventually close the window
            break;
        }
    }

    return QObject::eventFilter( obj, event );
}

void
RedTimer::loadIssue()
{
    ENTER();

    int issueId = qml("quickPick")->property("text").toInt();
    qml("quickPick")->setProperty( "text", "" );

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

        QString issueData = QString( "Issue #%1\n\nSubject: %2\n\n%3" )
                .arg(issue.id)
                .arg(issue.subject)
                .arg(issue.description);
        qml("issueData")->setProperty( "text", issueData );

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

    update();

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
    if( issue_.id == -1 )
    {
        issueSelector_->display();
        RETURN();
    }

    // Afterwards, start the timer again
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

    RETURN();
}

void
RedTimer::stop( bool resetTimerOnError, bool stopTimerAfterSaving )
{
    ENTER();

    // Check that an activity has been selected
    if( activityId_ == -1 )
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

    redmine_->createTimeEntry( timeEntry, [=](bool success, Error reason)
    {
        ENTER();

        if( !success && reason != Error::TIME_ENTRY_TOO_SHORT )
        {
            message( tr("Could not save the time entry. Please check your internet connection."),
                     QtCriticalMsg );
            startTimer();
            RETURN();
        }

        if( reason == Error::TIME_ENTRY_TOO_SHORT )
            message( tr("Not saving time entries shorter than one minute."), QtWarningMsg );

        if( !stopTimerAfterSaving )
            startTimer();

        if( success )
            message( tr("Saved time %1").arg(QTime(0, 0, 0).addSecs(counter_).toString("HH:mm:ss")) );

        if( success ||
            (resetTimerOnError && reason != Error::TIME_ENTRY_TOO_SHORT) )
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
RedTimer::update()
{
    ENTER();

    updateActivities();
    updateIssueStatuses();

    RETURN();
}

void
RedTimer::updateActivities()
{
    ENTER();

    redmine_->retrieveTimeEntryActivities( [&]( Enumerations activities )
    {
        ENTER();

        int currentIndex = 0;

        // Sort issues ascending by ID
        qSort( activities.begin(), activities.end(),
               []( const Enumeration& a, const Enumeration& b ){ return a.id < b.id; } );

        activityModel_.clear();
        activityModel_.insert( SimpleItem("Choose activity") );
        for( const auto& activity : activities )
        {
            if( activity.id == activityId_ )
                currentIndex = activityModel_.rowCount();

            activityModel_.insert( SimpleItem(activity) );
        }

        DEBUG()(activityModel_)(activityId_)(currentIndex);

        ctx_->setContextProperty( "activityModel", &activityModel_ );

        if( currentIndex != 0 )
            qml("activity")->setProperty( "currentIndex", currentIndex );

        RETURN();
    } );
}

void
RedTimer::updateIssueStatuses()
{
    ENTER();
    RETURN();
}
