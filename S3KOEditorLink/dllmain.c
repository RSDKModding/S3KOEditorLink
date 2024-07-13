// dllmain.cpp : Defines the entry point for the DLL application.
#include "S3KOEditorLink.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        LinkModule = hModule;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        if (OriginsModule)
            FreeLibrary(OriginsModule);
        OriginsModule = NULL;
        break;
    }

    return TRUE;
}

