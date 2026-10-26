#include <simppl/dispatcher.h>

#include <cstdlib>
#include <iostream>
#include <ostream>
#include <print>
#include <string_view>

#include "chrony/dbus/version/Version.hpp"

#include "aps/chrony-dbus/chrony_dbus_service.hpp"

namespace
{
void printUsage(std::ostream& output, std::string_view program)
{
    output << "Usage: " << program << " [-d] [-h]\n"
           << "  -d  Enable debug logging\n"
           << "  -h  Show this help and exit\n";
}
};  //namespace

int main(int argc, char* argv[])
{
    std::println("Starting chrony DBus service version {0}", APS_CHRONY_DBUS_SERVICE_VERSION_FULL_STRING);

    // Command line arguments
    bool debugLoggingIsEnabled = false;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view argument{argv[i]};

        if (argument == "-d")
        {
            debugLoggingIsEnabled = true;
        }
        else if (argument == "-h")
        {
            printUsage(std::cout, argv[0]);
            return EXIT_SUCCESS;
        }
        else
        {
            std::println("Unknown argument: {0}", argument);
            printUsage(std::cerr, argv[0]);
            return EXIT_FAILURE;
        }
    }

    /// @todo maybe add systemd socket activation support?
    simppl::dbus::Dispatcher disp("bus:system");
    const ChronyDBusService chronyService(disp, debugLoggingIsEnabled);

    disp.run();

    return EXIT_SUCCESS;
}
