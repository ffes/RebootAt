# RebootAt

## Introduction

For years I have installed [Cygwin](https://cygwin.com) on basically all my machines.
But with the introduction of WSL and especially WSL2 I stopped using it.

One of the Cygwin tools I used often on our servers at the office was the `shutdown` command.
Yes, Windows has a pretty decent `shutdown.exe` but the major advantage of the Cygwin `shutdown` is that it mimics the Linux `shutdown` command, especially the possibility to add a timestamp at the end.
And another minor disadvantage of `shutdown.exe` is that when you use the wrong command line options you could shutdown the machine instead of rebooting it.

```sh
shutdown -fri 23:30
```

This will forcefully reboot the machine and install the pending updates at 23:30.
The program will calculate the number of seconds between the moment of invocation and the requested time and pass that to the `InitiateShutdown()` Win32 API.

Every month I miss this functionality.
When you need to install the regular Windows updates on a number of servers, you want to reboot them at a moment it has the least impact for the users.
And you don't want to calculate that number of seconds.
And I don't want to have to go through the hassle of setting up a one-time Scheduled Task.
A command line tool is much easier.

I have maintained the [sources of Cygwin shutdown](https://github.com/cygwin/shutdown) for some time, so I took that as an inspiration for this tool.
And since some of the code has been reused, I will obviously release this with the GPL2 license as well.

## Usage

This command line tool reboots you machine at a given time.

The tool takes just one argument, a timestamp when the computer needs to reboot.

The word `now` (case insensitive) means just that, reboot immediately.

```sh
# Reboot immediately
RebootAt now
```

A timestamp means to reboot at the first time this time will occur.
The timestamp has to be in 24-hours notation.

```sh
# Reboot today at 23:30
RebootAt 23:30

# Reboot tomorrow at 2:00
RebootAt 2:00
```

`+` means the number following is in minutes.

```sh
# Reboot in 60 minutes
RebootAt +60
```
