#include "chronydbustest.h"

#include <gtest/gtest.h>

#include <simppl/dispatcher.h>
#include <simppl/error.h>
#include <simppl/stub.h>

#include <cassert>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "../dbus/dbusinterface.h"

using namespace std::chrono_literals;

TEST(ChronyDBusService, GetSources)
{
    try
    {
        simppl::dbus::Dispatcher dispatch("bus:session");
        simppl::dbus::Stub<org::freedesktop::ChronyDBus> stub(dispatch, "chronyDBusServer");

        const std::vector<ChronySourceData> sources = stub.getSources();

        EXPECT_GT(sources.size(), 0);
    }
    catch (const simppl::dbus::Error &e)
    {
        FAIL() << "Aborting after DBus error: '" << e.name() << "' message: '" << e.message() << "'\n";
    }
}

TEST(ChronyDBusService, AddClearManualTime)
{
    try
    {
        simppl::dbus::Dispatcher dispatch("bus:session");
        simppl::dbus::Stub<org::freedesktop::ChronyDBus> stub(dispatch, "chronyDBusServer");
        const std::string targetTime = "2025-07-17 09:00:00";

        // add manual time
        stub.setManualTimeEnabled(true);

        stub.addManualTime(targetTime);

        std::vector<std::string> manualTimeList = stub.getManualTimeList();
        bool foundTarget = false;
        // there can be multiple manual times set but we only care about targetTime
        for (const auto &timeStr : manualTimeList)
        {
            if (timeStr == targetTime) foundTarget = true;
        }

        EXPECT_TRUE(foundTarget);

        // clear manual time list
        stub.clearManualTimeList();

        manualTimeList = stub.getManualTimeList();
        std::cout << "Manual time list size after clearing: " << manualTimeList.size() << "\n";
    }
    catch (const simppl::dbus::Error &e)
    {
        FAIL() << "Aborting after DBus error: '" << e.name() << "' message: '" << e.message() << "'\n";
    }
}

TEST(ChronyDBusService, AddDeleteServers)
{
    try
    {
        simppl::dbus::Dispatcher dispatch("bus:session");
        simppl::dbus::Stub<org::freedesktop::ChronyDBus> stub(dispatch, "chronyDBusServer");

        // add server
        std::vector<AddServersData> newServers{};
        AddServersData s1;
        s1.name = "time1.uni-paderborn.de";
        newServers.push_back(s1);

        for (const auto &server : newServers) { std::cout << "adding new server: " << server.name << "\n"; }
        stub.addServers(newServers);
        std::this_thread::sleep_for(250ms);
        const std::vector<ChronySourceData> sourcesAfterAdd = stub.getSources();
        bool foundAddedServer = false;

        for (const auto &sourceData : sourcesAfterAdd)
        {
            if (sourceData.name == s1.name) { foundAddedServer = true; }
        }
        EXPECT_TRUE(foundAddedServer);

        // delete server
        const std::vector<std::string> delServers = {{s1.name}};
        for (const auto &serverName : delServers) { std::cout << "deleting server: " << serverName << "\n"; }
        stub.deleteServers(delServers);

        std::this_thread::sleep_for(250ms);

        const std::vector<ChronySourceData> sourcesAfterDelete = stub.getSources();
        bool foundDeletedServer = false;

        for (const auto &sourceData : sourcesAfterDelete)
        {
            if (sourceData.name == s1.name) { foundDeletedServer = true; }
        }
        EXPECT_FALSE(foundDeletedServer);
    }
    catch (const simppl::dbus::Error &e)
    {
        FAIL() << "Aborting after DBus error: '" << e.name() << "' message: '" << e.message() << "'\n";
    }
}
