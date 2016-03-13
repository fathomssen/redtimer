#ifndef REDTIMER_H
#define REDTIMER_H

#include <QApplication>
#include <QQmlApplicationEngine>

#include <memory>

namespace redtimer {

class RedTimer;

using RedTimer_p  = std::shared_ptr<RedTimer>;
using RedTimer_cp = const RedTimer_p;

class RedTimer
{
private:
  QApplication app;
  QQmlApplicationEngine engine;

public:
  RedTimer( int argc, char *argv[] );

  /**
   * @brief Display the main window
   */
  void init();

  /**
   * @brief Run the application
   * @return Status code
   */
  int display();
};

} // redtimer

#endif // REDTIMER_H
