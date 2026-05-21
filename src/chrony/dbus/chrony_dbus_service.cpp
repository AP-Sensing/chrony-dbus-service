#include "chrony/dbus/chrony_dbus_service.hpp"

#include <chrony/candm.hpp>
#include <chrony/client.hpp>
#include <netinet/in.h>
#include <simppl/dispatcher.h>
#include <simppl/skeleton.h>
#include <simppl/stub.h>

#include <chrono>  // NOLINT (misc-include-cleaner) False positive
#include <cstdint>
#include <filesystem>
#include <format>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "chrony/dbus/dbus_interface.hpp"
#include "chrony/dbus/polkit_interface.hpp"

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
        std::cout << ">> getSources enter" << std::endl;
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.getSources")) { return; }

        if (!checkCommandSocket())
        {
            std::cerr << "!! getSources error: chronyd command socket is unavailable!" << std::endl
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        const auto [success, errStr, sourceList] = chrony::client::process_cmd_sources();
        if (success) { respond_with(getSources(sourceList)); }
        else
        {
            std::cerr << "!! getSources error: " << errStr.value_or("Invalid request or no response from chronyd!") << std::endl;
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        std::cout << "<< getSources exit" << std::endl;
    };

    getTrackingData >> [this]()
    {
        std::cout << ">> getTrackingData enter" << std::endl;
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.getTrackingData")) { return; }
        if (!checkCommandSocket())
        {
            std::cerr << "!! getTrackingData error: chronyd command socket is unavailable!" << std::endl;
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        const auto [success, errStr, trackingData] = chrony::client::process_cmd_tracking();
        if (success) { respond_with(getTrackingData(trackingData)); }
        else
        {
            std::cerr << "!! getTrackingData error: " << errStr.value_or("Invalid request or no response from chronyd!") << std::endl;
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        std::cout << "<< getTrackingData exit" << std::endl;
    };

    addServers >> [this](const std::vector<AddServersData> &serverList)
    {
        std::cout << ">> addservers enter" << std::endl;
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.addServers")) { return; }
        if (!checkCommandSocket())
        {
            std::cerr << "!! addservers error: chronyd command socket is unavailable!" << std::endl;
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        for (const auto &server : serverList)
        {
            std::cout << std::format("Adding server: {} port: {} flags: {}", server.name, server.port,
                                     static_cast<std::uint16_t>(server.flags))
                      << std::endl;
            const auto [success, errStr] = chrony::client::process_cmd_add_source(server);
            if (!success)
            {
                std::cerr << "!! addservers error: " << errStr.value_or("Invalid request or no response from chronyd!") << std::endl;
                respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                                 errStr.value_or("Invalid request or no response from chronyd!").c_str()));
                return;
            }
        }
        respond_with(addServers());
        std::cout << "<< addservers exit" << std::endl;
    };

    deleteServers >> [this](const std::vector<std::string> &serverList)
    {
        std::cout << ">> deleteServers enter" << std::endl;
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.deleteServers")) { return; }
        if (!checkCommandSocket())
        {
            std::cerr << "!! deleteServers error: chronyd command socket is unavailable!" << std::endl;
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        for (const auto &server : serverList)
        {
            const auto [success, errStr] = chrony::client::process_cmd_delete(server);
            if (!success)
            {
                std::cerr << "!! deleteServers error: " << errStr.value_or("Invalid request or no response from chronyd!") << std::endl;
                respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                                 errStr.value_or("Invalid request or no response from chronyd!").c_str()));
                return;
            }
        }
        respond_with(deleteServers());
        std::cout << "<< deleteServers exit" << std::endl;
    };

    addManualTime >>
        [this](const std::string &
                   time)  // cppcheck-suppress y2038-unsafe-call // the code will refuse to compile in cases where the y2038 problem applies
    {
        std::cout << ">> addManualTime enter" << std::endl;
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.addManualTime")) { return; }
        if (!checkCommandSocket())
        {
            std::cerr << "!! addManualTime error: chronyd command socket is unavailable!" << std::endl
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        /// @todo remove all other manual entries?
        const auto [success, errStr] = chrony::client::process_cmd_settime(
            time);  // cppcheck-suppress y2038-unsafe-call // the code will refuse to compile in cases where the y2038 problem applies
        if (success) { respond_with(addManualTime()); }
        else
        {
            std::cerr << "!! addManualTime error: " << errStr.value_or("Invalid request or no response from chronyd!") << std::endl;
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        std::cout << "<< addManualTime exit" << std::endl;
    };

    clearManualTimeList >> [this]()
    {
        std::cout << ">> clearManualTimeList enter" << std::endl;
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.clearManualTimeList")) { return; }
        if (!checkCommandSocket())
        {
            std::cerr << "!! clearManualTimeList error: chronyd command socket is unavailable!" << std::endl
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        const auto [success, errStr] = ::chrony::client::process_cmd_clear_manual_list();
        if (success) { respond_with(clearManualTimeList()); }
        else
        {
            std::cerr << "!! clearManualTimeList error: " << errStr.value_or("Invalid request or no response from chronyd!") << std::endl;
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        std::cout << "<< clearManualTimeList exit" << std::endl;
    };

    getManualTimeList >> [this]()
    {
        std::cout << ">> getManualTimeList enter" << std::endl;
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.getManualTimeList")) { return; }
        if (!checkCommandSocket())
        {
            std::cerr << "!! getManualTimeList error: chronyd command socket is unavailable!" << std::endl
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        const auto [success, errStr, list] = chrony::client::process_cmd_manual_list();
        if (success) { respond_with(getManualTimeList(list)); }
        else
        {
            std::cerr << "!! getManualTimeList error: " << errStr.value_or("Invalid request or no response from chronyd!") << std::endl;
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        std::cout << ">> getManualTimeList exit" << std::endl;
    };

    setManualTimeEnabled >> [this](bool enabled)
    {
        std::cout << ">> setManualTimeEnabled enter" << std::endl;
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.setManualTimeEnabled")) { return; }
        if (!checkCommandSocket())
        {
            std::cerr << "!! setManualTimeEnabled error: chronyd command socket is unavailable!" << std::endl
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
            std::cerr << "!! setManualTimeEnabled error: " << std::format("Error: chronyd returned status: {}", reply.status) << std::endl;
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             std::format("Error: chronyd returned status: {}", reply.status).c_str()));
        }
        std::cout << "<< setManualTimeEnabled exit" << std::endl;
    };
    makeStep >> [this]()
    {
        std::cout << ">> makeStep enter" << std::endl;
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.makeStep")) { return; }
        if (!checkCommandSocket())
        {
            std::cerr << "!! makeStep error: chronyd command socket is unavailable!" << std::endl
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        CMD_Request request;
        CMD_Reply reply;

        request.command = htons(REQ_MAKESTEP);
        const bool success = chrony::client::request_reply(&request, &reply, RPY_NULL, 1);
        if (success) { respond_with(makeStep()); }
        else
        {
            std::cerr << "!! makeStep error: " << std::format("Error: chronyd returned status: {}", reply.status) << std::endl;
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             std::format("Error: chronyd returned status: {}", reply.status).c_str()));
        }
        std::cout << "<< makeStep exit" << std::endl;
    };
}

bool ChronyDBusService::checkPolkitPermissions(const std::string &actionId)
{
    const std::string busName = dbus_message_get_sender(current_request().msg_);
    std::cout << ">> checkPolkitPermissions enter" << std::endl;
    std::cout << "busName = " << busName << std::endl;
    std::cout << "actionId = " << actionId << std::endl;
    simppl::dbus::Dispatcher dispatch("bus:system");
    simppl::dbus::Stub<org::freedesktop::PolicyKit1::Authority> polkitStub(dispatch, "org.freedesktop.PolicyKit1",
                                                                           "/org/freedesktop/PolicyKit1/Authority");

    const std::string cancellationId{std::format("{}::{}", busName, actionId)};
    const org::freedesktop::PolicyKit1::Subject subject{.subject_kind = "system-bus-name", .subject_details = {{"name", busName}}};
    const std::map<std::string, std::string> details{{"polkit.message", "chrony-dbus-service polkit auth"}};
    try
    {
        const org::freedesktop::PolicyKit1::AuthorizationResult authResult =
            polkitStub.CheckAuthorization(subject, actionId, details, org::freedesktop::PolicyKit1::CheckAuthorizationFlags::None,
                                          cancellationId);
        std::cout << "authResult.is_authorized = " << authResult.is_authorized << std::endl;
        std::cout << "<< checkPolkitPermissions exit" << std::endl;
        if (!authResult.is_authorized)
        {
            std::cerr << "!! Polkit returned is_authorized = false for sender: " << busName << std::endl;
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "org.freedesktop.PolicyKit1.CheckAuthorization failed!"));
            return false;
        }
        return true;
    }
    catch (simppl::dbus::Error &e)
    {
        std::cout << "!! DBus error: " << e.what() << std::endl;
        std::cerr << "!! Polkit returned is_authorized = false for sender: " << busName << std::endl;
        respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "org.freedesktop.PolicyKit1.CheckAuthorization failed!"));
        return false;
    }
}
