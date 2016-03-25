#include "RedTimer.h"

#include <QApplication>

#include <memory>

using namespace redtimer;
using namespace std;

int main(int argc, char* argv[])
{
    QApplication app( argc, argv );

    new RedTimer( &app );

    return app.exec();
}
