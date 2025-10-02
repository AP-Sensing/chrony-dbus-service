# This is the README for chrony-dbus-service.

## What is chrony?
chrony is a versatile implementation of the Network Time Protocol (NTP).

see [gitlab.com/chrony](https://gitlab.com/chrony/chrony)
  

## What is chrony-dbus-service?
To configure chronyd with the chronyc command line tool, each command has to written in a parseable line of text.  
Executing chronyc from within another application doesn't provide a good usability and in case that the chronyc parser is changed it will cause maintainability issues.
The chrony-dbus-service provides partial functionality of configuring chornyd like it would be possible with chronyc, but instead this functionality is provided as an accessible DBus interface API.
To compile completely in c++ the files in src/chrony were adapted.

## Features
- ** getSources: ** Lists chronyd sources like calling `chronyc sources`
- ** addServers: ** Adds NTP servers to chronyd like calling `chronyc add server`
- ** deleteServers: ** Removes NTP servers from chronyd like calling `chronyc delete`
- ** addManualTime: ** Adds a manual time entry to chronyd like calling `chronyc settime`
- ** clearManualTimeList: ** Removes all manual time entries from chronyd like calling `chronyc manual delete` for each entry
- ** getManualTimeList: ** Lists chronyd manual time entries like calling `chronyc manual list`
- ** setManualTimeEnabled: ** Enables or disables chronyd manual time entries like calling `chronyc manual on` or `chronyc manual off`


## How to use chrony-dbus-service?
You can use any DBus compatible library like e.g. libdbus-c++ or simppl.  
For an example see [chronydbustest.cpp](src/tests/chronydbustest.cpp).
