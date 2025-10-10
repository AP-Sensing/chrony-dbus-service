#pragma once

#include <simppl/dispatcher.h>
#include <simppl/skeleton.h>

#include "chrony/dbus/dbus_interface.hpp"

class ChronyDBusService : simppl::dbus::Skeleton<org::freedesktop::ChronyDBus>
{
   public:
    explicit ChronyDBusService(simppl::dbus::Dispatcher &disp);
    ~ChronyDBusService() override = default;
};
