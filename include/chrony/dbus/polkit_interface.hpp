#pragma once

#include <simppl/any.h>
#include <simppl/interface.h>
#include <simppl/struct.h>

#include <cstdint>
#include <map>
#include <string>

namespace org::freedesktop
{
INTERFACE(DBus)  // NOLINT(altera-struct-pack-align) Can't fix since it is inside simppl
{
    /// Returns the connection credentials
    /// documentation at https://dbus.freedesktop.org/doc/dbus-specification.html
    /// method: org.freedesktop.DBus.GetConnectionCredentials
    Method<simppl::dbus::in<std::string>,                                // Unique bus name
           simppl::dbus::out<std::map<std::string, simppl::dbus::Any>>,  // Credentials
           simppl::dbus::_throw<simppl::dbus::Error>>
        GetConnectionCredentials;

    // constructor
    DBus()
        : INIT(GetConnectionCredentials)
    {
    }
};

namespace PolicyKit1
{
enum CheckAuthorizationFlags : std::uint32_t
{
    None = 0x00000000,                 ///< No flags set.
    AllowUserInteraction = 0x00000001  ///< Can obtain the authorization through authentication via authentication agent
};

struct AuthorizationResult
{
    using serializer_type = simppl::dbus::make_serializer<bool, bool, std::map<std::string, std::string>>::type;

    bool is_authorized;  ///< true if authentication succeeded
    bool is_challenge;   ///< no suitable authentication agent available or AllowUserInteraction was not set
    /// possible keys:
    /// polkit.temporary_authorization_id
    /// polkit.retains_authorization_after_challenge
    /// polkit.dismissed
    std::map<std::string, std::string> details;
} __attribute__((aligned(64)));

struct Subject
{
    using serializer_type = simppl::dbus::make_serializer<std::string, std::map<std::string, simppl::dbus::Any>>::type;
    /// unix-process (value type)keys: (uint32)pid, (uint64)start-time
    /// unix-session (value type)keys: (string)session-id
    /// system-bus-name (value type)keys: (string)name
    std::string subject_kind;
    std::map<std::string, simppl::dbus::Any> subject_details;
} __attribute__((aligned(64)));

/// @note this is an interface in an interface, use the second parameter of the simppl::dbus::Stub
/// @example simppl::dbus::Stub<org::freedesktop::PolicyKit1::Authority> polkitStub(d, "org.freedesktop.PolicyKit1",
/// "/org/freedesktop/PolicyKit1/Authority");
INTERFACE(Authority)  // NOLINT(altera-struct-pack-align) Can't fix since it is inside simppl
{
    /// Returns the authorization result
    /// documentation at
    /// https://www.freedesktop.org/software/polkit/docs/latest/eggdbus-interface-org.freedesktop.PolicyKit1.Authority.html#eggdbus-method-org.freedesktop.PolicyKit1.Authority.CheckAuthorization
    Method<simppl::dbus::in<Subject>,                             // <arg type="(sa{sv})" name="subject" direction="in">
           simppl::dbus::in<std::string>,                         // <arg type="s" name="action_id" direction="in">
           simppl::dbus::in<std::map<std::string, std::string>>,  // <arg type="a{ss}" name="details" direction="in">
           simppl::dbus::in<std::uint32_t>,                       // <arg type="u" name="flags" direction="in">
           simppl::dbus::in<std::string>,                         // <arg type="s" name="cancellation_id" direction="in">
           simppl::dbus::out<AuthorizationResult>,                // <arg type="(bba{ss})" name="result" direction="out">
           simppl::dbus::_throw<simppl::dbus::Error>>
        CheckAuthorization;

    // constructor
    Authority()
        : INIT(CheckAuthorization)
    {
    }
};
}  // namespace PolicyKit1
}  // namespace org::freedesktop
