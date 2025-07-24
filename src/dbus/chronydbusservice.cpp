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
    getSources >> [this]()
    {
        std::cout << ">> getSources enter" << "\n";
        const auto [success, errStr, sourceList] = chrony::client::process_cmd_sources();
        if (success) { respond_with(getSources(sourceList)); }
        else
        {
            std::cout << "!! getSources error: " << errStr.value_or("Invalid request or no response from chronyd!") << "\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        std::cout << "<< getSources exit" << "\n";
    };

    addServers >> [this](const std::vector<AddServersData> &serverList)
    {
        std::cout << ">> addservers enter" << "\n";
        for (const auto &server : serverList)
        {
            std::cout << "Adding server: " << server.name << " port: " << server.port << "\n";
            const auto [success, errStr] = chrony::client::process_cmd_add_source(server);
            if (!success)
            {
                std::cout << "!! addservers error: " << errStr.value_or("Invalid request or no response from chronyd!") << "\n";
                respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                                 errStr.value_or("Invalid request or no response from chronyd!").c_str()));
                return;
            }
        }
        respond_with(addServers());
        std::cout << "<< addservers exit" << "\n";
    };

    deleteServers >> [this](const std::vector<std::string> &serverList)
    {
        std::cout << ">> deleteServers enter" << "\n";
        for (const auto &server : serverList)
        {
            const auto [success, errStr] = chrony::client::process_cmd_delete(server);
            if (!success)
            {
                std::cout << "!! deleteServers error: " << errStr.value_or("Invalid request or no response from chronyd!") << "\n";
                respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                                 errStr.value_or("Invalid request or no response from chronyd!").c_str()));
                return;
            }
        }
        respond_with(deleteServers());
        std::cout << "<< deleteServers exit" << "\n";
    };

    clearManualTimeList >> [this]()
    {
        std::cout << ">> clearManualTimeList enter" << "\n";
        const auto [success, errStr] = ::chrony::client::process_cmd_clear_manual_list();
        if (success) { respond_with(clearManualTimeList()); }
        else
        {
            std::cout << "!! clearManualTimeList error: " << errStr.value_or("Invalid request or no response from chronyd!") << "\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
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
        else
        {
            std::cout << "!! setManualTimeEnabled error: " << std::format("Error: chronyd returned status: {}", reply.status) << "\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             std::format("Error: chronyd returned status: {}", reply.status).c_str()));
        }
        std::cout << "<< setManualTimeEnabled exit" << "\n";
    };

    getManualTimeList >> [this]()
    {
        std::cout << ">> getManualTimeList enter" << "\n" << "\n";
        const auto [success, errStr, list] = chrony::client::process_cmd_manual_list();
        if (success) { respond_with(getManualTimeList(list)); }
        else
        {
            std::cout << "!! getManualTimeList error: " << errStr.value_or("Invalid request or no response from chronyd!") << "\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        std::cout << ">> getManualTimeList exit" << "\n";
    };

    setManualTime >> [this](const std::string &time)
    {
        std::cout << ">> setManualTime enter" << "\n";

        /// @todo remove all other manual entries?
        const auto [success, errStr] = chrony::client::process_cmd_settime(time);
        if (success) { respond_with(setManualTime()); }
        else
        {
            std::cout << "!! setManualTime error: " << errStr.value_or("Invalid request or no response from chronyd!") << "\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        std::cout << "<< setManualTime exit" << "\n";
    };
}
