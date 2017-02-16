#include "HelperMethods.h"
#include <Pathcch.h>
#include <Shlwapi.h>
#include <string>
	
HelperMethods::HelperMethods()
{
}


HelperMethods::~HelperMethods()
{
}



byte* HelperMethods::GetRegistryValue(HKEY hKey, const wchar_t* keyPath, const wchar_t* keyValue, REGSAM regsam) {
	RegOpenKeyExW(hKey, keyPath, 0, regsam, &hKey);

	if (hKey == NULL)
	{
		OutputDebugStringW(L"Error accessing registry key\n");
		return NULL;
	}

	DWORD hexLength = 0;

	RegQueryValueExW(hKey, keyValue, NULL, NULL, NULL, &hexLength);

	byte* valueData = new byte[hexLength];

	LSTATUS lstatus = RegQueryValueExW(hKey, keyValue, NULL, NULL, valueData, &hexLength);

	RegCloseKey(hKey);

	return lstatus == ERROR_SUCCESS ? valueData : NULL;
}

bool HelperMethods::SetRegistryValue(HKEY hKey, const wchar_t* keyPath, const wchar_t* keyValue,const wchar_t* keyData, REGSAM regsam) {
	RegOpenKeyExW(hKey, keyPath, 0, regsam, &hKey);

	if (hKey == NULL)
	{
		OutputDebugStringW(L"Error accessing registry key\n");
		return false;
	}

	LSTATUS lstatus = RegSetValueExW(hKey, keyValue, 0, REG_SZ, (byte*)keyData, wcslen(keyData) + 1); //+1 because it must include the null character with REG_SZ
	
	RegCloseKey(hKey);

	return lstatus == ERROR_SUCCESS ? true : false;
}

//Returns same pointer modified
wchar_t * HelperMethods::RemovePathFilename(wchar_t * filePath) {
	PathRemoveFileSpecW(filePath);
	return filePath;
}

//Returns same pointer modified
wchar_t * HelperMethods::RemovePathExtension(wchar_t * filePath) {
	PathRemoveExtensionW(filePath);
	return filePath;
}
//Returns different pointer
wchar_t * HelperMethods::GetPathFilename(const wchar_t * filePath) {
	return PathFindFileNameW(filePath);
}