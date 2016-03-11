#include "redtimer.h"

using namespace redtimer;

RedTimer::RedTimer( int argc, char *argv[] )
  : app(argc, argv)
{
}

void
RedTimer::init()
{
  engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
}

int
RedTimer::display()
{
  return app.exec();
}
