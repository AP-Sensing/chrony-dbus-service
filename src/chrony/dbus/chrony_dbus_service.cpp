#include "chrony/dbus/chrony_dbus_service.hpp"

#include <netinet/in.h>
#include <simppl/dispatcher.h>
#include <simppl/skeleton.h>

#include <chrono>  // NOLINT (misc-include-cleaner) False positive
#include <cstdint>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "chrony/chronycandm.h"
#include "chrony/chronyclient.h"
#include "chrony/dbus/dbus_interface.hpp"

using namespace std::chrono_literals;

namespace
{
bool checkCommandSocket()
{
    bool retVal = false;
    const std::filesystem::path commandSocketPath = "/var/run/chrony/chronyd.sock";
    for (int failCount = 0; failCount < 10; ++failCount)
    {
        if (std::filesystem::exists(commandSocketPath))
        {
            retVal = true;
            break;
        }
        std::this_thread::sleep_for(250ms);  // NOLINT (misc-include-cleaner) False positive
    }
    return retVal;
}
}  // namespace

ChronyDBusService::ChronyDBusService(simppl::dbus::Dispatcher &disp)
    : simppl::dbus::Skeleton<ChronyDBus>(disp, "chronyDBusServer")
{
    getSources >> [this]()
    {
        std::cout << ">> getSources enter" << "\n";
        if (!checkCommandSocket())
        {
            std::cerr << "!! getSources error: chronyd command socket is unavailable!\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        const auto [success, errStr, sourceList] = chrony::client::process_cmd_sources();
        if (success) { respond_with(getSources(sourceList)); }
        else
        {
            std::cerr << "!! getSources error: " << errStr.value_or("Invalid request or no response from chronyd!") << "\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        std::cout << "<< getSources exit" << "\n";
    };

    addServers >> [this](const std::vector<AddServersData> &serverList)
    {
        std::cout << ">> addservers enter" << "\n";
        if (!checkCommandSocket())
        {
            std::cerr << "!! addservers error: chronyd command socket is unavailable!\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        for (const auto &server : serverList)
        {
            std::cout << std::format("Adding server: {} port: {} flags: {}", server.name, server.port,
                                     static_cast<std::uint16_t>(server.flags))
                      << "\n";
            const auto [success, errStr] = chrony::client::process_cmd_add_source(server);
            if (!success)
            {
                std::cerr << "!! addservers error: " << errStr.value_or("Invalid request or no response from chronyd!") << "\n";
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
        if (!checkCommandSocket())
        {
            std::cerr << "!! deleteServers error: chronyd command socket is unavailable!\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        for (const auto &server : serverList)
        {
            const auto [success, errStr] = chrony::client::process_cmd_delete(server);
            if (!success)
            {
                std::cerr << "!! deleteServers error: " << errStr.value_or("Invalid request or no response from chronyd!") << "\n";
                respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                                 errStr.value_or("Invalid request or no response from chronyd!").c_str()));
                return;
            }
        }
        respond_with(deleteServers());
        std::cout << "<< deleteServers exit" << "\n";
    };

    addManualTime >>
        [this](const std::string &
                   time)  // cppcheck-suppress y2038-unsafe-call // the code will refuse to compile in cases where the y2038 problem applies
    {
        std::cout << ">> addManualTime enter" << "\n";
        if (!checkCommandSocket())
        {
            std::cerr << "!! addManualTime error: chronyd command socket is unavailable!\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        /// @todo remove all other manual entries?
        const auto [success, errStr] = chrony::client::process_cmd_settime(
            time);  // cppcheck-suppress y2038-unsafe-call // the code will refuse to compile in cases where the y2038 problem applies
        if (success) { respond_with(addManualTime()); }
        else
        {
            std::cerr << "!! addManualTime error: " << errStr.value_or("Invalid request or no response from chronyd!") << "\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        std::cout << "<< addManualTime exit" << "\n";
    };

    clearManualTimeList >> [this]()
    {
        std::cout << ">> clearManualTimeList enter" << "\n";
        if (!checkCommandSocket())
        {
            std::cerr << "!! clearManualTimeList error: chronyd command socket is unavailable!\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        const auto [success, errStr] = ::chrony::client::process_cmd_clear_manual_list();
        if (success) { respond_with(clearManualTimeList()); }
        else
        {
            std::cerr << "!! clearManualTimeList error: " << errStr.value_or("Invalid request or no response from chronyd!") << "\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        std::cout << "<< clearManualTimeList exit" << "\n";
    };

    getManualTimeList >> [this]()
    {
        std::cout << ">> getManualTimeList enter" << "\n";
        if (!checkCommandSocket())
        {
            std::cerr << "!! getManualTimeList error: chronyd command socket is unavailable!\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        const auto [success, errStr, list] = chrony::client::process_cmd_manual_list();
        if (success) { respond_with(getManualTimeList(list)); }
        else
        {
            std::cerr << "!! getManualTimeList error: " << errStr.value_or("Invalid request or no response from chronyd!") << "\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        std::cout << ">> getManualTimeList exit" << "\n";
    };

    setManualTimeEnabled >> [this](bool enabled)
    {
        std::cout << ">> setManualTimeEnabled enter" << "\n";
        if (!checkCommandSocket())
        {
            std::cerr << "!! setManualTimeEnabled error: chronyd command socket is unavailable!\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        CMD_Request request;
        CMD_Reply reply;

        /// @todo if enabled==false call cmd_refresh to re-sync the time?

        request.command = htons(REQ_MANUAL);
        request.data.manual.option = static_cast<std::int32_t>(htonl(enabled));

        const bool success = chrony::client::request_reply(&request, &reply, RPY_NULL, 1);
        if (success) { respond_with(setManualTimeEnabled()); }
        else
        {
            std::cerr << "!! setManualTimeEnabled error: " << std::format("Error: chronyd returned status: {}", reply.status) << "\n";
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             std::format("Error: chronyd returned status: {}", reply.status).c_str()));
        }
        std::cout << "<< setManualTimeEnabled exit" << "\n";
    };
}
