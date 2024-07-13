#include "S3KOEditorLink.h"

#define WRITE_BYTE(offset, val) WRITE_MEMORY(ORIGINS_ADDR(offset), (char)val)
#define INSTALL_HOOK_VAR(functionName, addr) original##functionName = addr; INSTALL_HOOK(functionName);
#define WRITE_POINTER(location, pointer) \
	{ \
		DWORD oldProtect; \
		VirtualProtect((void*)location, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect); \
		*(void**)location = pointer; \
		VirtualProtect((void*)location, sizeof(void*), oldProtect, NULL); \
	}

// Hooks
HOOK(void*, __fastcall, AllocateHeap, NULL, size_t size)
{
    return malloc(size);
}
HOOK(void, __fastcall, FreeHeap, NULL, void* buffer)
{
    free(buffer);
}

HOOK(void, __fastcall, DrawDebugString, NULL, Vector2 pos, const char* format, ...)
{
    // Start processing variable arguments
    va_list args;
    va_start(args, format);
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    vsnprintf(buffer, sizeof(buffer), format, args);

    // RSDK start
    uint16 ani = RSDK.LoadSpriteAnimation("Dev/Font.bin", SCOPE_STAGE);
    Animator animator;
    int32 startX = pos.x;

    for (int i = 0; buffer[i]; ++i)
    {
        char chr = buffer[i];
        if (chr == '\n')
        {
            pos.x = startX;
            pos.y += TO_FIXED(8);
            continue;
        }

        RSDK.SetSpriteAnimation(ani, 0, &animator, true, chr);
        RSDK.DrawSprite(&animator, &pos, false);
        pos.x += TO_FIXED(8);
    }

    // Clean up variable arguments
    va_end(args);
}

HOOK(void, __fastcall, Platform_Create, NULL, void* self, void* data)
{
    if (SceneInfo->inEditor && *((uint8*)ORIGINS_ADDR(0x1A54FA)) != 0xE9)
        WRITE_MEMORY(ORIGINS_ADDR(0x1A54FA), (char)0xE9, (char)0xD0, (char)0x02, (char)0x00, (char)0x00, (char)0x90);

    originalPlatform_Create(self, data);
}


void ApplyPatches()
{
    if (!OriginsModule)
        return;

    // Hooks
    INSTALL_HOOK_VAR(AllocateHeap, ORIGINS_ADDR(0xA79040));
    INSTALL_HOOK_VAR(FreeHeap, ORIGINS_ADDR(0xA79000));
    
    INSTALL_HOOK_VAR(DrawDebugString, ORIGINS_ADDR(0x184870));
    INSTALL_HOOK_VAR(Platform_Create, ORIGINS_ADDR(0x1A4D70));

    // Imports (IAT?)
    WRITE_POINTER((OriginsModule + 0xAA54B8), GetSystemTimeAsFileTime);

    // Object patches
    WRITE_BYTE(0x03DFDA, 0x51); // ThreePath_EditorLoad
    WRITE_BYTE(0x0438B5, 0x51); // S3K_EndingSetup_StaticLoad
    WRITE_BYTE(0x02BB30, 0x51); // S3K_FBZSetup_StaticLoad
    WRITE_BYTE(0x1C0CD0, 0xC3); // BoundsMarker_StageLoad
    WRITE_BYTE(0x1AF600, 0xC3); // PlatformControl_StageLoad
    WRITE_BYTE(0x1A3340, 0xC3); // GenericTrigger_StageLoad
    WRITE_BYTE(0x279A40, 0xC3); // This object needs attention
}
