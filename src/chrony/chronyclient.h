#ifndef CHRONYCLIENT_H
#define CHRONYCLIENT_H

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

#include <iostream>
#include <random>

#include "chronynameserv.h"
#include "chronypktlength.h"
#include "chronysocket.h"
#include "chronyutil.h"

// for struct ChronySourceData;
#include "../dbus/dbusinterface.h"

// all content in this namespace was originally copied from chrony https://gitlab.com/chrony/chrony
namespace chrony
{
#define SRC_DEFAULT_PORT 123
#define SRC_DEFAULT_MINPOLL 6
#define SRC_DEFAULT_MAXPOLL 10
#define SRC_DEFAULT_PRESEND_MINPOLL 100
#define SRC_DEFAULT_MAXDELAY 3.0
#define SRC_DEFAULT_MAXDELAYRATIO 0.0
#define SRC_DEFAULT_MAXDELAYDEVRATIO 10.0
#define SRC_DEFAULT_MINSTRATUM 0
#define SRC_DEFAULT_POLLTARGET 8
#define SRC_DEFAULT_MAXSOURCES 4
#define SRC_DEFAULT_MINSAMPLES (-1)
#define SRC_DEFAULT_MAXSAMPLES (-1)
#define SRC_DEFAULT_ASYMMETRY 1.0
#define SRC_DEFAULT_NTSPORT 4460
#define SRC_DEFAULT_CERTSET 0
#define INACTIVE_AUTHKEY 0
namespace client
{
static std::vector<::chrony::socket::Address> server_addresses{
    ::chrony::socket::Address{.type = ::chrony::socket::SCK_ADDR_UNIX, .addr = {.path = "/var/run/chrony/chronyd.sock"}}};

static int sock_fd = -1;
static int proto_version = PROTO_VERSION_NUMBER;

static int open_unix_socket2(const std::string &af_unix_address, const std::string &local_addr = "")
{
    assert(af_unix_address.size() < sizeof(sockaddr_un::sun_path));
    assert(local_addr.size() < sizeof(sockaddr_un::sun_path));

    const int sock_flags = SCK_FLAG_ALL_PERMISSIONS;
    const int sock_opt_value = 1;
    sock_fd = ::socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd < 0) { std::cerr << "Error opening socket file descriptor!" << "\n"; }
    union ::chrony::socket::sockaddr_all sock_address;
    sock_address.un.sun_family = AF_UNIX;
    strlcpy(sock_address.un.sun_path, af_unix_address.c_str(), af_unix_address.size() + 1);  // strlcopy copies size-1

    /* Bind the socket if a local address was specified */
    if (!local_addr.empty() && !::chrony::socket::bind_unix_address(sock_fd, local_addr.c_str(), sock_flags))
    {
        std::cerr << "Error binding local socket" << "\n";
    }

    errno = 0;
    if (setsockopt(sock_fd, SOL_SOCKET, sock_flags, &sock_opt_value, sizeof(sock_opt_value)) != 0)
    {
        std::cerr << "Error setting socket options: " << strerror(errno);
    }

    errno = 0;
    if (connect(sock_fd, &sock_address.sa, sizeof(sock_address)) != 0)
    {
        std::cerr << "Error opening socket " << sock_address.un.sun_path << ": " << strerror(errno);
        return false;
    }
    return sock_fd;
}

static int open_socket(struct ::chrony::socket::Address *addr)
{
    char *dir, *local_addr;
    size_t local_addr_len;

    switch (addr->type)
    {
        case socket::SCK_ADDR_UNIX:
            /* Construct path of our socket.  Use the same directory as the server
                 socket and include our process ID to allow multiple chronyc instances
                 running at the same time. */

            {
                const std::string addressPath{addr->addr.path};
                dir = (char *)malloc(addressPath.size());
                strlcpy(dir, addressPath.c_str(), addressPath.size() + 1);
                dir = ::chrony::util::UTI_PathToDir(dir);
                local_addr_len = strlen(dir) + 50;
                local_addr = (char *)malloc(local_addr_len);

                snprintf(local_addr, local_addr_len, "%s/chronyc.%d.sock", dir, (int)getpid());

                // sock_fd = open_unix_socket(addressPath.c_str(), local_addr, SOCK_DGRAM,  SCK_FLAG_ALL_PERMISSIONS);
                sock_fd = open_unix_socket2(addressPath, local_addr);
                free(dir);
                free(local_addr);
            }
            break;
        default:
            assert(0);
    }

    if (sock_fd < 0) return 0;

    return 1;
}

/* ================================================== */

static void close_io(void)
{
    if (sock_fd < 0) return;

    close(sock_fd);
    sock_fd = -1;
}

/* ================================================== */

static int open_io(void)
{
    static unsigned int address_index = 0;

    /* If a socket is already opened, close it and try the next address */
    if (sock_fd >= 0)
    {
        close_io();
        address_index++;
    }

    /* Find an address for which a socket can be opened and connected */
    for (::chrony::socket::Address address : server_addresses)
    {
        const std::string addressPath{address.addr.path};
        if (open_socket(&address))
        {
            std::cout << "open_io(): open_socket success!" << "\n";
            return 1;
        }
        else { std::cerr << "open_io(): open_socket failed!" << "\n"; }

        close_io();
    }

    /* Start from the first address if called again */
    address_index = 0;

    return 0;
}

/* This is the core protocol module.  Complete particular fields in
   the outgoing packet, send it, wait for a response, handle retries,
   etc.  Returns a Boolean indicating whether the protocol was
   successful or not.*/

static int submit_request(CMD_Request *request, CMD_Reply *reply)
{
    std::cout << "submit_request(): enter" << "\n";
    int quit = 0;
    int max_retries = 2;
    int initial_timeout = 1000;
    int select_status;
    int recv_status;
    int read_length;
    int command_length;
    int padding_length;
    struct timespec ts_now, ts_start;
    struct timeval tv;
    int n_attempts, new_attempt;
    double timeout;
    fd_set rdfd;

    request->pkt_type = PKT_TYPE_CMD_REQUEST;
    request->res1 = 0;
    request->res2 = 0;
    request->pad1 = 0;
    request->pad2 = 0;

    n_attempts = 0;
    new_attempt = 1;

    do {
        if (gettimeofday(&tv, NULL)) return 0;

        if (new_attempt)
        {
            new_attempt = 0;

            std::cout << "submit_request(): n_attempts/max_retries: " << n_attempts << "/" << max_retries << "\n";
            if (n_attempts > max_retries) return 0;

            ::chrony::util::UTI_TimevalToTimespec(&tv, &ts_start);

            std::random_device rDev;
            std::mt19937 randomGenerator(rDev());

            request->sequence = htons(
                randomGenerator());  /// @todo don't know if it is a security risk to use pseudorandom numbers here instead of /dev/urandom
            request->attempt = htons(n_attempts);
            request->version = proto_version;
            command_length = ::chrony::pktlength::PKL_CommandLength(request);
            padding_length = ::chrony::pktlength::PKL_CommandPaddingLength(request);
            assert(command_length > 0 && command_length > padding_length);

            std::cout << "submit_request(): request request->sequence: " << request->sequence << "\n";
            std::cout << "submit_request(): request proto_version: " << proto_version << "\n";
            std::cout << "submit_request(): request command_length: " << command_length << "\n";
            std::cout << "submit_request(): request padding_length: " << padding_length << "\n";

            n_attempts++;

            /* Zero the padding to not send any uninitialized data */
            memset(((char *)request) + command_length - padding_length, 0, padding_length);

            if (sock_fd < 0)
            {
                std::cerr << "submit_request(): No socket to send request" << "\n";
                return 0;
            }

            if (command_length < 0)
            {
                std::cerr << "Invalid length " << command_length << "\n";
                return -1;
            }

            int r = send(sock_fd, (void *)request, command_length, 0);

            if (r < 0)
            {
                std::cerr << "Could not send data fd=" << sock_fd << " len=" << command_length << " : " << strerror(errno) << "\n";
                return r;
            }

            std::cout << "Sent data fd=" << sock_fd << " len=" << r << "\n";
        }

        ::chrony::util::UTI_TimevalToTimespec(&tv, &ts_now);

        /* Check if the clock wasn't stepped back */
        if (::chrony::util::UTI_CompareTimespecs(&ts_now, &ts_start) < 0) ts_start = ts_now;

        timeout = initial_timeout / 1000.0 * (1U << (n_attempts - 1)) - ::chrony::util::UTI_DiffTimespecsToDouble(&ts_now, &ts_start);
        std::cerr << "submit_request(): Timeout " << timeout << " seconds" << "\n";

        /* Avoid calling select() with an invalid timeout */
        if (timeout <= 0.0)
        {
            new_attempt = 1;
            continue;
        }

        ::chrony::util::UTI_DoubleToTimeval(timeout, &tv);

        FD_ZERO(&rdfd);
        FD_SET(sock_fd, &rdfd);

        if (quit) return 0;

        select_status = select(sock_fd + 1, &rdfd, NULL, NULL, &tv);

        if (select_status < 0)
        {
            std::cerr << "select failed : " << strerror(errno) << "\n";
            return 0;
        }
        else if (select_status == 0)
        {
            std::cerr << "submit_request(): select_status == 0!" << "\n";
            /* Timeout must have elapsed, try a resend? */
            new_attempt = 1;
        }
        else
        {
            recv_status = recv(sock_fd, reply, sizeof(*reply), chrony::socket::get_recv_flags(0));

            if (recv_status < 0)
            {
                std::cerr << "submit_request(): recv_status < 0!" << "\n";
                new_attempt = 1;
            }
            else
            {
                read_length = recv_status;

                /* Check if the header is valid */
                if (static_cast<unsigned long int>(read_length) < offsetof(CMD_Reply, data)
                    || (reply->version != proto_version
                        && !(reply->version >= PROTO_VERSION_MISMATCH_COMPAT_CLIENT && ntohs(reply->status) == STT_BADPKTVERSION))
                    || reply->pkt_type != PKT_TYPE_CMD_REPLY || reply->res1 != 0 || reply->res2 != 0 || reply->command != request->command
                    || reply->sequence != request->sequence)
                {
                    std::cerr << "submit_request(): Invalid reply" << "\n";
                    continue;
                }

#if PROTO_VERSION_NUMBER == 6
                /* Protocol version 5 is similar to 6 except there is no padding.
                   If a version 5 reply with STT_BADPKTVERSION is received,
                   switch our version and try again. */
                if (proto_version == PROTO_VERSION_NUMBER && reply->version == PROTO_VERSION_NUMBER - 1)
                {
                    proto_version = PROTO_VERSION_NUMBER - 1;
                    n_attempts--;
                    new_attempt = 1;
                    continue;
                }
#else
#error unknown compatibility with PROTO_VERSION - 1
#endif

                /* Check that the packet contains all data it is supposed to have.
                   Unknown responses will always pass this test as their expected
                   length is zero. */
                if (read_length < ::chrony::pktlength::PKL_ReplyLength(reply))
                {
                    std::cerr << "submit_request(): Reply too short" << "\n";
                    new_attempt = 1;
                    continue;
                }

                /* Good packet received, print out results */
                std::cerr << "Reply cmd=" << ntohs(reply->command) << " reply=" << ntohs(reply->command) << " stat=" << ntohs(reply->status)
                          << "\n";
                break;
            }
        }
    } while (1);

    return 1;
}

static int request_reply(CMD_Request *request, CMD_Reply *reply, int requested_reply, int verbose)
{
    int status;

    while (!submit_request(request, reply))
    {
        std::cout << "request_reply(): trying to submit request" << "\n";
        /* Try connecting to other addresses before giving up */
        if (open_io()) continue;
        printf("506 Cannot talk to daemon\n");
        return 0;
    }

    status = ntohs(reply->status);
    std::cout << "request_reply(): request status: " << status << "\n";

    if (verbose || status != STT_SUCCESS)
    {
        switch (status)
        {
            case STT_SUCCESS:
                printf("200 OK");
                break;
            case STT_ACCESSALLOWED:
                printf("208 Access allowed");
                break;
            case STT_ACCESSDENIED:
                printf("209 Access denied");
                break;
            case STT_FAILED:
                printf("500 Failure");
                break;
            case STT_UNAUTH:
                printf("501 Not authorised");
                break;
            case STT_INVALID:
                printf("502 Invalid command");
                break;
            case STT_NOSUCHSOURCE:
                printf("503 No such source");
                break;
            case STT_INVALIDTS:
                printf("504 Duplicate or stale logon detected");
                break;
            case STT_NOTENABLED:
                printf("505 Facility not enabled in daemon");
                break;
            case STT_BADSUBNET:
                printf("507 Bad subnet");
                break;
            case STT_NOHOSTACCESS:
                printf("510 No command access from this host");
                break;
            case STT_SOURCEALREADYKNOWN:
                printf("511 Source already present");
                break;
            case STT_TOOMANYSOURCES:
                printf("512 Too many sources present");
                break;
            case STT_NORTC:
                printf("513 RTC driver not running");
                break;
            case STT_BADRTCFILE:
                printf("514 Can't write RTC parameters");
                break;
            case STT_INVALIDAF:
                printf("515 Invalid address family");
                break;
            case STT_BADSAMPLE:
                printf("516 Sample index out of range");
                break;
            case STT_BADPKTVERSION:
                printf("517 Protocol version mismatch");
                break;
            case STT_BADPKTLENGTH:
                printf("518 Packet length mismatch");
                break;
            case STT_INACTIVE:
                printf("519 Client logging is not active in the daemon");
                break;
            case STT_INVALIDNAME:
                printf("521 Invalid name");
                break;
            default:
                printf("520 Got unexpected error from daemon");
        }
        printf("\n");
    }

    if (status != STT_SUCCESS && status != STT_ACCESSALLOWED && status != STT_ACCESSDENIED) { return 0; }

    if (ntohs(reply->reply) != requested_reply)
    {
        printf("508 Bad reply from daemon\n");
        return 0;
    }

    /* Make sure an unknown response was not requested */
    assert(::chrony::pktlength::PKL_ReplyLength(reply));

    return 1;
}

static std::tuple<std::vector<ChronySourceData>, bool> process_cmd_sources()
{
    std::vector<ChronySourceData> retVal;
    std::cout << "process_cmd_sources(): enter" << "\n";
    CMD_Request request;
    CMD_Reply reply;
    IPAddr ip_addr;
    uint32_t i, mode, n_sources;

    request.command = htons(REQ_N_SOURCES);
    if (!request_reply(&request, &reply, RPY_N_SOURCES, 0)) return {retVal, false};

    n_sources = ntohl(reply.data.n_sources.n_sources);
    std::cout << "process_cmd_sources(): n_sources: " << n_sources << "\n";

    for (i = 0; i < n_sources; i++)
    {
        request.command = htons(REQ_SOURCE_DATA);
        request.data.source_data.index = htonl(i);
        if (!request_reply(&request, &reply, RPY_SOURCE_DATA, 0)) return {retVal, false};

        ::chrony::util::UTI_IPNetworkToHost(&reply.data.source_data.ip_addr, &ip_addr);
        std::cout << "process_cmd_sources(): source i: " << i << " ip_addr.addr.in4: " << ::chrony::util::UTI_IPToString(&ip_addr) << "\n";
        ChronySourceData data;
        data.ipAddress = ::chrony::util::UTI_IPToString(&ip_addr);
        mode = ntohs(reply.data.source_data.mode);
        data.sourceMode = static_cast<ChronySourceData::SourceMode>(mode);
        data.selectionState = static_cast<ChronySourceData::SelectionState>(ntohs(reply.data.source_data.state));
        data.pollratePow2 = ntohs(reply.data.source_data.poll);
        data.stratum = ntohs(reply.data.source_data.stratum);
        data.secondsSinceLastsample = ntohs(reply.data.source_data.since_sample);

        retVal.push_back(data);
        if (ip_addr.family == IPADDR_ID) continue;
    }

    return {retVal, true};
}

static int process_cmd_add_source(const AddServersData &data)
{
    int result = 0;
    CMD_Request request;
    CMD_Reply reply;

    // only currently support source servers
    request.command = htons(REQ_ADD_SOURCE);

    // set params not exported to the default values provided by include/chrony/cmdparse.c
    request.data.ntp_source.type = htonl(REQ_ADDSRC_SERVER);
    assert(data.name.size() < sizeof(request.data.ntp_source.name));
    strlcpy((char *)request.data.ntp_source.name, data.name.c_str(), data.name.size() + 1);  // strlcopy copies size-1

    request.data.ntp_source.port = htonl(data.port);
    request.data.ntp_source.minpoll = htonl(SRC_DEFAULT_MINPOLL);
    request.data.ntp_source.maxpoll = htonl(SRC_DEFAULT_MAXPOLL);
    request.data.ntp_source.presend_minpoll = htonl(SRC_DEFAULT_PRESEND_MINPOLL);
    request.data.ntp_source.min_stratum = htonl(SRC_DEFAULT_MINSTRATUM);
    request.data.ntp_source.poll_target = htonl(SRC_DEFAULT_POLLTARGET);
    request.data.ntp_source.version = htonl(0);
    request.data.ntp_source.max_sources = htonl(SRC_DEFAULT_MAXSOURCES);
    request.data.ntp_source.min_samples = htonl(SRC_DEFAULT_MINSAMPLES);
    request.data.ntp_source.max_samples = htonl(SRC_DEFAULT_MAXSAMPLES);
    request.data.ntp_source.authkey = htonl(data.ntsKeyId);
    request.data.ntp_source.nts_port = htonl(data.nts_port);
    request.data.ntp_source.max_delay = ::chrony::util::UTI_FloatHostToNetwork(SRC_DEFAULT_MAXDELAY);
    request.data.ntp_source.max_delay_ratio = ::chrony::util::UTI_FloatHostToNetwork(SRC_DEFAULT_MAXDELAYRATIO);
    request.data.ntp_source.max_delay_dev_ratio = ::chrony::util::UTI_FloatHostToNetwork(SRC_DEFAULT_MAXDELAYDEVRATIO);
    request.data.ntp_source.min_delay = ::chrony::util::UTI_FloatHostToNetwork(0.0);
    request.data.ntp_source.asymmetry = ::chrony::util::UTI_FloatHostToNetwork(SRC_DEFAULT_ASYMMETRY);
    request.data.ntp_source.offset = ::chrony::util::UTI_FloatHostToNetwork(0.0);
    request.data.ntp_source.flags = htonl(static_cast<uint32_t>(data.flags));
    request.data.ntp_source.filter_length = htonl(0);
    request.data.ntp_source.cert_set = htonl(data.ntsCertificateSet);
    request.data.ntp_source.max_delay_quant = ::chrony::util::UTI_FloatHostToNetwork(0.0);
    memset(request.data.ntp_source.reserved, 0, sizeof(request.data.ntp_source.reserved));

    result = ::chrony::client::request_reply(&request, &reply, RPY_NULL, 1);

    return result;
}

static int process_cmd_delete(const std::string &serverAddress)
{
    int result = 0;
    IPAddr address;
    CMD_Request request;
    CMD_Reply reply;
    if (::chrony::util::UTI_StringToIP(serverAddress.c_str(), &address) != 0)
    {
        if (::chrony::nameserv::DNS_Name2IPAddress(serverAddress.c_str(), &address, 1) != ::chrony::nameserv::DNS_Success)
        {
            std::cerr << "Could not parse serverAddress for process_cmd_delete" << "\n";
            result = 0;
            return result;
        }
    }

    request.command = htons(REQ_DEL_SOURCE);
    ::chrony::util::UTI_IPHostToNetwork(&address, &request.data.del_source.ip_addr);
    result = ::chrony::client::request_reply(&request, &reply, RPY_NULL, 1);
    return result;
}

static std::tuple<std::vector<std::string>, bool> process_cmd_manual_list()
{
    CMD_Request request;
    CMD_Reply reply;
    uint32_t i, n_samples;
    RPY_ManualListSample *sample;
    struct timespec when;
    std::vector<std::string> result;

    request.command = htons(REQ_MANUAL_LIST);
    if (!request_reply(&request, &reply, RPY_MANUAL_LIST2, 0)) return {result, false};

    n_samples = ntohl(reply.data.manual_list.n_samples);

    for (i = 0; i < n_samples && i < MAX_MANUAL_LIST_SAMPLES; i++)
    {
        sample = &reply.data.manual_list.samples[i];
        ::chrony::util::UTI_TimespecNetworkToHost(&sample->when, &when);
        result.push_back(::chrony::util::UTI_TimeToLogForm(when.tv_sec));
    }

    return {result, true};
}

static int process_cmd_settime(const std::string &newTimeString)
{
    struct timespec ts;
    time_t new_time;
    CMD_Request request;
    CMD_Reply reply;
    double dfreq_ppm, new_afreq_ppm;
    double offset;

    std::tm timeTemp = {};
    std::istringstream timeStream(newTimeString);
    timeStream >> std::get_time(&timeTemp, "%Y-%m-%d %H:%M:%S");
    std::cout << "Setting time to: " << timeTemp.tm_hour << ":" << timeTemp.tm_min << ":" << timeTemp.tm_sec << "\n";
    new_time = mktime(&timeTemp);

    if (new_time == -1) { printf("510 - Could not parse date string\n"); }
    else
    {
        ts.tv_sec = new_time;
        ts.tv_nsec = 0;
        ::chrony::util::UTI_TimespecHostToNetwork(&ts, &request.data.settime.ts);
        request.command = htons(REQ_SETTIME);
        if (request_reply(&request, &reply, RPY_MANUAL_TIMESTAMP2, 1))
        {
            offset = ::chrony::util::UTI_FloatNetworkToHost(reply.data.manual_timestamp.offset);
            dfreq_ppm = ::chrony::util::UTI_FloatNetworkToHost(reply.data.manual_timestamp.dfreq_ppm);
            new_afreq_ppm = ::chrony::util::UTI_FloatNetworkToHost(reply.data.manual_timestamp.new_afreq_ppm);
            printf("Clock was %.2f seconds fast.  Frequency change = %.2fppm, new frequency = %.2fppm\n", offset, dfreq_ppm, new_afreq_ppm);
            return 1;
        }
    }
    return 0;
}
}  // namespace client
}  // namespace chrony
#endif  // CHRONYCLIENT_H
