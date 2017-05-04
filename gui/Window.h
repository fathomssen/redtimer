#pragma once

#include <functional>

#include <QEvent>
#include <QHash>
#include <QObject>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QString>

namespace redtimer {

// forward declaration
class MainWindow;
class Settings;
struct ProfileData;

class Window : public QQuickView
{
    Q_OBJECT

    friend class MainWindow;

private:
    /// Close callback
    std::function<void()> closeCb_;

    /// Already displayed messages
    QHash<QString, bool> displayed_;

    /// Main item
    QQuickItem* item_ = nullptr;

    /// Window context
    QQmlContext* ctx_ = nullptr;

    /// Main window
    MainWindow* mainWindow_ = nullptr;

    /// Settings
    Settings* settings_ = nullptr;

protected:
    /// Counter to ensure that there are no idle callbacks after deleting the object
    int callbackCounter_ = 0;

    /// Flag to determine whether deleteLater() has been called
    bool deleteLater_ = false;

protected:
    /// Emit the closed signal upon closing
    bool emitClosedSignal_ = true;

    /// @name Getter
    /// @{

    /**
     * @brief Determines whether a connection is currently available
     *
     * @return true if connection is available, false otherwise
     */
    bool connected();

    /**
     * @brief Get the main window
     *
     * @return Main window object
     */
    MainWindow* mainWindow();

    /**
     * @brief Get the current profile data
     *
     * @return Profile data
     */
    ProfileData* profileData();

    /**
     * @brief Get a QML GUI item
     *
     * Fetches the root item if \c qmlItem is empty.
     *
     * @param qmlItem Name of the QML GUI item
     *
     * @return QML GUI item
     */
    QQuickItem* qml( QString qmlItem = "" );

    /**
     * @brief Get the settings object
     *
     * @return Settings
     */
    Settings* settings();

    /// @}

    /// @name Setter
    /// @{

    /**
     * @brief Set a context property
     *
     * @param key Key
     * @param value Value
     */
    void setCtxProperty( QString key, QObject* value );

    /// @}

public:
    /// Position and size of windows
    struct Data
    {
        /// Last window size
        QRect geometry;

        /// Last window position
        QPoint position;
    };

    /**
     * @brief Window constructor for a window from a QML file within a Qt resource file
     *
     * @param qml Path to the QML file within the Qt resouce file
     * @param mainWindow Main window object
     * @param closeCb Close callback
     */
    Window( QString qml, MainWindow* mainWindow, std::function<void()> closeCb = nullptr );

protected:
    /**
     * @brief Filter Qt events
     *
     * Emits the canceled signal when window has been closed.
     *
     * @param event Received event
     *
     * @return true if event has been processed, false otherwise
     */
    bool event( QEvent* event );

    /**
     * @brief Filter Qt key events
     *
     * Close window with Esc key.
     *
     * @param event Received key event
     */
    void keyPressEvent( QKeyEvent* event );

    /**
     * @brief Get window data
     *
     * @param windowData Window data
     */
    Data getWindowData();

    /**
     * @brief Set window data
     *
     * @param windowData Window data
     */
    void setWindowData( Data windowData );

public slots:
    /**
     * @brief Delete the object after all callbacks have finished
     */
    void deleteLater();

    /**
     * @brief Display a message
     *
     * @param text Message to display
     * @param type Message type (one of \c QtInfoMsg, \c QtWarningMsg and \c QtCriticalMsg)
     * @param force Force displaying the message even if it has been displayed before (default: true)
     */
    QQuickItem* message( QString text, QtMsgType type = QtInfoMsg, bool force = true );

    /**
     * @brief Delete an already displayed message
     *
     * @param text Message to delete
     */
    void deleteMessage( QString text );

signals:
    /**
     * @brief Signal that will be emitted when closing the window
     *
     * This signal will be emitted only once.
     */
    void closed();
};

/**
 * @brief QDebug stream operator for window data
 *
 * @param debug QDebug stream
 * @param item Window data
 *
 * @return QDebug stream
 */
inline QDebug
operator<<( QDebug debug, const Window::Data& item )
{
    QDebugStateSaver saver( debug );
    debug.nospace() << "[position: " << item.position << ", geometry: \"" << item.geometry << "\"]";

    return debug;
}

} // redtimer
