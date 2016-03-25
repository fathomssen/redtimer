#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QQuickItem>
#include <QQuickView>
#include <QSettings>

namespace redtimer {

class Settings : public QObject
{
    Q_OBJECT

private:
    /// Main window object
    QQuickView* win_;

    /// Main item object
    QQuickItem* item_;

    /// Application settings
    QSettings settings_;

    /// Redmine API key
    QString apiKey_;

    /// Redmine base URL
    QString url_;

    /// Last opened project
    int projectId_;

public:
    explicit Settings( QObject *parent = 0 );

    /**
     * @brief Get the Redmine API key
     *
     * @return Redmine API key
     */
    QString getApiKey() const;

    /**
     * @brief Get the Redmine base URL
     *
     * @return Redmine base URL
     */
    QString getUrl() const;

    /**
     * @brief Load settings from settings file
     */
    void load();

public slots:
    /**
     * @brief Close the Redmine connection dialog
     */
    void close();

    /**
     * @brief Display the Redmine connection dialog
     */
    void display();

    /**
     * @brief Save preferences from settings dialog to settings file
     */
    void save();

signals:

    /**
     * @brief Emitted when data have been saved
     */
    void saved();
};

} // redtimer

#endif // SETTINGS_H
