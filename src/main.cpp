#include <simppl/dispatcher.h>

#include "dbus/chronydbusservice.h"

int main()
{
    /// @todo maybe add systemd socket activation support?
    simppl::dbus::Dispatcher disp("bus:session");
    const ChronyDBusService chronyService(disp);

    disp.run();

    return 0;
}
