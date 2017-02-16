#include "HostInfo_HelperMethods.h"
#include <windows.h>
#include <stdio.h>
#include <ole2.h>
#include <oleauto.h>
#include <wbemidl.h>
#include <comdef.h>
#include <string>


#pragma comment(lib,"Wbemuuid.lib")


HostInfo_HelperMethods::HostInfo_HelperMethods()
{
}


HostInfo_HelperMethods::~HostInfo_HelperMethods()
{
}



void HostInfo_HelperMethods::DecodeProductKey(byte* productkey) {
	// Possible alpha-numeric characters in product key
	const wchar_t keyValues[] = L"BCDFGHJKMPQRTVWXY2346789";
	// Length of decoded product key in byte-form. Each byte represents 2 chars
	const int decodeStringLength = 15;
	// Decoded product key is of length 29 (plus terminating null character)
	wchar_t decodedChars[30] = L"\0";
	// Extract encoded product key from bytes [52,67]
	byte hexPid[16] = {};

	int count = 0;
	for (size_t i = 52; i <= 67; i++) {
		hexPid[count] = productkey[i];
		count += 1;
	}
	delete[] productkey;

	// Decode characters

	//28 because it's the last index of decodedChars without the null character
	//!= -1 because it should break after reaching 0 instead of going to UINT_MAX, -1 == UINT_MAX
	for (size_t i = 28; i != -1; i--) {
		// Every sixth char is a separator
		if ((i + 1) % 6 == 0) {
			decodedChars[i] = L'-';
		}
		else {
			// Do the actual decoding
			int digitMapIndex = 0;
			for (int j = decodeStringLength - 1; j >= 0; j--) {
				int byteValue = (digitMapIndex << 8) | (byte)hexPid[j];
				hexPid[j] = (byte)(byteValue / 24);
				digitMapIndex = byteValue % 24;
				decodedChars[i] = keyValues[digitMapIndex];
			}
		}
	}
	OutputDebugStringW(decodedChars);
}

void HostInfo_HelperMethods::DecodeProductKeyWin8AndUp(byte* productkey) {
	std::wstring key;
	const int keyOffset = 52;
	byte isWin8 = ((productkey[66] / 6) & 1);
	productkey[66] = ((productkey[66] & 0xf7) | (isWin8 & 2) * 4);

	const wchar_t keyValues[] = L"BCDFGHJKMPQRTVWXY2346789";
	int last = 0;

	for (size_t i = 24; i != -1; i--)
	{
		int current = 0;
		for (size_t j = 14; j != -1; j--) {
			current = current * 256;
			current = productkey[j + keyOffset] + current;
			productkey[j + keyOffset] = (byte)(current / 24);
			current = current % 24;
			last = current;
		}
		key = keyValues[current] + key;
	}
	delete[] productkey;

	std::wstring keypart1(key.substr(1, last));
	std::wstring keypart2(key.substr(last + 1, key.length() - (last + 1)));
	key = keypart1 + L"N" + keypart2;

	for (size_t i = 5; i < key.length(); i += 6) {
		key = key.insert(i, L"-");
	}

	OutputDebugStringW(key.c_str());
}



//http://blogs.msdn.com/b/oldnewthing/archive/2014/01/06/10487119.aspx

_COM_SMARTPTR_TYPEDEF(IWbemLocator, __uuidof(IWbemLocator));
_COM_SMARTPTR_TYPEDEF(IWbemServices, __uuidof(IWbemServices));
_COM_SMARTPTR_TYPEDEF(IWbemClassObject, __uuidof(IWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IEnumWbemClassObject, __uuidof(IEnumWbemClassObject));


UINT16 GetProperty(IWbemClassObject *pobj, PCWSTR pszProperty)
{
	_variant_t var;
	pobj->Get(pszProperty, 0, &var, nullptr, nullptr);
	return var.operator unsigned short();
}

void PrintProperty(IWbemClassObject *pobj, PCWSTR pszProperty)
{
	UINT16 value = GetProperty(pobj, pszProperty);

	if (value == 0) {
		OutputDebugStringW(L"Unspecified");
	}
	else if (value == 2) {
		OutputDebugStringW(L"Laptop");
	}
	else {
		OutputDebugStringW(L"Desktop");
	}

	/*Unspecified
		0
		Desktop
		1
		Mobile
		2
		Workstation
		3
		Enterprise Server
		4
		SOHO Server
		5
		Appliance PC
		6
		Performance Server
		7
		Maximum
		8*/

}


class CCoInitialize {
public:
	CCoInitialize() : m_hr(CoInitialize(NULL)) { }
	~CCoInitialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
	operator HRESULT() const { return m_hr; }
	HRESULT m_hr;
};

wchar_t * HostInfo_HelperMethods::ComputerType() {
	CCoInitialize init;

	IWbemLocatorPtr spLocator;
	CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&spLocator));

	IWbemServicesPtr spServices;
	spLocator->ConnectServer(_bstr_t(L"root\\cimv2"), nullptr, nullptr, 0, 0, nullptr, nullptr, &spServices);

	CoSetProxyBlanket(spServices, RPC_C_AUTHN_DEFAULT, RPC_C_AUTHZ_DEFAULT, COLE_DEFAULT_PRINCIPAL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE);

	IEnumWbemClassObjectPtr spEnum;
	spServices->ExecQuery(_bstr_t(L"WQL"), _bstr_t(L"select * from Win32_ComputerSystem"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &spEnum);

	IWbemClassObjectPtr spObject;
	ULONG cActual;
	if (spEnum->Next(WBEM_INFINITE, 1, &spObject, &cActual) == WBEM_S_NO_ERROR) {
		PrintProperty(spObject, L"PCSystemType");
	}
	return NULL;
}