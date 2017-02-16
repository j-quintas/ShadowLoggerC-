#pragma once
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>

class ProcessManagement_HelperMethods
{
public:
	ProcessManagement_HelperMethods();
	~ProcessManagement_HelperMethods();

	static std::vector<PROCESSENTRY32> GetProcesses();
	static HANDLE GetHandle(int, int);
	static wchar_t * GetPath(HANDLE);
	static wchar_t * GetMemoryUsage(HANDLE);
	static std::vector<THREADENTRY32> GetThreads();
	static int GetThreadCount(std::vector<THREADENTRY32>, int);
	static std::wstring GetFileDescription(wchar_t *);
};

