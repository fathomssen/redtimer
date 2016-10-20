#include "qtredmine/Logging.h"

#include "MainWindow.h"
#include "ProfileData.h"
#include "Settings.h"
#include "Window.h"

#include <QTimer>
#include <QUrl>

using namespace redtimer;
using namespace std;

Window::Window( QString qml, MainWindow* mainWindow, std::function<void()> closeCb )
    : QQuickView( QUrl(QString("qrc:/qml/").append(qml).append(".qml")) )
{
    ENTER();

    // Issue selector window access members
    ctx_ = rootContext();
    item_ = qobject_cast<QQuickItem*>( rootObject() );
    mainWindow_ = mainWindow;

    if( !settings_ )
        settings_ = mainWindow_->settings();

    setResizeMode( QQuickView::SizeRootObjectToView );

    setMinimumHeight( height() );
    setMinimumWidth( width() );

    if( closeCb )
        closeCb_ = closeCb;
    else
        closeCb_ = [&](){ close(); };

    RETURN();
}

void
Window::keyPressEvent( QKeyEvent* event )
{
    // Close window with Esc key
    if( event->key() == Qt::Key_Escape )
    {
        DEBUG("Closing window with Esc key");
        closeCb_();
    }

    QQuickView::keyPressEvent( event );
}

bool
Window::event( QEvent* event )
{
    // Show warning on close and if timer is running
    if( emitClosedSignal_ && event->type() == QEvent::Close )
    {
        DEBUG("Received close signal")(this);
        emitClosedSignal_ = false;
        closed();
    }

    return QQuickView::event( event );
}

bool
Window::connected()
{
    ENTER();

    bool ret = mainWindow_->connected();

    RETURN( ret );
}

void
Window::deleteLater()
{
    ENTER()(deleteLater_)(callbackCounter_);

    deleteLater_ = true;

    if( callbackCounter_ == 0 )
    {
        DEBUG() << "Deleting the Window object";
        QQuickView::deleteLater();
    }
    else
        DEBUG() << "Not yet deleting the Window object";

    RETURN();
}

void
Window::deleteMessage( QString text )
{
    ENTER()(text);

    displayed_.remove( text );

    RETURN();
}

MainWindow*
Window::mainWindow()
{
    ENTER();
    RETURN( mainWindow_ );
}

QQuickItem*
Window::message( QString text, QtMsgType type, bool force )
{
    ENTER()(text)(type)(force);

    if( !force && displayed_.contains(text) )
        RETURN( nullptr );

    QString colour;
    QSystemTrayIcon::MessageIcon icon;

    switch( type )
    {
    case QtInfoMsg:
        colour = "#006400";
        icon = QSystemTrayIcon::Information;
        break;
    case QtWarningMsg:
        colour = "#FF8C00";
        icon = QSystemTrayIcon::Warning;
        break;
    case QtCriticalMsg:
        colour = "#8B0000";
        icon = QSystemTrayIcon::Critical;
        break;
    case QtDebugMsg:
    case QtFatalMsg:
        DEBUG() << "Error: Unsupported message type";
        RETURN( nullptr );
    }

    QQuickItem* item = nullptr;

    // If tray icon is displayed, only show a tray notification
    if( mainWindow_->trayIcon() )
    {
        mainWindow_->trayIcon()->showMessage( "RedTimer", text, icon );
    }
    else
    {
        int timeout = 5000;

        QQuickView* view = new QQuickView( QUrl(QStringLiteral("qrc:/qml/MessageBox.qml")), this );
        item = view->rootObject();
        item->setParentItem( item_ );

        item->findChild<QQuickItem*>("message")->setProperty( "color", colour );
        item->findChild<QQuickItem*>("message")->setProperty( "text", text );

        if( type != QtCriticalMsg )
        {
            QTimer* timer = new QTimer( this );
            timer->setSingleShot( true );
            timer->setInterval( timeout );
            timer->start();

            connect( timer, &QTimer::timeout, this, [=](){ if(item) item->deleteLater(); } );
        }
    }

    displayed_.insert( text, true );

    RETURN( item );
}

Window::Data
Window::getWindowData()
{
    ENTER();

    Data data;
    data.geometry = geometry();
    data.position = position();

    RETURN( data );
}

ProfileData*
Window::profileData()
{
    ENTER();

    int profileId = mainWindow_->profileId();
    ProfileData* profileData = settings_->profileData( profileId );

    RETURN( profileData, *profileData );
}

QQuickItem*
Window::qml( QString qmlItem )
{
    ENTER()(qmlItem);

    if( qmlItem.isEmpty() )
        RETURN( item_ );
    else
    {
        QQuickItem* child = item_->findChild<QQuickItem*>( qmlItem );
        if( !child )
        {
            DEBUG() << "QML item not found";
            throw( "QML item not found" );
        }

        RETURN( child );
    }
}

void
Window::setCtxProperty( QString key, QObject* value )
{
    ENTER();
    ctx_->setContextProperty( key, value );
    RETURN();
}

void
Window::setProfileId( int profileId )
{
    ENTER()(profileId);

    settings_->setProfileId( profileId );

    RETURN();
}

void
Window::setProfileId( QString profileName )
{
    ENTER()(profileName);

    if( !profileName.isEmpty() )
        settings_->setProfileId( profileName );

    RETURN();
}

Settings*
Window::settings()
{
    ENTER();
    RETURN( settings_, *settings_ );
}

void
Window::setWindowData( Window::Data data )
{
    ENTER()(data.geometry)(data.position);

    QRect geometry = data.geometry;
    if( !geometry.isNull() )
        setGeometry( geometry );

    QPoint position = data.position;
    if( !position.isNull() )
        setPosition( position );

    RETURN();
}
