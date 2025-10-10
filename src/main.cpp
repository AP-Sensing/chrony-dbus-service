#include <simppl/dispatcher.h>

#include <cstdlib>
#include <print>

#include "chrony/dbus/version/Version.hpp"

#define HAVE_LONG_TIME_T  // Indicate to chrony that it should use/expect long time_t (64bit) values.
#include "chrony/dbus/chrony_dbus_service.hpp"

int main()
{
    std::println("Starting chrony DBus service version {0}", APS_CHRONY_DBUS_SERVICE_VERSION_FULL_STRING);

    /// @todo maybe add systemd socket activation support?
    simppl::dbus::Dispatcher disp("bus:system");
    const ChronyDBusService chronyService(disp);

    disp.run();

    return EXIT_SUCCESS;
}
