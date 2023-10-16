/**
 *
 * Reboot the local machine at a given time
 *
 */

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

/**
 * Get the human readable error message belonging to the given Windows error code value
 *
 * @param err The Windows error code
 *
 * @return The error message belonging to the given error
 */
static std::string GetErrorMessage(DWORD err)
{
	char errbuf[256];
	sprintf_s(errbuf, 256, "Error %lu ", err);
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) errbuf + strlen(errbuf),
		sizeof(errbuf) - strlen(errbuf), NULL);
	return errbuf;
}

/**
 * Elevate the privileges required to preform a reboot
 *
 * @return Returns `true` if the privileges were elevated and the reboot can be invoked
 */
static bool ElevatePrivileges()
{
	// If the privilege hasn't been found, we're trying to reboot anyway.
	TOKEN_PRIVILEGES privs{};
	if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &privs.Privileges[0].Luid))
	{
		std::cerr << "Warning: can't evaluate privilege: " << GetErrorMessage(GetLastError()) << std::endl;
		return true;
	}

	privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	privs.PrivilegeCount = 1;

	HANDLE token;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
	{
		std::cerr << "Can't open process token: " << GetErrorMessage(GetLastError()) << std::endl;
		return false;
	}

	if (!AdjustTokenPrivileges(token, FALSE, &privs, 0, NULL, NULL))
	{
		std::cerr << "Can't set required privilege: " << GetErrorMessage(GetLastError()) << std::endl;
		return false;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		std::cerr << "Required privilege not held: " << GetErrorMessage(GetLastError()) << std::endl;
		return false;
	}

	if (!RevertToSelf())
	{
		std::cerr << "Can't activate required privilege" << GetErrorMessage(GetLastError()) << std::endl;
		return false;
	}

	return true;
}

/**
 * Reboot the system
 *
 * @param secs The number of seconds before the reboot should occur
 * @param msg The message to display in the system popup
 * @param force All sessions are forcefully logged off
 *
 * @return Returns `true` if reboot was passed to the OS successful
 */
static bool Reboot(DWORD secs, std::wstring msg, bool force)
{
	if (!ElevatePrivileges())
		return false;

	// The system needs to be rebooted
	DWORD dwFlags = SHUTDOWN_RESTART;

	// Force the reboot
	if (force)
		dwFlags |= SHUTDOWN_FORCE_OTHERS | SHUTDOWN_FORCE_SELF;

	// Make sure updates are installed
	dwFlags |= SHUTDOWN_INSTALL_UPDATES;

	// Put the Rebooting in front of the message
	msg = L"System will reboot " + msg;

	// Display the message
	std::wcout << msg.c_str() << std::endl;

	// Perform the reboot itself
	const DWORD dwReason = SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_FLAG_PLANNED;
	const DWORD ret = InitiateShutdown(NULL, (LPWSTR) msg.c_str(), secs, dwFlags, dwReason);
	return ret == ERROR_SUCCESS;
}

/**
 * Convert a given timestamp to the number of seconds and the corresponding message
 *
 * @param timestamp The timestamp to parse
 * @param secs The number of seconds from now until the given timestamp occurs
 * @param timemsg The human readable time
 *
 * @return Returns `true` if the timestamp was successfully parsed
 */
static bool TimespanToSeconds(std::string timestamp, long& secs, std::wstring& timemsg)
{
	std::transform(timestamp.begin(), timestamp.end(), timestamp.begin(), ::toupper);
	if (timestamp == "NOW")
	{
		secs = 0;
		timemsg = L"NOW";
		return true;
	}

	// Timestamp starting with a `+` means minutes
	if (timestamp.at(0) == '+')
	{
		int minutes = std::stoi(timestamp.substr(1));
		secs = minutes * 60;
		timemsg = L"in " + std::to_wstring(minutes) + L" minutes";
		return true;
	}

	// Timestamp containing a `:` must be a real timestamp
	size_t colon = timestamp.find(":");
	if (colon != std::string::npos)
	{
		int hour = std::stoi(timestamp.substr(0, colon));
		int minute = std::stoi(timestamp.substr(colon + 1));

		// Do some basic checking of the timestamp
		if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
			return false;

		// Calculate the number of seconds until we need to reboot
		time_t now = time(NULL);	// number of seconds
		tm loc;
		localtime_s(&loc, &now);	// converted to a tm struct

		// If the given hour is smaller then the current hour, it is the next day (now = 17:30, timestamp = 02:10)
		// If the hours are the same and the given minutes is smaller, it the next day as well (now = 17:30, timestamp = 17:00)
		if (hour < loc.tm_hour || (loc.tm_hour == hour && minute < loc.tm_min))
		{
			time_t nextday = time(NULL) + (24 * 60 * 60);
			localtime_s(&loc, &nextday);
		}

		// Substitude the given hour and minute
		loc.tm_hour = hour;
		loc.tm_min = minute;
		loc.tm_sec = 0;	// Always reboot the whole minute

		// Convert the new time to number of seconds
		time_t then = mktime(&loc);
		secs = then - now;

		// Construct the message
		std::wostringstream stream;
		stream << L"at " << std::setfill(L'0') << std::setw(2) << hour << L":" << std::setfill(L'0') << std::setw(2) << minute;
		timemsg = stream.str();

		return true;
	}

	return false;
}

int main(int argc, char * argv[])
{
	// Is there a timestamp at the command line?
	if (argc != 2)
	{
		std::cerr << "No timestamp given." << std::endl;
		return 1;
	}

	// Convert the timestamp to the number of seconds
	long secs;
	std::wstring timemsg;
	if (!TimespanToSeconds(argv[1], secs, timemsg))
	{
		std::cerr << "Invalid timestamp: " << argv[1] << std::endl;
		return 1;
	}

	// Try to perform the reboot
	if (Reboot(secs, timemsg, true))
		return 0;
	return 1;
}
