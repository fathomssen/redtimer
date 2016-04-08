#include "RedTimer.h"

#include <QApplication>

#include <memory>

using namespace redtimer;
using namespace std;

int main(int argc, char* argv[])
{
    QApplication app( argc, argv );

    app.setWindowIcon( QIcon(":/icons/clock_red.svg") );

    new RedTimer( &app );

    return app.exec();
}
