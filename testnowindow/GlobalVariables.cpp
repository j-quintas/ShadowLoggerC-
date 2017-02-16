#include "GlobalVariables.h"
#include "HelperMethods.h"
#include <ShlObj.h>


GlobalVariables::GlobalVariables()
{
}


GlobalVariables::~GlobalVariables()
{
}


std::wstring GetAppDataPath() {
	wchar_t * path;
	SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);
	std::wstring _path = path;
	CoTaskMemFree(path);
	return _path;
}



const std::wstring AppDefaultDirectory(GetAppDataPath() + L"\\temp");