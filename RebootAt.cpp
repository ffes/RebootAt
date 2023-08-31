/**
 * 
 * Reboot the local machine at a given time
 * 
 */

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <iostream>

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
 * @param msg  The message to display in the system popup
 *
 * @return Returns `true` if reboot was passed to the OS successful
 */
static bool Reboot(DWORD secs, std::wstring msg)
{
	if (!ElevatePrivileges())
		return false;

	DWORD dwFlags = SHUTDOWN_RESTART;
	DWORD dwReason = SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_FLAG_PLANNED;

	// Force
	dwFlags |= SHUTDOWN_FORCE_OTHERS | SHUTDOWN_FORCE_SELF;

	// Display the message
	std::wcout << msg.c_str() << std::endl;

	// Perform the reboot itself
	DWORD ret = InitiateShutdown(NULL, (LPWSTR) msg.c_str(), secs, dwFlags, dwReason);
	return ret == ERROR_SUCCESS;
}

int main(int argc, char * argv[])
{
	std::wstring msg = L"Rebooting in an hour";

	if (Reboot(3600, msg))
		return 0;
	return 1;
}
