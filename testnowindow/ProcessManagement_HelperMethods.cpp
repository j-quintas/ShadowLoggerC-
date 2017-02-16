#include "ProcessManagement_HelperMethods.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <Psapi.h>
#include <strsafe.h>
#include <Shlwapi.h>

#pragma comment(lib,"Version.lib")

ProcessManagement_HelperMethods::ProcessManagement_HelperMethods()
{
}


ProcessManagement_HelperMethods::~ProcessManagement_HelperMethods()
{
}


std::vector<PROCESSENTRY32W> ProcessManagement_HelperMethods::GetProcesses() {
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(PROCESSENTRY32W);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	std::vector<PROCESSENTRY32W> processList;

	if (Process32FirstW(snapshot, &entry) == TRUE) //First process is SYSTEM
	{
		processList.push_back(entry);
		while (Process32NextW(snapshot, &entry) == TRUE)
		{
			processList.push_back(entry); //add to vector
		}
	}

	CloseHandle(snapshot);
	return processList;
}

HANDLE ProcessManagement_HelperMethods::GetHandle(int PROCESS_ACCESS, int PID) {
	return OpenProcess(PROCESS_ACCESS, false, PID);
}

wchar_t * ProcessManagement_HelperMethods::GetPath(HANDLE pHandle) {
	if (pHandle == NULL) return L"N/A";

	wchar_t * pPath = new wchar_t[MAX_PATH];
	DWORD pPathSize = wcslen(pPath);
	QueryFullProcessImageNameW(pHandle, 0, pPath, &pPathSize);
	return pPath;
}

wchar_t * ProcessManagement_HelperMethods::GetMemoryUsage(HANDLE pHandle) {
	if (pHandle == NULL) return L"N/A";
	PROCESS_MEMORY_COUNTERS_EX memUsage;
	memUsage.cb = sizeof(memUsage);
	GetProcessMemoryInfo(pHandle, (PROCESS_MEMORY_COUNTERS*)&memUsage, memUsage.cb); //Cast is required apparently, https://social.msdn.microsoft.com/Forums/en-US/720198c4-04a2-4737-9159-6e23a217d6b7/question-about-getprocessmemoryinfo?forum=Vsexpressvc

	wchar_t * buffer = new wchar_t[100];

	StrFormatByteSizeW(memUsage.PrivateUsage, buffer, sizeof(buffer));
	return buffer;
}

std::vector<THREADENTRY32> ProcessManagement_HelperMethods::GetThreads() {
	//This takes 9ms to enumerate threads of 100~ processes
	//http://blogs.msdn.com/b/oldnewthing/archive/2006/02/23/537856.aspx

	std::vector<THREADENTRY32> threadList;

	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	if (h != INVALID_HANDLE_VALUE) {
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		if (Thread32First(h, &te)) {
			do {
				if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) +
					sizeof(te.th32OwnerProcessID)) {
					threadList.push_back(te);
				}
				te.dwSize = sizeof(te);
			} while (Thread32Next(h, &te));
		}
		CloseHandle(h);
	}
	return threadList;
}

int ProcessManagement_HelperMethods::GetThreadCount(std::vector<THREADENTRY32> threadList, int PID) {
	int count = 0;
	for (size_t i = 0; i < threadList.size(); i++) {
		if (threadList[i].th32OwnerProcessID == PID) {
			count += 1;
		}
	}
	return count;
}

struct LANGANDCODEPAGE {
	WORD wLanguage;
	WORD wCodePage;
} *lpTranslate;

std::wstring ProcessManagement_HelperMethods::GetFileDescription(wchar_t * pPath) {
	int fileVersionInfoSize = GetFileVersionInfoSizeW(pPath, NULL);

	if (fileVersionInfoSize == 0)	return L"N/A"; //Shouldn't happen

	wchar_t * fileVersionInfo = new wchar_t[fileVersionInfoSize];

	GetFileVersionInfoW(pPath, NULL, fileVersionInfoSize, fileVersionInfo);

	UINT cbTranslate = 0;

	// Structure used to store enumerated languages and code pages.

	HRESULT hr;

	// Read the list of languages and code pages.

	BOOL a = VerQueryValueW(fileVersionInfo,
		TEXT("\\VarFileInfo\\Translation"),
		(LPVOID*)&lpTranslate,
		&cbTranslate);

	// Read the file description for each language and code page.

	wchar_t * subBlock = new wchar_t[50];

	for (size_t i = 0; i < (cbTranslate / sizeof(struct LANGANDCODEPAGE)); i++)
	{
		hr = StringCchPrintfW(subBlock, 50,
			TEXT("\\StringFileInfo\\%04x%04x\\FileDescription"),
			lpTranslate[i].wLanguage,
			lpTranslate[i].wCodePage);

		if (FAILED(hr))
		{
			return L"N/A";
		}

		void * lpBuffer = NULL;
		UINT dwBytes = 0;

		// Retrieve file description for language and code page "i". 
		VerQueryValueW(fileVersionInfo,
			subBlock,
			&lpBuffer,
			&dwBytes);

		std::wstring description((wchar_t*)lpBuffer);

		delete[] subBlock;
		delete[] fileVersionInfo;
		return description;
	}
	return L"N/A";
}
