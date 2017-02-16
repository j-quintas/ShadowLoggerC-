#include <vector>
#include <string>

#pragma once
class ProcessManagement
{
public:
	ProcessManagement();
	~ProcessManagement();

	static std::vector<int> GetProcessesIDsByName(wchar_t *);
	static void TerminateProcessByID(int);
	static std::wstring ListProcesses();
	static void ExecuteFile(const wchar_t *, bool);
};
