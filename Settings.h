#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QQuickItem>
#include <QQuickView>
#include <QSettings>

#include <memory>

namespace redtimer {

class Settings : public QObject
{
  Q_OBJECT

private:
  /**
   * @brief Main window object
   */
  QQuickView* win_;

  /**
   * @brief Main item object
   */
  QQuickItem* item_;

  /**
   * @brief Application settings
   */
  QSettings settings_;

  /**
   * @brief Redmine API key
   */
  QString apikey_;

  /**
   * @brief Redmine URL
   */
  QString url_;

public:
  explicit Settings( QObject *parent = 0 );

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
};

} // redtimer

#endif // SETTINGS_H
