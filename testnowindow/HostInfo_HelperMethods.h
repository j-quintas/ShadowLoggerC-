#include <windows.h>

#pragma once
class HostInfo_HelperMethods
{
public:
	HostInfo_HelperMethods();
	~HostInfo_HelperMethods();

	static wchar_t * ComputerType();
	static void DecodeProductKey(byte*);
	static void DecodeProductKeyWin8AndUp(byte*);
};


