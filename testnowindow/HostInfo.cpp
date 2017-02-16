#include "HostInfo.h"
#include "HostInfo_HelperMethods.h"
#include "HelperMethods.h"
#include <Windows.h>
#include <VersionHelpers.h>
#include <Lmcons.h>
#include <sstream>
#include <TlHelp32.h>
#include <Psapi.h>
#include <iomanip>
#include <Shlwapi.h>
#include <wininet.h>


#pragma comment(lib, "wininet")
#pragma comment(lib, "Shlwapi.lib")



HostInfo::HostInfo()
{
}


HostInfo::~HostInfo()
{
}


//TODO: Order this methods as shown in the logs


void HostInfo::UserName() {
	wchar_t userName[UNLEN + 1];

	DWORD userNameLength = UNLEN + 1;

	GetUserNameW(userName, &userNameLength);
	OutputDebugStringW(userName);
}

void HostInfo::ComputerName() {
	wchar_t computerName[MAX_COMPUTERNAME_LENGTH + 1];

	DWORD computerNameLength = MAX_COMPUTERNAME_LENGTH + 1;

	GetComputerNameW(computerName, &computerNameLength);
	OutputDebugStringW(computerName);
}

//This only works if running as x86
void HostInfo::CPUArchitecture() {
	BOOL wow64;
	IsWow64Process(GetCurrentProcess(), &wow64);

	if (wow64) {
		OutputDebugStringW(L"64-bit");
	}
	else {
		OutputDebugStringW(L"32-bit");
	}
}

void HostInfo::IdleTime() {
	LASTINPUTINFO lastInput;
	lastInput.cbSize = sizeof(LASTINPUTINFO);

	GetLastInputInfo(&lastInput);

	ULONGLONG tickCount = GetTickCount64();

	ULONGLONG idleTime = tickCount - lastInput.dwTime;

	std::wstringstream a;

	a << idleTime;

	OutputDebugStringW(a.str().c_str());
}

void HostInfo::SystemRAMUsedPercentage() {
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);

	GlobalMemoryStatusEx(&memInfo);

	std::wstringstream a;
	a << memInfo.dwMemoryLoad;
	OutputDebugStringW(a.str().c_str());
}

void HostInfo::ProcessStartTime() {
	FILETIME creationTime;
	FILETIME exitTime;
	FILETIME kernelTime;
	FILETIME userTime;

	GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime);

	SYSTEMTIME creationSystemTime;
	FileTimeToSystemTime(&creationTime, &creationSystemTime);

	std::wstringstream a;
	a << creationSystemTime.wDay << "/";
	a << creationSystemTime.wMonth << "/";
	a << creationSystemTime.wYear << " ";
	a << creationSystemTime.wHour << ":";
	a << creationSystemTime.wMinute << ":";
	a << creationSystemTime.wSecond;

	OutputDebugStringW(a.str().c_str());
}

void HostInfo::IsProcessAdmin() {
	//Tested and working

	HANDLE processToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &processToken);
	DWORD tokenSize = 0;
	GetTokenInformation(processToken, TokenElevation, NULL, tokenSize, &tokenSize);

	TOKEN_ELEVATION elevated;
	if (GetTokenInformation(processToken, TokenElevation, &elevated, tokenSize, &tokenSize)) {
		OutputDebugStringW(L"GetTokenInformation SUCCESS\n");
	}

	if (elevated.TokenIsElevated) {
		OutputDebugStringW(L"Admin\n");
	}
	else {
		OutputDebugStringW(L"Not Admin\n");
	}

	CloseHandle(processToken);
}

void HostInfo::IsUserAdmin() {
	HANDLE processToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &processToken);
	DWORD tokenSize = 0;
	GetTokenInformation(processToken, TokenElevationType, NULL, tokenSize, &tokenSize);

	TOKEN_ELEVATION_TYPE elevated;
	if (GetTokenInformation(processToken, TokenElevationType, &elevated, tokenSize, &tokenSize)) {
		OutputDebugStringW(L"GetTokenInformation SUCCESS\n");
	}

	switch (elevated) {
	case TokenElevationTypeDefault:
		OutputDebugStringW(L"Normal User (Admin if UAC disabled)\n");
		break;
	case TokenElevationTypeFull:		//Both mean that the current user is admin because Limited means the token passed is limited privileges
	case TokenElevationTypeLimited:		//which means the user didn't ran as admin, but they're admin
		OutputDebugStringW(L"Admin\n");
		break;
	default:
		break;
	}

	CloseHandle(processToken);
}

void HostInfo::PowerStatus() {
	SYSTEM_POWER_STATUS status;
	GetSystemPowerStatus(&status);

	switch (status.ACLineStatus) {
	case 0: OutputDebugStringW(L"Battery\n");
		break;
	case 1: OutputDebugStringW(L"AC\n");
		break;
	case 255: OutputDebugStringW(L"Unkown\n");
		break;
	}

	switch (status.BatteryFlag) {
	case 128: OutputDebugStringW(L"No battery\n");
	}

	std::wstringstream a;

	a << status.BatteryLifePercent;

	OutputDebugStringW(a.str().c_str());
	OutputDebugStringW(L"%\n");

	if (status.BatteryLifeTime == MAXDWORD) {
		OutputDebugStringW(L"Unkown\n");
	}
	else {
		a.clear();
		a << status.BatteryLifeTime / 60;
		OutputDebugStringW(a.str().c_str());
		OutputDebugStringW(L"min\n");
	}
}

void HostInfo::OSVersion() {
	if (IsWindows8Point1OrGreater()) {
		OutputDebugStringW(L"Windows 8.1");
	}
	else if (IsWindows8OrGreater()) {
		OutputDebugStringW(L"Windows 8");
	}
	else if (IsWindows7SP1OrGreater()) {
		OutputDebugStringW(L"Windows 7 SP1");
	}
	else if (IsWindows7OrGreater()) {
		OutputDebugStringW(L"Windows 7");
	}
	else if (IsWindowsVistaSP2OrGreater()) {
		OutputDebugStringW(L"Windows Vista SP2");
	}
	else if (IsWindowsVistaSP1OrGreater()) {
		OutputDebugStringW(L"Windows Vista SP1");
	}
	else if (IsWindowsVistaOrGreater()) {
		OutputDebugStringW(L"Windows Vista");
	}
	else if (IsWindowsXPSP3OrGreater()) {
		OutputDebugStringW(L"Windows XP SP3");
	}
	else if (IsWindowsXPSP2OrGreater()) {
		OutputDebugStringW(L"Windows XP SP2");
	}
	else if (IsWindowsXPSP1OrGreater()) {
		OutputDebugStringW(L"Windows XP SP1");
	}
	else if (IsWindowsXPOrGreater()) {
		OutputDebugStringW(L"Windows XP");
	}
}

void HostInfo::ProcessThreadCount() {
	int count = 0;
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h != INVALID_HANDLE_VALUE) {
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		if (Thread32First(h, &te)) {
			int PID = GetCurrentProcessId();
			do {
				if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) +
					sizeof(te.th32OwnerProcessID)) {
					if (te.th32OwnerProcessID == PID) {
						count += 1;
					}
				}
				te.dwSize = sizeof(te);
			} while (Thread32Next(h, &te));
		}
		CloseHandle(h);
	}
	std::wstringstream a;
	a << count;
	OutputDebugStringW(a.str().c_str());
	OutputDebugStringW(L" Thread(s)\n");
}

void HostInfo::ProcessRAMUsage() {
	PROCESS_MEMORY_COUNTERS_EX memUsage;
	memUsage.cb = sizeof(memUsage);
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&memUsage, memUsage.cb); //Cast is required apparently, https://social.msdn.microsoft.com/Forums/en-US/720198c4-04a2-4737-9159-6e23a217d6b7/question-about-getprocessmemoryinfo?forum=Vsexpressvc

	wchar_t buffer[100];

	StrFormatByteSizeW(memUsage.PrivateUsage, buffer, sizeof(buffer));
	OutputDebugStringW(buffer);
	OutputDebugStringW(L"\n");
}

void HostInfo::LocalTime() {
	time_t t = time(0);   // get time now
	struct tm now;
	localtime_s(&now, &t);

	char timeFormatted[30];
	strftime(timeFormatted, 30, "%Y/%m/%d %H:%M:%S", &now);

	size_t convertedChars = 0;
	wchar_t timeFormattedWide[30];

	mbstowcs_s(&convertedChars, timeFormattedWide, timeFormatted, 29);

	OutputDebugStringW(timeFormattedWide);
	OutputDebugStringW(L"\n");
}

void HostInfo::ProcessPath() {
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, sizeof(path));
	OutputDebugStringW(path);
	OutputDebugStringW(L"\n");
}

void HostInfo::ProductKey() {
	byte* productKey = HelperMethods::GetRegistryValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"DigitalProductId", KEY_QUERY_VALUE | KEY_WOW64_64KEY);;

	IsWindows8OrGreater() ? HostInfo_HelperMethods::DecodeProductKeyWin8AndUp(productKey) : HostInfo_HelperMethods::DecodeProductKey(productKey);
}

void HostInfo::UACStatus() {
	if (!IsWindows8OrGreater()) {
		const wchar_t * valueName = L"EnableLUA";
		HKEY keyHandle;
		RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &keyHandle);

		if (keyHandle == NULL) {
			OutputDebugStringW(L"Error accessing registry key\n");
		}

		DWORD hexLength = 0;
		if (RegQueryValueExW(keyHandle, valueName, NULL, NULL, NULL, &hexLength) == ERROR_SUCCESS) {
			OutputDebugStringW(L"Success\n");
		}

		DWORD uac = 0;
		if (RegQueryValueExW(keyHandle, valueName, NULL, NULL, (LPBYTE)&uac, &hexLength) == ERROR_SUCCESS) {
			OutputDebugStringW(L"Success\n");
		}

		uac ? OutputDebugStringW(L"UAC Enabled\n") : OutputDebugStringW(L"UAC Disabled\n");

		RegCloseKey(keyHandle);
		return;
	}
	
	OutputDebugStringW(L"Unkown\n"); //Windows 8 makes it impossible to turn off UAC, turning it off from registry is unsupported, no way to definitely check
}

void HostInfo::ExternalIP() {
	HINTERNET hInternet, hFile;
	DWORD rSize;
	char buffer[30] = "\0";

	hInternet = InternetOpenW(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	hFile = InternetOpenUrlW(hInternet, L"http://icanhazip.com", NULL, 0, INTERNET_FLAG_RELOAD, 0);
	
	InternetReadFile(hFile, &buffer, sizeof(buffer), &rSize);

	rSize = MultiByteToWideChar(CP_UTF8, 0, buffer, sizeof(buffer), 0, 0);

	wchar_t * bufferW = new wchar_t[rSize];

	MultiByteToWideChar(CP_UTF8, 0, buffer, sizeof(buffer), bufferW, wcslen(bufferW));

	InternetCloseHandle(hFile);
	InternetCloseHandle(hInternet);

	OutputDebugStringW(bufferW);
	delete[] bufferW;
}

void HostInfo::ComputerType() {
	HostInfo_HelperMethods::ComputerType();
}