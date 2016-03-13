#ifndef REDTIMER_H
#define REDTIMER_H

#include "Settings.h"
//#include "redmine-qt/RedmineClient.hpp"

#include <QApplication>
#include <QObject>
#include <QQuickView>

#include <memory>

namespace redtimer {

// forward declaration of class RedTimer
class RedTimer;

// some typedefs for shared pointers
//using RedmineClient_p  = std::shared_ptr<RedmineClient>;
//using RedmineClient_cp = const RedmineClient_p;

class RedTimer : public QObject
{
  Q_OBJECT

private:
  QApplication app_;

  /**
   * @brief Redmine client object
   *
   * Object that holds the connection to a Redmine instance
   */
//  RedmineClient_p redmineClient_;

  /**
   * @brief Redmine connection dialog object
   */
  Settings* settings_;

  /**
   * @brief Main window object
   */
  QQuickView* win_;

public:
  explicit RedTimer( int argc, char *argv[] );

  /**
   * @brief Run the application
   * @return Status code
   */
  int display();

public slots:
};

} // redtimer

#endif // REDTIMER_H
