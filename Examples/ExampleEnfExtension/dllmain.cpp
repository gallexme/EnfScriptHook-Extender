// dllmain.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
#include "stdafx.h"
#include <stdio.h>

#include <EnfScriptHook.h>

char* HelloDayZ(char* sup) {
	printf("HELLO DAYZ: %s", sup);

	return sup;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		scriptRegister((char*)"HelloDayz", reinterpret_cast<INT64>(&HelloDayZ));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

