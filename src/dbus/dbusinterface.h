/* This file is licensed under the MIT-0 license:
Copyright 2025 Samuel Stirtzel <s.stirtzel@googlemail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H
// required for serializer_type to work
#include <simppl/any.h>
#include <simppl/interface.h>
#include <simppl/struct.h>

using namespace simppl::dbus;

// see man `chrony.conf` and `man chronyc`

struct ChronySourceData
{
    enum class SourceMode : std::uint16_t
    {
        Server = 0,         ///< Source is an NTP server
        Peer = 1,           ///< Used if chrony is configured to relay time to other peers
        ReferenceClock = 2  ///< Source is a reference clock e.g. a GPS or an IEEE 1588 PTP hardware clock with PHC driver
    };

    enum class SelectionState : std::uint16_t
    {
        Selected = 0,            ///< Source is used to sync time
        Unselectable = 1,        ///< Source is configured to be never used unless manually specified
        FalseTicker = 2,         ///< Source is probably not accurate
        TooMuchJitter = 3,       ///< Source has too much measurement variance
        NotSelected = 4,         ///< Source is available for selection but currently not selected
        SelectableCombined = 5,  ///< Source is used in combination with other sources to sync time
    };

    typedef make_serializer<std::string, std::int16_t, std::uint16_t, SelectionState, SourceMode, std::uint32_t>::type serializer_type;

    /// can be either a hostname / IP address, reference clock name or internal identifier (e.g. ID#123456789)
    std::string name;
    /// pow(2, X) seconds, e.g. pollratePow2 = -1 -> 0.5 seconds
    std::int16_t pollratePow2;
    /// stratum is the distance to a reference clock in the measurement chain, stratum 1 is directly connected to a reference clock
    std::uint16_t stratum;
    SelectionState selectionState;
    SourceMode sourceMode;
    /// last synchronization time, this can be 10 minutes or more depending on the /etc/chrony.conf
    std::uint32_t secondsSinceLastsample;
};

struct AddServersData
{
    enum ServerFlags : std::uint16_t
    {
        None = 0x0,              ///< just for completeness
        Online = 0x1,            ///< default for added sources
        AutoOffline = 0x2,       ///< sets the server as offline if it is unreachable e.g. for unstable connections
        IBurst = 0x4,            ///< start syncing sooner by using a burst of requests
        Prefer = 0x8,            ///< sources with this flag will be selected before ones without it
        NoSelect = 0x10,         ///< source will never be selected if it has this flag
        Trusted = 0x20,          ///< used to force sync to a source with wrong time
        Required = 0x40,         ///< for sources that need to be selected
        Interleaved = 0x80,      ///< NTP interleaved mode
        Burst = 0x100,           ///< syncs in bursts but limited to the pollrate
        NTSEnabled = 0x200,      ///< Network Time Security
        Copy = 0x400,            ///< for multiple instances of chrony on the same host
        MonoRoot = 0x800,        ///< NTP extension field mono root
        NetCorrection = 0x1000,  ///< NTP extension field net correction
        IPv4 = 0x2000,           ///< added source is IPv4 reachable
        IPv6 = 0x4000            ///< added source is IPv6 reachable
    };

    typedef make_serializer<std::string, std::uint16_t, std::uint16_t, std::uint32_t, std::uint32_t, ServerFlags>::type serializer_type;

    /// can be either a hostname or IP address
    std::string name;
    /// NTP default port is 123
    std::uint16_t port = 123;
    /// NTS default port is 4460
    std::uint16_t nts_port = 4460;
    /// this id needs to be the same as the one provided in /etc/chrony.conf or a /etc/chrony.d/ conf fragment
    std::uint32_t ntsKeyId = 0;
    /// defaults to 0, set 0 also includes the system CAs by default
    std::uint32_t ntsCertificateSet = 0;
    /// config flags determine how chronyd communicates with this server
    ServerFlags flags = ServerFlags::Online;
};

namespace org::freedesktop
{
INTERFACE(ChronyDBus)
{
    /// Returns a list of NTP servers that chrony uses to sync time
    Method<out<std::vector<ChronySourceData>>, _throw<simppl::dbus::Error>> getSources;
    /// Adds a list of NTP servers for time sync
    Method<in<std::vector<AddServersData>>, _throw<simppl::dbus::Error>> addServers;
    /// Deletes a list of servers with the given address string
    Method<in<std::vector<std::string>>, _throw<simppl::dbus::Error>> deleteServers;
    /// Enables or disables manual time control (if enabled and a manual time is set, then that time is used as a reference instead of NTP
    /// servers)
    /// Adds a manual time entry with the format "yyyy-mm-dd HH:MM:SS"
    Method<in<std::string>, _throw<simppl::dbus::Error>> addManualTime;
    Method<_throw<simppl::dbus::Error>> clearManualTimeList;
    /// Lists all manual time entries with the format "yyyy-mm-dd HH:MM:SS"
    Method<out<std::vector<std::string>>, _throw<simppl::dbus::Error>> getManualTimeList;
    /// @note Disabling manual time currently doesn't delete manual time entries
    Method<in<bool>, _throw<simppl::dbus::Error>> setManualTimeEnabled;

    // constructor
    ChronyDBus()
        : INIT(getSources),
          INIT(addServers),
          INIT(deleteServers),
          INIT(addManualTime),
          INIT(clearManualTimeList),
          INIT(getManualTimeList),
          INIT(setManualTimeEnabled)
    {
    }
};
}  // namespace org::freedesktop

#endif  // DBUSINTERFACE_H
