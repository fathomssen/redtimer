#pragma once

#include "MainWindow.h"
#include "Models.h"
#include "Window.h"

#include <QSortFilterProxyModel>

namespace redtimer {

/**
 * @brief An profile selector for RedTimer
 */
class ProfileSelector : public Window
{
    Q_OBJECT

private:
    /// List of available profile IDs
    QStringList profileIds_;

    /// Current profile ID
    QString profileId_;

    /// List of profiles in the GUI
    SimpleModel profileModel_;

public:
    /**
     * @brief Constructor for an ProfileSelector object
     *
     * @param ids List of profile IDs. If empty, will be loaded within constructor.
     * @param mainWindow Main window object
     */
    explicit ProfileSelector( const QStringList& ids = QStringList(), MainWindow* mainWindow = nullptr );

    /**
     * @brief Display the profile selector.
     */
    void display();

    /**
     * @brief Get all available profile IDs
     *
     * @return List of all available profile IDs
     */
    static QStringList profileIds();

private slots:
    /**
     * @brief Close the profile selector dialog and return selected profile ID
     */
    void apply();

    /**
     * @brief Slot to a selected profile
     */
    void profileSelected( int index );

signals:
    /**
     * @brief Emitted when OK has been pressed
     */
    void applied();

    /**
     * @brief Emitted when an profile has been selected
     */
    void selected( QString profileId );
};

} // redtimer
