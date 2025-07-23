#include <iostream>

#include "chronydbustest.h"

#include <iostream>
#include <cassert>

#include <simppl/dispatcher.h>
#include <simppl/stub.h>
#include <simppl/skeleton.h>

#include "../dbus/dbusinterface.h"
using namespace std;

/// @todo add gtest

int main()
{
    simppl::dbus::Dispatcher disp("bus:session");
    simppl::dbus::Stub<org::freedesktop::ChronyDBus> stub(disp, "chronyDBusServer");

    std::vector<ChronySourceData> sources = stub.getSources();

    for(const auto &source : sources)
    {
        std::cout << "Received source: " << source.ipAddress << std::endl;
    }

    stub.setManualTimeEnabled(true);

    stub.setManualTime("2025-07-17 09:00:00");

    std::vector<std::string> manualTimeList = stub.getManualTimeList();
    for(const auto &timeStr : manualTimeList)
    {
        std::cout << "Received manual time entry: " << timeStr << std::endl;
    }

    std::vector<AddServersData> newServers {};
    AddServersData s1, s2;
    s1.name="127.0.0.1";
    newServers.push_back(s1);
    s2.name="127.0.0.2";
    newServers.push_back(s2);

    for(const auto & server : newServers)
    {
        std::cout << "adding new server: " << server.name << std::endl;
    }
    stub.addServers(newServers);

    std::vector<std::string> delServers = {{"127.0.0.1"},{"127.0.0.2"}};
    stub.deleteServers(delServers);

    return 0;
}
