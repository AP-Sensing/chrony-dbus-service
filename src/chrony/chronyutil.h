#ifndef CHRONYUTIL_H
#define CHRONYUTIL_H

#include <algorithm>
#include <cstdint>

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

#include <arpa/inet.h>
#include <sys/time.h>

#include <cinttypes>
#include <string>

// all content in this namespace was originally copied from chrony https://gitlab.com/chrony/chrony
namespace chrony
{
namespace util
{
void UTI_TimevalToTimespec(const struct timeval *tv, struct timespec *ts)
{
    ts->tv_sec = tv->tv_sec;
    ts->tv_nsec = 1000 * tv->tv_usec;
}

int UTI_CompareTimespecs(const struct timespec *a, const struct timespec *b)
{
    if (a->tv_sec < b->tv_sec) return -1;
    if (a->tv_sec > b->tv_sec) return 1;
    if (a->tv_nsec < b->tv_nsec) return -1;
    if (a->tv_nsec > b->tv_nsec) return 1;
    return 0;
}

/* Calculate result = a - b and return as a double */
double UTI_DiffTimespecsToDouble(const struct timespec *a, const struct timespec *b)
{
    return ((double)a->tv_sec - (double)b->tv_sec) + 1.0e-9 * (a->tv_nsec - b->tv_nsec);
}

void UTI_TimespecNetworkToHost(const Timespec *src, struct timespec *dest)
{
    uint32_t sec_low, nsec;
#ifdef HAVE_LONG_TIME_T
    uint32_t sec_high;
#endif

    sec_low = ntohl(src->tv_sec_low);
#ifdef HAVE_LONG_TIME_T
    sec_high = ntohl(src->tv_sec_high);
    if (sec_high == TV_NOHIGHSEC) sec_high = 0;

    dest->tv_sec = (uint64_t)sec_high << 32 | sec_low;
#else
    dest->tv_sec = sec_low;
#endif

    nsec = ntohl(src->tv_nsec);
    dest->tv_nsec = std::min(nsec, 999999999U);
}

/* ================================================== */

void UTI_TimespecHostToNetwork(const struct timespec *src, Timespec *dest)
{
    dest->tv_nsec = htonl(src->tv_nsec);
#ifdef HAVE_LONG_TIME_T
    dest->tv_sec_high = htonl((uint64_t)src->tv_sec >> 32);
#else
    dest->tv_sec_high = htonl(TV_NOHIGHSEC);
#endif
    dest->tv_sec_low = htonl(src->tv_sec);
}

std::string UTI_TimeToLogForm(time_t t)
{
    struct tm *stm;
    char result[64];

    stm = gmtime(&t);

    if (stm)
        strftime(result, 64, "%Y-%m-%d %H:%M:%S", stm);
    else
        snprintf(result, 64, "INVALID    INVALID ");

    return std::string(result);
}

void UTI_NormaliseTimeval(struct timeval *x)
{
    /* Reduce tv_usec to within +-1000000 of zero. JGH */
    if ((x->tv_usec >= 1000000) || (x->tv_usec <= -1000000))
    {
        x->tv_sec += x->tv_usec / 1000000;
        x->tv_usec = x->tv_usec % 1000000;
    }

    /* Make tv_usec positive. JGH */
    if (x->tv_usec < 0)
    {
        --x->tv_sec;
        x->tv_usec += 1000000;
    }
}

void UTI_DoubleToTimeval(double a, struct timeval *b)
{
    double frac_part;

    b->tv_sec = a;
    frac_part = 1.0e6 * (a - b->tv_sec);
    b->tv_usec = round(frac_part);
    ::chrony::util::UTI_NormaliseTimeval(b);
}

void UTI_IPNetworkToHost(const IPAddr *src, IPAddr *dest)
{
    dest->family = ntohs(src->family);
    dest->_pad = 0;

    switch (dest->family)
    {
        case IPADDR_INET4:
            dest->addr.in4 = ntohl(src->addr.in4);
            break;
        case IPADDR_INET6:
            memcpy(dest->addr.in6, src->addr.in6, sizeof(dest->addr.in6));
            break;
        case IPADDR_ID:
            dest->addr.id = ntohl(src->addr.id);
            break;
        default:
            dest->family = IPADDR_UNSPEC;
    }
}
char *UTI_PathToDir(char *path)
{
    char *dir, *slash;
    size_t dir_len;

    slash = strrchr(path, '/');

    if (!slash) return strdup(".");

    if (slash == path) return strdup("/");

    dir_len = slash - path;

    dir = (char *)malloc(dir_len + 1);
    memcpy(dir, path, dir_len);
    dir[dir_len] = '\0';

    return dir;
}
std::string UTI_IPToString(const IPAddr *addr)
{
    unsigned long a, b, c, d, ip;
    std::vector<uint8_t> ip6(16);
    std::string result;

    switch (addr->family)
    {
        case IPADDR_UNSPEC:
            result = "[UNSPEC]";
            break;
        case IPADDR_INET4:
            ip = addr->addr.in4;
            a = (ip >> 24) & 0xff;
            b = (ip >> 16) & 0xff;
            c = (ip >> 8) & 0xff;
            d = (ip >> 0) & 0xff;
            result = std::format("{}.{}.{}.{}", a, b, c, d);
            break;
        case IPADDR_INET6:
            ip6 = {addr->addr.in6, addr->addr.in6 + sizeof(addr->addr.in6)};
#ifdef FEAT_IPV6
            inet_ntop(AF_INET6, ip6, result, BUFFER_LENGTH);
#else
            {
                std::vector<std::string> ip6_vec;
                for (a = 0; a < 8; a++) ip6_vec.push_back(std::format("{:04x}", (uint16_t)(ip6[2 * a] << 8 | ip6[2 * a + 1])));
                result = std::accumulate(ip6_vec.begin(), ip6_vec.end(), std::string(":"));
            }
#endif
            break;
        case IPADDR_ID:
            result = std::format("ID#{:x}", addr->addr.id);
            break;
        default:
            result = "[UNKNOWN]";
    }
    return result;
}

int UTI_StringToIP(const char *addr, IPAddr *ip)
{
    struct in_addr in4;
#ifdef FEAT_IPV6
    struct in6_addr in6;
#endif

    if (inet_pton(AF_INET, addr, &in4) > 0)
    {
        ip->family = IPADDR_INET4;
        ip->_pad = 0;
        ip->addr.in4 = ntohl(in4.s_addr);
        return 1;
    }

#ifdef FEAT_IPV6
    if (inet_pton(AF_INET6, addr, &in6) > 0)
    {
        ip->family = IPADDR_INET6;
        ip->_pad = 0;
        memcpy(ip->addr.in6, in6.s6_addr, sizeof(ip->addr.in6));
        return 1;
    }
#endif

    return 0;
}
void UTI_IPHostToNetwork(const IPAddr *src, IPAddr *dest)
{
    /* Don't send uninitialized bytes over network */
    memset(dest, 0, sizeof(IPAddr));

    dest->family = htons(src->family);

    switch (src->family)
    {
        case IPADDR_INET4:
            dest->addr.in4 = htonl(src->addr.in4);
            break;
        case IPADDR_INET6:
            memcpy(dest->addr.in6, src->addr.in6, sizeof(dest->addr.in6));
            break;
        case IPADDR_ID:
            dest->addr.id = htonl(src->addr.id);
            break;
        default:
            dest->family = htons(IPADDR_UNSPEC);
    }
}

#define FLOAT_EXP_BITS 7
#define FLOAT_EXP_MIN (-(1 << (FLOAT_EXP_BITS - 1)))
#define FLOAT_EXP_MAX (-FLOAT_EXP_MIN - 1)
#define FLOAT_COEF_BITS ((int)sizeof(int32_t) * 8 - FLOAT_EXP_BITS)
#define FLOAT_COEF_MIN (-(1 << (FLOAT_COEF_BITS - 1)))
#define FLOAT_COEF_MAX (-FLOAT_COEF_MIN - 1)

Float UTI_FloatHostToNetwork(double x)
{
    int32_t exp, coef, neg;
    Float f;

    if (x < 0.0)
    {
        x = -x;
        neg = 1;
    }
    else if (x >= 0.0) { neg = 0; }
    else
    {
        /* Save NaN as zero */
        x = 0.0;
        neg = 0;
    }

    if (x < 1.0e-100) { exp = coef = 0; }
    else if (x > 1.0e100)
    {
        exp = FLOAT_EXP_MAX;
        coef = FLOAT_COEF_MAX + neg;
    }
    else
    {
        exp = log(x) / log(2) + 1;
        coef = x * pow(2.0, -exp + FLOAT_COEF_BITS) + 0.5;

        assert(coef > 0);

        /* we may need to shift up to two bits down */
        while (coef > FLOAT_COEF_MAX + neg)
        {
            coef >>= 1;
            exp++;
        }

        if (exp > FLOAT_EXP_MAX)
        {
            /* overflow */
            exp = FLOAT_EXP_MAX;
            coef = FLOAT_COEF_MAX + neg;
        }
        else if (exp < FLOAT_EXP_MIN)
        {
            /* underflow */
            if (exp + FLOAT_COEF_BITS >= FLOAT_EXP_MIN)
            {
                coef >>= FLOAT_EXP_MIN - exp;
                exp = FLOAT_EXP_MIN;
            }
            else { exp = coef = 0; }
        }
    }

    /* negate back */
    if (neg) coef = (uint32_t)-coef << FLOAT_EXP_BITS >> FLOAT_EXP_BITS;

    f.f = htonl((uint32_t)exp << FLOAT_COEF_BITS | coef);
    return f;
}
double UTI_FloatNetworkToHost(Float f)
{
    int32_t exp, coef;
    uint32_t x;

    x = ntohl(f.f);

    exp = x >> FLOAT_COEF_BITS;
    if (exp >= 1 << (FLOAT_EXP_BITS - 1)) exp -= 1 << FLOAT_EXP_BITS;
    exp -= FLOAT_COEF_BITS;

    coef = x % (1U << FLOAT_COEF_BITS);
    if (coef >= 1 << (FLOAT_COEF_BITS - 1)) coef -= 1 << FLOAT_COEF_BITS;

    return coef * pow(2.0, exp);
}

int UTI_StringToIdIP(const char *addr, IPAddr *ip)
{
    if (sscanf(addr, "ID#%" SCNu32, &ip->addr.id) == 1)
    {
        ip->family = IPADDR_ID;
        ip->_pad = 0;
        return 1;
    }

    return 0;
}
}  // namespace util
}  // namespace chrony

#endif  // CHRONYUTIL_H
