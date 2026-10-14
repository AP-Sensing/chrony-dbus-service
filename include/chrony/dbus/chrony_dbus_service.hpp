#pragma once

#include <simppl/dispatcher.h>
#include <simppl/skeleton.h>
#include <iosfwd>

#include "chrony/dbus/dbus_interface.hpp"

class ChronyDBusService : simppl::dbus::Skeleton<org::freedesktop::ChronyDBus>
{
   public:
    explicit ChronyDBusService(simppl::dbus::Dispatcher &disp, const bool debugLoggingIsEnabled = false);
    ~ChronyDBusService() override = default;

   private:
    bool checkPolkitPermissions(const std::string &actionId);
    const bool debugLoggingIsEnabled;

    template<typename... Args>
    static void logTo(std::ostream& output, Args&&... args);

    template<typename... Args>
    void debugLog(Args&&... args) const;

    template<typename... Args>
    static void errorLog(Args&&... args);
};
