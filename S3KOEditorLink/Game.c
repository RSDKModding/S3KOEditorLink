#include "S3KOEditorLink.h"
#include "GamePatches.h"

// RSDK
int32 RSDKRevision = RETRO_REVISION;
RSDKFunctionTable RSDK;
RSDKSceneInfo *SceneInfo = NULL;

// Handles
HMODULE LinkModule = NULL;
HMODULE OriginsModule = NULL;

HMODULE LoadLocalOrigins()
{
    if (OriginsModule)
        return OriginsModule;

    // Backup
    char oldWorkingDirectory[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, oldWorkingDirectory);
    
    char modulePath[MAX_PATH];
    if (GetModuleFileNameA(LinkModule, modulePath, MAX_PATH) == 0)
        return OriginsModule;
    
    // Path
    char* pos = strrchr(modulePath, '\\');
    if (pos == NULL)
        return OriginsModule;
    pos[0] = '\0';

    SetCurrentDirectoryA(modulePath);
    OriginsModule = LoadLibraryA("SonicOrigins.exe");
    SetCurrentDirectoryA(oldWorkingDirectory);
    return OriginsModule;
}

HMODULE LoadOriginsModule()
{
    // Check if the module is already loaded
    OriginsModule = GetModuleHandleA("SonicOrigins.exe");
    if (OriginsModule)
        return OriginsModule;
    
    // Load module
    OriginsModule = LoadLibraryA("SonicOrigins.exe");
    if (!OriginsModule)
        OriginsModule = LoadLocalOrigins();

    if (!OriginsModule)
    {
        MessageBoxA(NULL, "Failed to load SonicOrigins.exe", "Error", MB_ICONERROR | MB_OK);
        OriginsModule = NULL;
    }

    return OriginsModule;
}

void RegisterAllObjects()
{
    // Aslong as it works
    intptr_t start = (intptr_t)ORIGINS_ADDR(0xAA5C50);
    intptr_t end   = (intptr_t)ORIGINS_ADDR(0xAAD0F8);
    while (start < end)
    {
        if (*(void**)start)
        {   
            if ((*(unsigned short**)start)[0] == 0x158B &&
                (*(unsigned short**)start)[3] == 0xFA81 &&
                (*(unsigned short**)start)[4] == 0x0400 &&
                (*(unsigned short**)start)[5] == 0x0000)
                ((void(__fastcall*)())((*(void**)start)))();
        }
        start += sizeof(void*);
    }
}

void LinkGameLogicDLL(EngineInfo* info)
{
    // Engine
    memset(&RSDK, 0, sizeof(RSDKFunctionTable));
    if (info->functionTable)
        memcpy(&RSDK, info->functionTable, sizeof(RSDKFunctionTable));
    SceneInfo = info->sceneInfo;

    if (LoadOriginsModule())
    {
        // Globals
        ((void(__fastcall*)(void*, int, void*))(ORIGINS_ADDR(0xAD730)))(ORIGINS_ADDR(0x4000210), 0x4C35B0, ORIGINS_ADDR(0x3A86E0));

        // Register objects
        int32 objectCount = *(int32*)(ORIGINS_ADDR(0x28BDC78));
        if (objectCount == 0)
        {
            ApplyPatches();
            RegisterAllObjects();
        }

        // Call link
        ((void(__fastcall*)(EngineInfo*))((EngineInfo*)ORIGINS_ADDR(0xAD750)))(info);
    }
}