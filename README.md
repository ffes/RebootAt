# RebootAt

## Introduction

For years I have installed [Cygwin](https://cygwin.com) on basically all my machines.
But with the introduction of WSL and especially WSL2 I stopped using it.

One of the Cygwin tools I used often on our servers at the office was the `shutdown` command.
Yes, Windows has a pretty decent `shutdown.exe` but the major advantage of the Cygwin `shutdown` is that it mimics the Linux `shutdown` command, especially the possibility to add a timestamp at the end.

```
shutdown -fri 23:30
```

This will forcefully reboot the machine and install the pending updates at 23:30.
The program will calculate the number of seconds between the moment of invocation and the requested time and pass that to the `InitiateShutdown()` Win32 API.

Every month I miss this functionality.
When you need to install the regular Windows updates on a number of servers, you want to reboot them at a moment it has the least impact for the users.
And you don't want to calculate that number of seconds.

I have maintained the [sources of Cygwin shutdown](https://github.com/cygwin/shutdown) for some time, so I took that as an inspiration for this tool.
And since some of the code has been reused, I will obviously release this with the GPL2 license as well.

## Usage

This command line tool reboot you machine at a given time

The tool can take a couple of arguments.
The `time` argument is required.

```
RebootAt 23:30
```

```
RebootAt -f now
```
