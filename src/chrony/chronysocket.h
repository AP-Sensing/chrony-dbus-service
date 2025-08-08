#ifndef CHRONYSOCKET_H
#define CHRONYSOCKET_H

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

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "chronyaddressing.h"
#include "chronyutil.h"

// all content in this namespace was originally copied from chrony https://gitlab.com/chrony/chrony
namespace chrony
{
namespace socket
{
/* Flags for opening sockets */
#define SCK_FLAG_BLOCK 1
#define SCK_FLAG_BROADCAST 2
#define SCK_FLAG_RX_DEST_ADDR 4
#define SCK_FLAG_ALL_PERMISSIONS 8
#define SCK_FLAG_PRIV_BIND 16

/* Flags for receiving and sending messages */
#define SCK_FLAG_MSG_ERRQUEUE 1
#define SCK_FLAG_MSG_DESCRIPTOR 2

typedef enum
{
    SCK_ADDR_UNSPEC = 0,
    SCK_ADDR_IP,
    SCK_ADDR_UNIX
} SCK_AddressType;

union sockaddr_all
{
    sockaddr_un un;
    sockaddr sa;
};

struct Address
{
    ::chrony::socket::SCK_AddressType type;
    union
    {
        chrony::addressing::IPSockAddr ip;
        std::string_view path;
    } addr;
};

static int get_recv_flags(int flags)
{
    int recv_flags = 0;

    if (flags & SCK_FLAG_MSG_ERRQUEUE)
    {
#ifdef MSG_ERRQUEUE
        recv_flags |= MSG_ERRQUEUE;
#else
        assert(0);
#endif
    }

    return recv_flags;
}
static socklen_t set_unix_sockaddr(struct sockaddr_un *sun, const char *addr)
{
    size_t len = strlen(addr);

    if (len + 1 > sizeof(sun->sun_path))
    {
        std::cout << std::format("Unix socket path {} too long", addr) << "\n";
        return 0;
    }

    memset(sun, 0, sizeof(*sun));
    sun->sun_family = AF_UNIX;
    memcpy(sun->sun_path, addr, len);

    return offsetof(struct sockaddr_un, sun_path) + len + 1;
}

static int bind_unix_address(int sock_fd, const char *addr, int flags)
{
    union ::chrony::socket::sockaddr_all saddr;
    socklen_t saddr_len;

    saddr_len = set_unix_sockaddr(&saddr.un, addr);
    if (saddr_len == 0) return 0;

    if (unlink(addr) < 0) std::cout << std::format("Could not remove {} : {}", addr, strerror(errno)) << "\n";

    /* PRV_BindSocket() doesn't support Unix sockets yet */
    if (bind(sock_fd, &saddr.sa, saddr_len) < 0)
    {
        std::cout << std::format("Could not bind Unix socket to {} : {}", addr, strerror(errno)) << "\n";
        return 0;
    }

    /* Allow access to everyone with access to the directory if requested */
    if (flags & SCK_FLAG_ALL_PERMISSIONS && chmod(addr, 0666) < 0)
    {
        std::cout << std::format("Could not change permissions of {} : {}", addr, strerror(errno)) << "\n";
        return 0;
    }

    return 1;
}

int SCK_IPSockAddrToSockaddr(chrony::addressing::IPSockAddr *ip_sa, struct sockaddr *sa, int sa_length)
{
    switch (ip_sa->ip_addr.family)
    {
        case IPADDR_INET4:
            if (sa_length < (int)sizeof(struct sockaddr_in)) return 0;
            memset(sa, 0, sizeof(struct sockaddr_in));
            sa->sa_family = AF_INET;
            ((struct sockaddr_in *)sa)->sin_addr.s_addr = htonl(ip_sa->ip_addr.addr.in4);
            ((struct sockaddr_in *)sa)->sin_port = htons(ip_sa->port);
#ifdef SIN6_LEN
            ((struct sockaddr_in *)sa)->sin_len = sizeof(struct sockaddr_in);
#endif
            return sizeof(struct sockaddr_in);
#ifdef FEAT_IPV6
        case IPADDR_INET6:
            if (sa_length < (int)sizeof(struct sockaddr_in6)) return 0;
            memset(sa, 0, sizeof(struct sockaddr_in6));
            sa->sa_family = AF_INET6;
            memcpy(&((struct sockaddr_in6 *)sa)->sin6_addr.s6_addr, ip_sa->ip_addr.addr.in6,
                   sizeof(((struct sockaddr_in6 *)sa)->sin6_addr.s6_addr));
            ((struct sockaddr_in6 *)sa)->sin6_port = htons(ip_sa->port);
#ifdef SIN6_LEN
            ((struct sockaddr_in6 *)sa)->sin6_len = sizeof(struct sockaddr_in6);
#endif
            return sizeof(struct sockaddr_in6);
#endif
        default:
            if (sa_length < (int)sizeof(struct sockaddr)) return 0;
            memset(sa, 0, sizeof(struct sockaddr));
            sa->sa_family = AF_UNSPEC;
            return 0;
    }
}
}  // namespace socket
}  // namespace chrony
#endif  // CHRONYSOCKET_H
