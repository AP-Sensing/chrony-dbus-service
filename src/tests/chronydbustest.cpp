#include <gtest/gtest.h>

#include <simppl/dispatcher.h>
#include <simppl/error.h>
#include <simppl/stub.h>

#include <cassert>
// NOLINTNEXTLINE(misc-include-cleaner)
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
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
        const std::string targetTime = "2026-01-01 09:00:00";
        const std::string otherTime = "2026-01-01 10:00:00";

        // add manual time
        stub.setManualTimeEnabled(true);

        int tries = 0;
        bool foundTarget = false;
        std::vector<std::string> manualTimeList;
        // need to retry once because chrony returns the time with slew at first and it is not possible to trigger
        // `chronyc makestep` in a container
        /* e.g.:
                [root@localhost build]# chronyc manual on
                200 OK
                [root@localhost build]# chronyc settime 16:00
                200 OK
                Clock was -28506.27 seconds fast.  Frequency change = 0.00ppm, new frequency = 16.92ppm
                [root@localhost build]# chronyc manual list
                210 n_samples = 1
                #    Date     Time(UTC)    Slewed   Original   Residual
                =======================================================
                 0 2025-08-21 08:04:53  -28506.27  -28506.27       0.00
        */
        while (!foundTarget && tries < 2)
        {
            stub.clearManualTimeList();
            stub.addManualTime(otherTime);
            // NOLINTNEXTLINE(misc-include-cleaner)
            std::this_thread::sleep_for(250ms);

            stub.clearManualTimeList();
            stub.addManualTime(targetTime);
            // NOLINTNEXTLINE(misc-include-cleaner)
            std::this_thread::sleep_for(250ms);

            manualTimeList = stub.getManualTimeList();
            // find our targetTime
            for (const auto &timeStr : manualTimeList)
            {
                if (timeStr == targetTime) { foundTarget = true; }
                else { std::cout << "found unexpected time: " << timeStr << "\n"; }
            }
            // NOLINTNEXTLINE(misc-include-cleaner)
            std::this_thread::sleep_for(500ms);
            ++tries;
        }
        EXPECT_TRUE(foundTarget);

        stub.clearManualTimeList();

        manualTimeList = stub.getManualTimeList();
        EXPECT_EQ(manualTimeList.size(), 0);
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
        // NOLINTNEXTLINE(misc-include-cleaner)
        std::this_thread::sleep_for(250ms);
        const std::vector<ChronySourceData> sourcesAfterAdd = stub.getSources();
        bool foundAddedServer = false;

        for (const auto &sourceData : sourcesAfterAdd)
        {
            if (sourceData.name == s1.name)
            {
                std::cout << "found added server" << "\n";
                foundAddedServer = true;
            }
        }
        EXPECT_TRUE(foundAddedServer);

        // delete server
        const std::vector<std::string> delServers = {{s1.name}};
        for (const auto &serverName : delServers) { std::cout << "deleting server: " << serverName << "\n"; }
        stub.deleteServers(delServers);
        // NOLINTNEXTLINE(misc-include-cleaner)
        std::this_thread::sleep_for(250ms);

        const std::vector<ChronySourceData> sourcesAfterDelete = stub.getSources();
        bool foundDeletedServer = false;

        for (const auto &sourceData : sourcesAfterDelete)
        {
            if (sourceData.name == s1.name)
            {
                std::cout << "Error: deleted server still in the list" << "\n";
                foundDeletedServer = true;
            }
        }
        EXPECT_FALSE(foundDeletedServer);
    }
    catch (const simppl::dbus::Error &e)
    {
        FAIL() << "Aborting after DBus error: '" << e.name() << "' message: '" << e.message() << "'\n";
    }
}
