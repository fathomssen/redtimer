#include "MainWindow.h"
#include "Window.h"
#include "logging.h"

#include <QTimer>
#include <QUrl>

using namespace redtimer;
using namespace std;

Window::Window( QString qml, MainWindow* mainWindow )
    : QQuickView( QUrl(qml) )
{
    ENTER();

    // Issue selector window access members
    ctx_ = rootContext();
    item_ = qobject_cast<QQuickItem*>( rootObject() );
    mainWindow_ = mainWindow;

    setResizeMode( QQuickView::SizeRootObjectToView );

    setMinimumHeight( height() );
    setMinimumWidth( width() );

    RETURN();
}

bool
Window::event( QEvent* event )
{
    // Show warning on close and if timer is running
    if( emitClosedSignal_ && event->type() == QEvent::Close )
    {
        DEBUG("Received close signal")(this)(event->type());
        emitClosedSignal_ = false;
        closed();
    }

    return QQuickView::event( event );
}

MainWindow*
Window::mainWindow()
{
    ENTER();
    RETURN( mainWindow_ );
}

QQuickItem*
Window::message( QString text, QTimer* timer, QtMsgType type )
{
    ENTER()(text)(timer)(type);

    QString colour = "#006400";
    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information;

    switch( type )
    {
    case QtInfoMsg:
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
        QQuickView* view = new QQuickView( QUrl(QStringLiteral("qrc:/MessageBox.qml")), this );
        item = view->rootObject();
        item->setParentItem( item_ );

        item->findChild<QQuickItem*>("message")->setProperty( "color", colour );
        item->findChild<QQuickItem*>("message")->setProperty( "text", text );

        if( timer )
            connect( timer, &QTimer::timeout, this, [=](){ if(item) item->deleteLater(); } );
    }

    RETURN( item );
}

QQuickItem*
Window::message( QString text, QtMsgType type, int timeout )
{
    ENTER()(text)(type)(timeout);

    QTimer* timer = nullptr;

    if( type != QtCriticalMsg && timeout > 0 )
    {
        timer = new QTimer();
        timer->setSingleShot( true );
        timer->setInterval( timeout );
        timer->start();
    }

    RETURN( message(text, timer, type) );
}

QQuickItem*
Window::qml( QString qmlItem )
{
    ENTER()(qmlItem);

    if( qmlItem.isEmpty() )
        RETURN( item_ );
    else
        RETURN( item_->findChild<QQuickItem*>(qmlItem) );
}
