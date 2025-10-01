#include <simppl/dispatcher.h>

#define HAVE_LONG_TIME_T

#include "dbus/chronydbusservice.h"

int main()
{
    /// @todo maybe add systemd socket activation support?
    simppl::dbus::Dispatcher disp("bus:system");
    const ChronyDBusService chronyService(disp);

    disp.run();

    return 0;
}
