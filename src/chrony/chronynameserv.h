#ifndef CHRONYNAMESERV_H
#define CHRONYNAMESERV_H

/*
 * original copyright
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 **********************************************************************

  =======================================================================

  Definitions for the network protocol used for command and monitoring
  of the timeserver.

  */

#include <netdb.h>

#include <cstdint>

#include "chronyutil.h"

// all content in this namespace was originally copied from chrony https://gitlab.com/chrony/chrony
namespace chrony
{
namespace nameserv
{
typedef enum
{
    DNS_Success,
    DNS_TryAgain,
    DNS_Failure
} DNS_Status;

DNS_Status DNS_Name2IPAddress(const char *name, IPAddr *ip_addrs, int max_addrs)
{
    struct addrinfo hints, *res, *ai;
    int i, result;
    int address_family = IPADDR_UNSPEC;
    IPAddr ip;

    max_addrs = std::min(max_addrs, 16);

    for (i = 0; i < max_addrs; i++) ip_addrs[i].family = IPADDR_UNSPEC;

    /* Avoid calling getaddrinfo() if the name is an IP address */
    if (::chrony::util::UTI_StringToIP(name, &ip))
    {
        if (address_family != IPADDR_UNSPEC && ip.family != address_family) return DNS_Failure;
        if (max_addrs >= 1) ip_addrs[0] = ip;
        return DNS_Success;
    }

    memset(&hints, 0, sizeof(hints));

    switch (address_family)
    {
        case IPADDR_INET4:
            hints.ai_family = AF_INET;
            break;
#ifdef FEAT_IPV6
        case IPADDR_INET6:
            hints.ai_family = AF_INET6;
            break;
#endif
        default:
            hints.ai_family = AF_UNSPEC;
    }
    hints.ai_socktype = SOCK_DGRAM;

    result = getaddrinfo(name, NULL, &hints, &res);

    if (result)
    {
#ifdef FORCE_DNSRETRY
        return DNS_TryAgain;
#else
        return result == EAI_AGAIN ? DNS_TryAgain : DNS_Failure;
#endif
    }

    for (ai = res, i = 0; i < max_addrs && ai != NULL; ai = ai->ai_next)
    {
        switch (ai->ai_family)
        {
            case AF_INET:
                if (address_family != IPADDR_UNSPEC && address_family != IPADDR_INET4) continue;
                ip_addrs[i].family = IPADDR_INET4;
                ip_addrs[i].addr.in4 = ntohl(((struct sockaddr_in *)ai->ai_addr)->sin_addr.s_addr);
                i++;
                break;
#ifdef FEAT_IPV6
            case AF_INET6:
                if (address_family != IPADDR_UNSPEC && address_family != IPADDR_INET6) continue;
                /* Don't return an address that would lose a scope ID */
                if (((struct sockaddr_in6 *)ai->ai_addr)->sin6_scope_id != 0) continue;
                ip_addrs[i].family = IPADDR_INET6;
                memcpy(&ip_addrs[i].addr.in6, &((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr.s6_addr, sizeof(ip_addrs->addr.in6));
                i++;
                break;
#endif
        }
    }

    freeaddrinfo(res);

    return !max_addrs || ip_addrs[0].family != IPADDR_UNSPEC ? DNS_Success : DNS_Failure;
}
}  // namespace nameserv
}  // namespace chrony

#endif  // CHRONYNAMESERV_H
