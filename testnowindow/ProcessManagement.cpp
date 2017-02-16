#include "ProcessManagement.h"
#include "ProcessManagement_HelperMethods.h"
#include <windows.h>
#include <tlhelp32.h>
#include <sstream>
#include <vector>
#include <chrono>
#include <sys/stat.h>

ProcessManagement::ProcessManagement()
{
}


ProcessManagement::~ProcessManagement()
{
}


bool FileExists(const wchar_t * name) {
	struct _stat buffer;
	return (_wstat(name, &buffer) == 0); //true if file exists
}

std::vector<int> ProcessManagement::GetProcessesIDsByName(wchar_t * pName) {
	std::vector<int> pidMatched;

	for each (PROCESSENTRY32 p in ProcessManagement_HelperMethods::GetProcesses())
	{
		if (_wcsicmp(p.szExeFile, pName) == 0) {	//If strings match
			pidMatched.push_back(p.th32ProcessID);
		}
	}
	return pidMatched;
}

void ProcessManagement::TerminateProcessByID(int PID) {
	HANDLE p = ProcessManagement_HelperMethods::GetHandle(PROCESS_TERMINATE, PID);
	if (p == NULL) return;

	TerminateProcess(p, 9);
	CloseHandle(p);
}

std::wstring ProcessManagement::ListProcesses() {
	//172ms  to complete the block on 130~ processes.

	std::wstringstream processInfoList;
	const wchar_t * separator = L" -- ";
	
	std::vector<THREADENTRY32> threadList = ProcessManagement_HelperMethods::GetThreads();

	for each (PROCESSENTRY32W p  in ProcessManagement_HelperMethods::GetProcesses())
	{
		processInfoList << p.szExeFile << separator; //Process name including .exe
		processInfoList << p.th32ProcessID << separator;
		processInfoList << p.th32ParentProcessID << separator;

		HANDLE pHandle = ProcessManagement_HelperMethods::GetHandle(PROCESS_QUERY_LIMITED_INFORMATION, p.th32ProcessID);
		wchar_t * pPath = ProcessManagement_HelperMethods::GetPath(pHandle); //delete pointer
		CloseHandle(pHandle);
		
		processInfoList << pPath << separator;

		HANDLE pHandleMemory = ProcessManagement_HelperMethods::GetHandle(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, p.th32ProcessID);
		wchar_t * pMemUsage = ProcessManagement_HelperMethods::GetMemoryUsage(pHandleMemory); //delete pointer
		CloseHandle(pHandleMemory);

		processInfoList << pMemUsage << separator;
		processInfoList << ProcessManagement_HelperMethods::GetThreadCount(threadList, p.th32ProcessID) << separator;

		//Final parameter, no need for separator

		pPath == L"N/A" ? processInfoList << L"N/A" : processInfoList << ProcessManagement_HelperMethods::GetFileDescription(pPath);
		
		if (pPath != L"N/A") {
			delete[] pPath;
		}

		if (pMemUsage != L"N/A") {
			delete[] pMemUsage;
		}
		processInfoList << L"\n";
	}

	return processInfoList.str();
}

void ProcessManagement::ExecuteFile(const wchar_t * filePath, bool elevate) {
	//runas doesn't work if the file isn't an .exe or .bat
	//don't know why, when in need search SO

	if (!FileExists(filePath)) return;
	
	wchar_t * verb = NULL;

	if (elevate) {
		verb = L"runas";
	}

	ShellExecuteW(NULL, verb, filePath, NULL, NULL, SW_SHOWNORMAL);
}