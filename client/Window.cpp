#include "MainWindow.h"
#include "Window.h"
#include "logging.h"

#include <QTimer>
#include <QUrl>

using namespace redtimer;
using namespace std;

Window::Window( QString qml, MainWindow* mainWindow )
    : QQuickView( QUrl(QString("qrc:/qml/").append(qml).append(".qml")) )
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

void
Window::keyPressEvent( QKeyEvent* event )
{
    // Close window with Esc key
    if( event->key() == Qt::Key_Escape )
    {
        DEBUG("Closing window with Esc key");
        close();
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

WindowData
Window::getWindowData()
{
    ENTER();

    WindowData windowData;
    windowData.geometry = geometry();
    windowData.position = position();

    RETURN( windowData );
}

void
Window::setWindowData( WindowData windowData )
{
    ENTER()(windowData.geometry)(windowData.position);

    QRect geometry = windowData.geometry;
    if( !geometry.isNull() )
        setGeometry( geometry );

    QPoint position = windowData.position;
    if( !position.isNull() )
        setPosition( position );

    RETURN();
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
