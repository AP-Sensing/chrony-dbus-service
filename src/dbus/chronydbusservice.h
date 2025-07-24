#ifndef CHRONYDBUSSERVICE_H
#define CHRONYDBUSSERVICE_H

#include <simppl/dispatcher.h>
#include <simppl/skeleton.h>

#include "dbusinterface.h"

class ChronyDBusService : simppl::dbus::Skeleton<org::freedesktop::ChronyDBus>
{
   public:
    ChronyDBusService(simppl::dbus::Dispatcher &disp);
};

#endif  // CHRONYDBUSSERVICE_H
