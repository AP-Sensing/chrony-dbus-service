#include <simppl/dispatcher.h>

#include "dbus/chronydbusservice.h"

int main()
{
    simppl::dbus::Dispatcher disp("bus:session");
    const ChronyDBusService chronyService(disp);

    disp.run();

    return 0;
}
