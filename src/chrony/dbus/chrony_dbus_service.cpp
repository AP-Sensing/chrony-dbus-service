#include "chrony/dbus/chrony_dbus_service.hpp"

#include <chrony/candm.hpp>
#include <chrony/client.hpp>
#include <chrony/logging.hpp>
#include <dbus/dbus.h>
#include <netinet/in.h>
#include <simppl/dispatcher.h>
#include <simppl/skeleton.h>
#include <simppl/stub.h>

#include <chrono>  // NOLINT (misc-include-cleaner) False positive
#include <cstdint>
#include <filesystem>
#include <format>
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

ChronyDBusService::ChronyDBusService(simppl::dbus::Dispatcher &disp, const bool debugLoggingIsEnabled)
    : simppl::dbus::Skeleton<ChronyDBus>(disp, "chronyDBusServer")
{
    // Control debug logging.
    logging::debugLoggingIsEnabled = debugLoggingIsEnabled;

    // Set up callbacks.
    getSources >> [this]()
    {
        logging::debugLog(">> getSources enter");
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.getSources")) { return; }

        if (!checkCommandSocket())
        {
            logging::errorLog("!! getSources error: chronyd command socket is unavailable!");
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        const auto [success, errStr, sourceList] = chrony::client::process_cmd_sources();
        if (success) { respond_with(getSources(sourceList)); }
        else
        {
            logging::errorLog("!! getSources error: ", errStr.value_or("Invalid request or no response from chronyd!"));
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        logging::debugLog("<< getSources exit");
    };

    getTrackingData >> [this]()
    {
        logging::debugLog(">> getTrackingData enter");
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.getTrackingData")) { return; }
        if (!checkCommandSocket())
        {
            logging::errorLog("!! getTrackingData error: chronyd command socket is unavailable!");
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        const auto [success, errStr, trackingData] = chrony::client::process_cmd_tracking();
        if (success) { respond_with(getTrackingData(trackingData)); }
        else
        {
            logging::errorLog("!! getTrackingData error: ", errStr.value_or("Invalid request or no response from chronyd!"));
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        logging::debugLog("<< getTrackingData exit");
    };

    addServers >> [this](const std::vector<AddServersData> &serverList)
    {
        logging::debugLog(">> addservers enter");
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.addServers")) { return; }
        if (!checkCommandSocket())
        {
            logging::errorLog("!! addservers error: chronyd command socket is unavailable!");
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        for (const auto &server : serverList)
        {
            logging::debugLog(std::format("Adding server: {} port: {} flags: {}", server.name, server.port,
                                     static_cast<std::uint16_t>(server.flags))
                     );
            const auto [success, errStr] = chrony::client::process_cmd_add_source(server);
            if (!success)
            {
                logging::errorLog("!! addservers error: ", errStr.value_or("Invalid request or no response from chronyd!"));
                respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                                 errStr.value_or("Invalid request or no response from chronyd!").c_str()));
                return;
            }
        }
        respond_with(addServers());
        logging::debugLog("<< addservers exit");
    };

    deleteServers >> [this](const std::vector<std::string> &serverList)
    {
        logging::debugLog(">> deleteServers enter");
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.deleteServers")) { return; }
        if (!checkCommandSocket())
        {
            logging::errorLog("!! deleteServers error: chronyd command socket is unavailable!");
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        for (const auto &server : serverList)
        {
            const auto [success, errStr] = chrony::client::process_cmd_delete(server);
            if (!success)
            {
                logging::errorLog("!! deleteServers error: ", errStr.value_or("Invalid request or no response from chronyd!"));
                respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                                 errStr.value_or("Invalid request or no response from chronyd!").c_str()));
                return;
            }
        }
        respond_with(deleteServers());
        logging::debugLog("<< deleteServers exit");
    };

    addManualTime >>
        [this](const std::string &
                   time)  // cppcheck-suppress y2038-unsafe-call // the code will refuse to compile in cases where the y2038 problem applies
    {
        logging::debugLog(">> addManualTime enter");
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.addManualTime")) { return; }
        if (!checkCommandSocket())
        {
            logging::errorLog("!! addManualTime error: chronyd command socket is unavailable!");
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        /// @todo remove all other manual entries?
        const auto [success, errStr] = chrony::client::process_cmd_settime(
            time);  // cppcheck-suppress y2038-unsafe-call // the code will refuse to compile in cases where the y2038 problem applies
        if (success) { respond_with(addManualTime()); }
        else
        {
            logging::errorLog("!! addManualTime error: ", errStr.value_or("Invalid request or no response from chronyd!"));
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        logging::debugLog("<< addManualTime exit");
    };

    clearManualTimeList >> [this]()
    {
        logging::debugLog(">> clearManualTimeList enter");
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.clearManualTimeList")) { return; }
        if (!checkCommandSocket())
        {
            logging::errorLog("!! clearManualTimeList error: chronyd command socket is unavailable!");
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        const auto [success, errStr] = ::chrony::client::process_cmd_clear_manual_list();
        if (success) { respond_with(clearManualTimeList()); }
        else
        {
            logging::errorLog("!! clearManualTimeList error: ", errStr.value_or("Invalid request or no response from chronyd!"));
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        logging::debugLog("<< clearManualTimeList exit");
    };

    getManualTimeList >> [this]()
    {
        logging::debugLog(">> getManualTimeList enter");
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.getManualTimeList")) { return; }
        if (!checkCommandSocket())
        {
            logging::errorLog("!! getManualTimeList error: chronyd command socket is unavailable!");
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        const auto [success, errStr, list] = chrony::client::process_cmd_manual_list();
        if (success) { respond_with(getManualTimeList(list)); }
        else
        {
            logging::errorLog("!! getManualTimeList error: ", errStr.value_or("Invalid request or no response from chronyd!"));
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             errStr.value_or("Invalid request or no response from chronyd!").c_str()));
        }
        logging::debugLog(">> getManualTimeList exit");
    };

    setManualTimeEnabled >> [this](bool enabled)
    {
        logging::debugLog(">> setManualTimeEnabled enter");
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.setManualTimeEnabled")) { return; }
        if (!checkCommandSocket())
        {
            logging::errorLog("!! setManualTimeEnabled error: chronyd command socket is unavailable!");
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
            logging::errorLog("!! setManualTimeEnabled error: ", std::format("Error: chronyd returned status: {}", reply.status));
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             std::format("Error: chronyd returned status: {}", reply.status).c_str()));
        }
        logging::debugLog("<< setManualTimeEnabled exit");
    };
    makeStep >> [this]()
    {
        logging::debugLog(">> makeStep enter");
        if (!checkPolkitPermissions("org.freedesktop.ChronyDBus.chronyDBusServer.makeStep")) { return; }
        if (!checkCommandSocket())
        {
            logging::errorLog("!! makeStep error: chronyd command socket is unavailable!");
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "The chronyd command socket is unavailable!"));
        }
        CMD_Request request;
        CMD_Reply reply;

        request.command = htons(REQ_MAKESTEP);
        const bool success = chrony::client::request_reply(&request, &reply, RPY_NULL, 1);
        if (success) { respond_with(makeStep()); }
        else
        {
            logging::errorLog("!! makeStep error: ", std::format("Error: chronyd returned status: {}", reply.status));
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed",
                                             std::format("Error: chronyd returned status: {}", reply.status).c_str()));
        }
        logging::debugLog("<< makeStep exit");
    };
}

bool ChronyDBusService::checkPolkitPermissions(const std::string &actionId)
{
    // NOLINTNEXTLINE(misc-include-cleaner) We are including the recommended header <dbus/dbus.h>
    const std::string busName = dbus_message_get_sender(current_request().msg_);
    logging::debugLog(">> checkPolkitPermissions enter");
    logging::debugLog("busName = ", busName);
    logging::debugLog("actionId = ", actionId);
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
        logging::debugLog("authResult.is_authorized = ", authResult.is_authorized);
        logging::debugLog("<< checkPolkitPermissions exit");
        if (!authResult.is_authorized)
        {
            logging::errorLog("!! Polkit returned is_authorized = false for sender: ", busName);
            respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "org.freedesktop.PolicyKit1.CheckAuthorization failed!"));
            return false;
        }
        return true;
    }
    catch (simppl::dbus::Error &e)
    {
        logging::debugLog("!! DBus error: ", e.what());
        logging::errorLog("!! Polkit returned is_authorized = false for sender: ", busName);
        respond_with(simppl::dbus::Error("org.freedesktop.DBus.Error.Failed", "org.freedesktop.PolicyKit1.CheckAuthorization failed!"));
        return false;
    }
}
