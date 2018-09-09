#include <windows.h>
#include <stdio.h>
#include "MinHook.h"
#include <io.h>
#include <fcntl.h>
#include "PatternScanner.h"
#include "ASI Loader/ASILoader.h"
#if defined _M_X64
#pragma comment(lib, "lib/libMinHook-x64-v141-mt.lib")
#elif defined _M_IX86
#pragma comment(lib, "libMinHook.x86.lib")
#endif

HINSTANCE mHinst = 0, mHinstDLL = 0;
extern "C" UINT_PTR mProcs[12] = {0};

void LoadOriginalDll();
void Start();
LPCSTR mImportNames[] = {"DllMain", "XInputEnable", "XInputGetBatteryInformation", "XInputGetCapabilities", "XInputGetDSoundAudioDeviceGuids", "XInputGetKeystroke", "XInputGetState", "XInputSetState", (LPCSTR)100, (LPCSTR)101, (LPCSTR)102, (LPCSTR)103};
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved ) {
	mHinst = hinstDLL;
	if ( fdwReason == DLL_PROCESS_ATTACH ) {
		LoadOriginalDll();
		for ( int i = 0; i < 12; i++ )
			mProcs[ i ] = (UINT_PTR)GetProcAddress( mHinstDLL, mImportNames[ i ] );
		Start();
	} else if ( fdwReason == DLL_PROCESS_DETACH ) {
		FreeLibrary( mHinstDLL );
	}
	return ( TRUE );
}

extern "C" void DllMain_wrapper();
extern "C" void XInputEnable_wrapper();
extern "C" void XInputGetBatteryInformation_wrapper();
extern "C" void XInputGetCapabilities_wrapper();
extern "C" void XInputGetDSoundAudioDeviceGuids_wrapper();
extern "C" void XInputGetKeystroke_wrapper();
extern "C" void XInputGetState_wrapper();
extern "C" void XInputSetState_wrapper();
extern "C" void ExportByOrdinal100();
extern "C" void ExportByOrdinal101();
extern "C" void ExportByOrdinal102();
extern "C" void ExportByOrdinal103();


// Loads the original DLL from the default system directory
//	Function originally written by Michael Koch
void LoadOriginalDll()
{
	char buffer[MAX_PATH];

	// Get path to system dir and to xinput1_3.dll
	GetSystemDirectory(buffer, MAX_PATH);

	// Append DLL name
	strcat_s(buffer, "\\xinput1_3.dll");

	// Try to load the system's xinput1_3.dll, if pointer empty
	if (!mHinstDLL) mHinstDLL = LoadLibrary(buffer);

	// Debug
	if (!mHinstDLL)
	{
		OutputDebugString("PROXYDLL: Original xinput1_3.dll not loaded ERROR ****\r\n");
		ExitProcess(0); // Exit the hard way
	}
}
void Console() {
	AllocConsole();

	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
	FILE* hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, NULL, _IONBF, 128);
	*stdin = *hf_in;
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	printf("HEYA");

	printf("BYE");
}



#define EXE_BASE 0x140000000

#define FUNCTION_InitModules 0x140276a90
#define FUNCTION_AddFunction 0x1402bba50
typedef int64_t* (__fastcall* InitModules)(int64_t scriptmoduleaddress);

typedef int64_t* (__fastcall* AddFunction)(int64_t scriptmoduleaddress, char* FunctionName, int64_t function, int32_t SomeFlag);
// Pointer for calling original MessageBoxW.
InitModules InitModulesOrig = NULL;


UINT_PTR moduleBase = NULL;
int64_t scriptmodule;

void __declspec(dllexport)  scriptRegister(char* name, int64_t function) {


	AddFunction fAddFunctions = (AddFunction)(moduleBase + (FUNCTION_AddFunction - EXE_BASE));
	fAddFunctions(scriptmodule, name, function, 0);
}

int64_t* MyInitModules(int64_t scriptmoduleaddress) {
	scriptmodule = scriptmoduleaddress;
	printf("SUP");
	printf("Init Address = %x", scriptmoduleaddress);
	ASILoader::Initialize();
	return InitModulesOrig(scriptmoduleaddress);
}
void Hook() {

	if (!moduleBase)
		moduleBase = reinterpret_cast<UINT_PTR>(GetModuleHandle(NULL));
	InitModules fInitModules = (InitModules)(moduleBase + (FUNCTION_InitModules - EXE_BASE));

	auto source = moduleBase + (FUNCTION_InitModules - EXE_BASE);
	uintptr_t ptr;
	//while (true) {
	//	try
	//	{
			//Replace Me With Pattern Scanner
			ptr = moduleBase + 0x140276a90 - EXE_BASE;//FindPattern(std::string("cc cc cc cc cc cc 48 89 6c 24 10 48 89 74 24 18") , std::string(""));
													  //ptr += 6;
	//	}
	//catch (const std::exception&)
	//{
	//	Sleep(50);
	//}

	//}
	// Initialize MinHook.
	MH_STATUS status = MH_Initialize();
	if (status != MH_OK)
	{
		printf("COULD NOT INIT %d ", status);
	}
	// Create a hook for MessageBoxW, in disabled state.
	if (MH_CreateHook((INT_PTR*)ptr, &MyInitModules,
		reinterpret_cast<LPVOID*>(&InitModulesOrig)) != MH_OK)
	{
		printf("COULD NOT HOOK");
	}
	if (MH_EnableHook((INT_PTR*)ptr) != MH_OK)
	{

		printf("COULD NOT Enable HOOK");
	}

}
void Start() {

	Console();
	Hook();
}