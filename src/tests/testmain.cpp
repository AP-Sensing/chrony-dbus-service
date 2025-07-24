#include <simppl/dispatcher.h>
#include <simppl/error.h>
#include <simppl/stub.h>

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "../dbus/dbusinterface.h"
/// @todo add gtest
// #include "chronydbustest.h"
using namespace std;

int main()
{
    try
    {
        /// @todo start DBus and export the session address
        /// @todo start chrony -xd
        /// @todo start chrony-dbus-service
        simppl::dbus::Dispatcher disp("bus:session");
        simppl::dbus::Stub<org::freedesktop::ChronyDBus> stub(disp, "chronyDBusServer");

        const std::vector<ChronySourceData> sources = stub.getSources();

        for (const auto &source : sources) { std::cout << "Received source: " << source.ipAddress << "\n"; }

        stub.setManualTimeEnabled(true);

        stub.setManualTime("2025-07-17 09:00:00");

        std::vector<std::string> manualTimeList = stub.getManualTimeList();
        for (const auto &timeStr : manualTimeList) { std::cout << "Received manual time entry: " << timeStr << "\n"; }

        stub.clearManualTimeList();

        manualTimeList = stub.getManualTimeList();
        std::cout << "Manual time list size after clearing: " << manualTimeList.size() << "\n";

        std::vector<AddServersData> newServers{};
        AddServersData s1;
        s1.name = "time1.uni-paderborn.de";
        newServers.push_back(s1);

        for (const auto &server : newServers) { std::cout << "adding new server: " << server.name << "\n"; }
        stub.addServers(newServers);

        const std::vector<std::string> delServers = {{"time1.uni-paderborn.de"}};
        stub.deleteServers(delServers);
    }
    catch (const simppl::dbus::Error &e)
    {
        std::cerr << "Aborting after DBus error: '" << e.name() << "' message: '" << e.message() << "'\n";
        return 1;
    }

    return 0;
}
