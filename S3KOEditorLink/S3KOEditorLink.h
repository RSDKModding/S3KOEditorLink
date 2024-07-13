#ifndef S3KOEDITORLINK_H
#define S3KOEDITORLINK_H
#include "Game.h"
#include <Helpers.h>

#define ORIGINS_ADDR(offset) ((void*)((intptr_t)OriginsModule + offset))

// Module
extern HMODULE LinkModule;
extern HMODULE OriginsModule;

#endif /* S3KOEDITORLINK_H */