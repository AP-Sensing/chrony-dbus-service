#include "chronydbusservice.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdint>
#include <random>
#include <netdb.h>
#include <resolv.h>
#include <ctime>

#include <simppl/dispatcher.h>
#include <simppl/skeleton.h>

#include "dbusinterface.h"
#include "../chrony/chronycandm.h"
#include "../chrony/chronyutil.h"
#include "../chrony/chronypktlength.h"
#include "../chrony/chronysocket.h"
#include "../chrony/chronyclient.h"

class ChronyDBusServer : simppl::dbus::Skeleton<org::freedesktop::ChronyDBus>
{
public:
  ChronyDBusServer(simppl::dbus::Dispatcher& disp)
    : simppl::dbus::Skeleton<ChronyDBus>(disp, "chronyDBusServer")
  {
    getSources >> [this]()
    {
      std::cout << ">> getSources enter" << std::endl;
      const auto [sourceList, success] = chrony::client::process_cmd_sources();
      if(success)
      {
        respond_with(getSources(sourceList));
      }
      else
      {
        respond_with(simppl::dbus::Error("Chronyd communication error", "Invalid request or no response from chronyd!"));
      }
      std::cout << "<< getSources exit" << std::endl;
    };

    addServers >> [this](const std::vector<AddServersData> &serverList)
    {
      std::cout << ">> addservers enter" << std::endl;
      bool success = true;
      for(const auto &server : serverList)
      {
        std::cout << "Adding server: " << server.name << std::endl;
        success = success && chrony::client::process_cmd_add_source(server);

      }
      if(success)
      {
        respond_with(addServers());
      }
      else
      {
        respond_with(simppl::dbus::Error("Chronyd communication error", "Invalid request or no response from chronyd!"));
      }
      std::cout << "<< addservers exit" << std::endl;
    };

    deleteServers >> [this](const std::vector<std::string> &serverList)
    {
      std::cout << ">> deleteServers enter" << std::endl;
      bool success = true;
      for(const auto &server : serverList)
      {
        success = success && chrony::client::process_cmd_delete(server);
      }
      if(success)
      {
        respond_with(deleteServers());
      }
      else
      {
        respond_with(simppl::dbus::Error("Chronyd communication error", "Invalid request or no response from chronyd!"));
      }
      std::cout << "<< deleteServers exit" << std::endl;
    };

    setManualTimeEnabled >> [this](bool enabled)
    {
      std::cout << ">> setManualTimeEnabled enter" << std::endl;
      CMD_Request request;
      CMD_Reply reply;


      /// @todo if enabled==false remove all manual entries and call cmd_refresh

      request.command = htons(REQ_MANUAL);
      request.data.manual.option = htonl(enabled);

      bool success = chrony::client::request_reply(&request, &reply, RPY_NULL, 1);
      if(success)
      {
        respond_with(setManualTimeEnabled());
      }
      else
      {
        respond_with(simppl::dbus::Error("Chronyd communication error", "Invalid request or no response from chronyd!"));
      }
      std::cout << "<< setManualTimeEnabled exit" << std::endl;
    };

    getManualTimeList >> [this]()
    {
      std::cout << ">> getManualTimeList enter" << std::endl << std::endl;
      const auto [list, success] = chrony::client::process_cmd_manual_list();
      if(success)
      {
        respond_with(getManualTimeList(list));
      }
      else
      {
        respond_with(simppl::dbus::Error("Chronyd communication error", "Invalid request or no response from chronyd!"));
      }
      std::cout << ">> getManualTimeList exit" << std::endl;
    };

    setManualTime >> [this](const std::string& time)
    {
      std::cout << ">> setManualTime enter" << std::endl;

      /// @todo remove all other manual entries
      bool success = chrony::client::process_cmd_settime(time);
      if(success)
      {
        respond_with(setManualTime());
      }
      else
      {
        respond_with(simppl::dbus::Error("Chronyd communication error", "Invalid or no response from chronyd!"));
      }
      std::cout << "<< setManualTime exit" << std::endl;
    };
  }
};

ChronyDBusService::ChronyDBusService()
{
  simppl::dbus::Dispatcher disp("bus:session");
  ChronyDBusServer instance(disp);

  disp.run();
}
