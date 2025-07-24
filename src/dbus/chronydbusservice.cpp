#include "chronydbusservice.h"

// #include <netdb.h>
// #include <resolv.h>
#include <netinet/in.h>

// #include <algorithm>
// #include <cstdint>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <string>
// #include <random>
#include <simppl/dispatcher.h>
#include <simppl/skeleton.h>

#include <vector>

#include "../chrony/chronycandm.h"
#include "../chrony/chronyclient.h"
#include "dbusinterface.h"

ChronyDBusService::ChronyDBusService(simppl::dbus::Dispatcher &disp)
    : simppl::dbus::Skeleton<ChronyDBus>(disp, "chronyDBusServer")
{
    /// @todo improve error handling with error messages based on the chrony reply status code

    getSources >> [this]()
    {
        std::cout << ">> getSources enter" << "\n";
        const auto [sourceList, success] = chrony::client::process_cmd_sources();
        if (success) { respond_with(getSources(sourceList)); }
        else { respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "Invalid request or no response from chronyd!")); }
        std::cout << "<< getSources exit" << "\n";
    };

    addServers >> [this](const std::vector<AddServersData> &serverList)
    {
        std::cout << ">> addservers enter" << "\n";
        bool success = true;
        for (const auto &server : serverList)
        {
            std::cout << "Adding server: " << server.name << " port: " << server.port << "\n";
            success = success && chrony::client::process_cmd_add_source(server);
        }
        if (success) { respond_with(addServers()); }
        else { respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "Invalid request or no response from chronyd!")); }
        std::cout << "<< addservers exit" << "\n";
    };

    deleteServers >> [this](const std::vector<std::string> &serverList)
    {
        std::cout << ">> deleteServers enter" << "\n";
        bool success = true;
        for (const auto &server : serverList) { success = success && chrony::client::process_cmd_delete(server); }
        if (success) { respond_with(deleteServers()); }
        else { respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "Invalid request or no response from chronyd!")); }
        std::cout << "<< deleteServers exit" << "\n";
    };

    clearManualTimeList >> [this]()
    {
        std::cout << ">> clearManualTimeList enter" << "\n";
        const bool success = ::chrony::client::process_cmd_clear_manual_list();
        if (success) { respond_with(clearManualTimeList()); }
        else { respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "Invalid request or no response from chronyd!")); }
        std::cout << "<< clearManualTimeList exit" << "\n";
    };

    setManualTimeEnabled >> [this](bool enabled)
    {
        std::cout << ">> setManualTimeEnabled enter" << "\n";
        CMD_Request request;
        CMD_Reply reply;

        /// @todo if enabled==false call cmd_refresh to re-sync the time

        request.command = htons(REQ_MANUAL);
        request.data.manual.option = static_cast<std::int32_t>(htonl(enabled));

        const bool success = chrony::client::request_reply(&request, &reply, RPY_NULL, 1);
        if (success) { respond_with(setManualTimeEnabled()); }
        else { respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "Invalid request or no response from chronyd!")); }
        std::cout << "<< setManualTimeEnabled exit" << "\n";
    };

    getManualTimeList >> [this]()
    {
        std::cout << ">> getManualTimeList enter" << "\n" << "\n";
        const auto [list, success] = chrony::client::process_cmd_manual_list();
        if (success) { respond_with(getManualTimeList(list)); }
        else { respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "Invalid request or no response from chronyd!")); }
        std::cout << ">> getManualTimeList exit" << "\n";
    };

    setManualTime >> [this](const std::string &time)
    {
        std::cout << ">> setManualTime enter" << "\n";

        /// @todo remove all other manual entries?
        const bool success = chrony::client::process_cmd_settime(time);
        if (success) { respond_with(setManualTime()); }
        else { respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "Invalid or no response from chronyd!")); }
        std::cout << "<< setManualTime exit" << "\n";
    };
}
