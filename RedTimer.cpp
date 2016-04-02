#include "RedTimer.h"
#include "logging.h"

#include <QEventLoop>
#include <QMessageBox>
#include <QObject>

using namespace qtredmine;
using namespace redtimer;
using namespace std;

RedTimer::RedTimer( QObject* parent )
    : QObject( parent )
{
    ENTER();

    // Main window initialisation
    win_ = new QQuickView();
    win_->installEventFilter( this );
    win_->setResizeMode( QQuickView::SizeRootObjectToView );
    win_->setSource( QUrl(QStringLiteral("qrc:/RedTimer.qml")) );
    win_->setModality( Qt::ApplicationModal );

    // Additional window manager properties
    Qt::WindowFlags flags = Qt::Window;
    flags |= Qt::CustomizeWindowHint  | Qt::WindowTitleHint;
    flags |= Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint;
    win_->setFlags( flags );

    win_->show();

    // Main window access members
    ctx_ = win_->rootContext();
    item_ = qobject_cast<QQuickItem*>( win_->rootObject() );
    qmlCounter_ = item_->findChild<QQuickItem*>( "counter" );

    // Connect to Redmine
    redmine_ = new SimpleRedmineClient( this );

    // Settings initialisation
    settings_ = new Settings( this );
    settings_->load();
    reconnect();

    // Issue selector initialisation
    issueSelector_ = new IssueSelector( redmine_, this );
    issueSelector_->setProjectId( settings_->getProject() );

    // Timer initialisation
    timer_ = new QTimer( this );
    timer_->setTimerType( Qt::VeryCoarseTimer );
    timer_->setInterval( 1000 );

    // Counter initialisation
    counter_ = 0;

    // Apply loaded settings
    activityId_ = settings_->getActivity();
    issueSelector_->setProjectId( settings_->getProject() );
    loadIssue( settings_->getIssue(), false );

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
    settings_->save();

    RETURN();
}

bool RedTimer::eventFilter( QObject* obj, QEvent* event )
{
    // Show warning on close and if timer is running
    if( event->type() == QEvent::Close && timer_->isActive() )
    {
        DEBUG() << "Received close event while timer is running";

        QMessageBox msgBox( QMessageBox::Warning, QString("RedTimer"),
                            QString("The timer is currently running"),
                            QMessageBox::NoButton, qobject_cast<QWidget*>(win_) );
        msgBox.setInformativeText( "Do you want to save the logged time?" );
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
            // Save the currently tracked time by stopping the tracking            
            stop();

            DEBUG() << "Entering quit loop";
            QEventLoop* quitLoop = new QEventLoop( this );
            connect( this, &RedTimer::timeEntrySaved, [=](){ quitLoop->quit(); } );
            quitLoop->exec();
            DEBUG() << "Leaving quit loop";
        }

        default:
            // The call to QObject::eventFilter below will eventually close the window
            break;
        }

        DEBUG() << "Passing the close event to QObject";
    }

    return QObject::eventFilter( obj, event );
}

void
RedTimer::init()
{
    ENTER();

    // Connect the settings button
    connect( item_->findChild<QQuickItem*>("settings"), SIGNAL(clicked()),
             settings_, SLOT(display()) );

    // Connect the issue selector button
    connect( item_->findChild<QQuickItem*>("selectIssue"), SIGNAL(clicked()),
             issueSelector_, SLOT(display()) );

    // Connect the text field
    connect( item_->findChild<QQuickItem*>("quickPick"), SIGNAL(accepted()),
             this, SLOT(loadIssue()) );

    // Connect the start/stop button
    connect( item_->findChild<QQuickItem*>("startStop"), SIGNAL(clicked()),
             this, SLOT(startStop()) );

    // Connect the project selected signal to the projectSelected slot
    connect( item_->findChild<QQuickItem*>("activity"), SIGNAL(activated(int)),
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
RedTimer::loadIssue()
{
    ENTER();

    int issueId = item_->findChild<QQuickItem*>("quickPick")->property("text").toInt();
    item_->findChild<QQuickItem*>("quickPick")->setProperty( "text", "" );

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
        item_->findChild<QQuickItem*>("issueData")->setProperty( "text", issueData );

        if( startTimer )
            start();

        RETURN();
    },
    issueId );

    RETURN();
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
    timer_->start();

    // Set the start/stop button icon to stop
    item_->findChild<QQuickItem*>("startStop")->setProperty(
                "iconSource", "qrc:///open-iconic/svg/media-stop.svg" );
    item_->findChild<QQuickItem*>("startStop")->setProperty( "text", "Stop time tracking" );

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
RedTimer::stop( bool resetTimerOnError, bool stopTimer )
{
    ENTER();

    // Save the tracked time
    TimeEntry timeEntry;
    timeEntry.activity.id = activityId_;
    timeEntry.hours       = (double)counter_ / 3600; // Seconds to hours conversion
    timeEntry.issue.id    = issue_.id;

    redmine_->createTimeEntry( timeEntry, [=](bool success, Error reason)
    {
        ENTER();

        if( !success && reason != Error::TIME_ENTRY_TOO_SHORT )
            RETURN();

        if( stopTimer )
        {
            timer_->stop();

            // Set the start/stop button icon to start
            item_->findChild<QQuickItem*>("startStop")->setProperty(
                        "iconSource", "qrc:///open-iconic/svg/media-play.svg" );
            item_->findChild<QQuickItem*>("startStop")->setProperty( "text", "Start time tracking" );
        }

        if( success ||
            (resetTimerOnError && reason != Error::TIME_ENTRY_TOO_SHORT) )
        {
            counter_ = 0;
            qmlCounter_->setProperty( "text", "00:00:00" );
        }

        timeEntrySaved();

        RETURN();
    });

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
RedTimer::activitySelected( int index )
{
    ENTER();

    activityId_ = activityModel_.at(index).id();
    DEBUG()(index)(activityId_);

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
            item_->findChild<QQuickItem*>("activity")->setProperty( "currentIndex", currentIndex );

        RETURN();
    } );
}

void
RedTimer::updateIssueStatuses()
{
    ENTER();
    RETURN();
}
