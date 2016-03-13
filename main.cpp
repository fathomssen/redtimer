#include "RedTimer.h"

#include <memory>

using namespace redtimer;
using namespace std;

int main(int argc, char *argv[])
{
  shared_ptr<RedTimer> redTimer = make_shared<RedTimer>( argc, argv );
  return redTimer->display();
}
