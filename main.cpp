#include <redtimer.h>

#include <memory>

using namespace redtimer;
using namespace std;

int main(int argc, char *argv[])
{
  RedTimer_p redTimer = make_shared<RedTimer>( argc, argv );
  redTimer->init();
  return redTimer->display();
}

