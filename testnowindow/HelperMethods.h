#include <Windows.h>

#pragma once
class HelperMethods
{
public:
	HelperMethods();
	~HelperMethods();

	static byte* GetRegistryValue(HKEY hKey, const wchar_t* keyPath, const wchar_t* keyValue, REGSAM regsam);
	static bool  SetRegistryValue(HKEY hKey, const wchar_t* keyPath, const wchar_t* keyValue, const wchar_t* keyData, REGSAM regsam);

	static wchar_t* RemovePathFilename(wchar_t *);
	static wchar_t* RemovePathExtension(wchar_t *);
	static wchar_t* GetPathFilename(const wchar_t *);
};