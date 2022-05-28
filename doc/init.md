Sample init scripts and service configuration for novobitcoind
==========================================================

Sample scripts and configuration files for systemd, Upstart and OpenRC
can be found in the contrib/init folder.

    contrib/init/novobitcoind.service:    systemd service unit configuration
    contrib/init/novobitcoind.openrc:     OpenRC compatible SysV style init script
    contrib/init/novobitcoind.openrcconf: OpenRC conf.d file
    contrib/init/novobitcoind.conf:       Upstart service configuration file
    contrib/init/novobitcoind.init:       CentOS compatible SysV style init script

1. Service User
---------------------------------

All three Linux startup configurations assume the existence of a "bitcoin" user
and group.  They must be created before attempting to use these scripts.
The OS X configuration assumes novobitcoind will be set up for the current user.

2. Configuration
---------------------------------

At a bare minimum, novobitcoind requires that the rpcpassword setting be set
when running as a daemon.  If the configuration file does not exist or this
setting is not set, novobitcoind will shutdown promptly after startup.

This password does not have to be remembered or typed as it is mostly used
as a fixed token that novobitcoind and client programs read from the configuration
file, however it is recommended that a strong and secure password be used
as this password is security critical to securing the wallet should the
wallet be enabled.

If novobitcoind is run with the "-server" flag (set by default), and no rpcpassword is set,
it will use a special cookie file for authentication. The cookie is generated with random
content when the daemon starts, and deleted when it exits. Read access to this file
controls who can access it through RPC.

By default the cookie is stored in the data directory, but it's location can be overridden
with the option '-rpccookiefile'.

This allows for running novobitcoind without having to do any manual configuration.

`conf`, `pid`, and `wallet` accept relative paths which are interpreted as
relative to the data directory. `wallet` *only* supports relative paths.

For an example configuration file that describes the configuration settings,
see `contrib/debian/examples/bitcoin.conf`.

3. Paths
---------------------------------

All three configurations assume several paths that might need to be adjusted.

Binary:              `/usr/bin/novobitcoind`
Configuration file:  `/etc/bitcoin/bitcoin.conf`
Data directory:      `/var/lib/novobitcoind`
PID file:            `/var/run/novobitcoind/novobitcoind.pid` (OpenRC and Upstart) or `/var/lib/novobitcoind/novobitcoind.pid` (systemd)
Lock file:           `/var/lock/subsys/novobitcoind` (CentOS)

The configuration file, PID directory (if applicable) and data directory
should all be owned by the bitcoin user and group.  It is advised for security
reasons to make the configuration file and data directory only readable by the
bitcoin user and group.  Access to bitcoin-cli and other novobitcoind rpc clients
can then be controlled by group membership.

4. Installing Service Configuration
-----------------------------------

4a) systemd

Installing this .service file consists of just copying it to
/usr/lib/systemd/system directory, followed by the command
`systemctl daemon-reload` in order to update running systemd configuration.

To test, run `systemctl start novobitcoind` and to enable for system startup run
`systemctl enable novobitcoind`

4b) OpenRC

Rename novobitcoind.openrc to novobitcoind and drop it in /etc/init.d.  Double
check ownership and permissions and make it executable.  Test it with
`/etc/init.d/novobitcoind start` and configure it to run on startup with
`rc-update add novobitcoind`

4c) Upstart (for Debian/Ubuntu based distributions)

Drop novobitcoind.conf in /etc/init.  Test by running `service novobitcoind start`
it will automatically start on reboot.

NOTE: This script is incompatible with CentOS 5 and Amazon Linux 2014 as they
use old versions of Upstart and do not supply the start-stop-daemon utility.

4d) CentOS

Copy novobitcoind.init to /etc/init.d/novobitcoind. Test by running `service novobitcoind start`.

Using this script, you can adjust the path and flags to the novobitcoind program by
setting the NOVOBITCOIND and FLAGS environment variables in the file
/etc/sysconfig/novobitcoind. You can also use the DAEMONOPTS environment variable here.

5. Auto-respawn
-----------------------------------

Auto respawning is currently only configured for Upstart and systemd.
Reasonable defaults have been chosen but YMMV.
