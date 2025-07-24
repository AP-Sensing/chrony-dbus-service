#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H
#include <simppl/any.h>
#include <simppl/interface.h>
#include <simppl/struct.h>

#include <map>

using namespace simppl::dbus;

// see man `chrony.conf` and `man chronyc`

struct ChronySourceData
{
    enum class SourceMode : std::uint16_t
    {
        Server = 0,
        Peer = 1,
        ReferenceClock = 2
    };

    enum class SelectionState : std::uint16_t
    {
        Selected = 0,
        Unselectable = 1,
        FalseTicker = 2,
        TooMuchJitter = 3,
        NotSelected = 4,
        SelectableCombined = 5,
    };

    typedef make_serializer<std::string, std::int16_t, std::uint16_t, SelectionState, SourceMode, std::uint32_t>::type serializer_type;

    std::string ipAddress;
    std::int16_t pollratePow2;
    std::uint16_t stratum;
    SelectionState selectionState;
    SourceMode sourceMode;
    std::uint32_t secondsSinceLastsample;
};

struct AddServersData
{
    enum class ServerFlags : std::uint16_t
    {
        Online = 0x1,
        AutoOffline = 0x2,
        IBurst = 0x4,
        Prefer = 0x8,
        NoSelect = 0x10,
        Trusted = 0x20,
        Required = 0x40,
        Interleaved = 0x80,
        Burst = 0x100,
        NTSEnabled = 0x200,
        Copy = 0x400,
        MonoRoot = 0x800,
        NetCorrection = 0x1000,
        IPv4 = 0x2000,
        IPv6 = 0x4000
    };

    typedef make_serializer<std::string, std::uint16_t, std::uint16_t, std::uint32_t, std::uint32_t, ServerFlags>::type serializer_type;

    std::string name;
    std::uint16_t port;
    std::uint16_t nts_port;
    std::uint32_t ntsKeyId;
    std::uint32_t ntsCertificateSet;
    ServerFlags flags;
};

namespace org::freedesktop
{
INTERFACE(ChronyDBus)
{
    Method<out<std::vector<ChronySourceData>>, _throw<simppl::dbus::Error>> getSources;
    Method<in<std::vector<AddServersData>>, _throw<simppl::dbus::Error>> addServers;
    Method<in<std::vector<std::string>>, _throw<simppl::dbus::Error>> deleteServers;
    Method<in<bool>, _throw<simppl::dbus::Error>> setManualTimeEnabled;
    Method<out<std::vector<std::string>>, _throw<simppl::dbus::Error>> getManualTimeList;
    // format is "yyyy-mm-dd HH:MM:SS"
    Method<in<std::string>, _throw<simppl::dbus::Error>> setManualTime;

    // constructor
    ChronyDBus()
        : INIT(getSources),
          INIT(addServers),
          INIT(deleteServers),
          INIT(setManualTimeEnabled),
          INIT(getManualTimeList),
          INIT(setManualTime)
    {
    }
};
}  // namespace org::freedesktop

#endif  // DBUSINTERFACE_H
