/*
 * JassAPI.cpp - JASS Native 函数实现
 * 基于 DreamDota 的 native_offsets_6387.inc
 */

#include "JassAPI.h"
#include <map>
#include <string>

//=============================================================================
// String storage for TextTags
// War3 may read string pointers asynchronously, so we must keep them alive
//=============================================================================
static std::map<uint32_t, std::string> g_TextTagStrings;

//=============================================================================
// JASS 函数指针类型
//=============================================================================

typedef uint32_t (__fastcall *GetItemTypeId_t)(uint32_t item);
typedef float (__fastcall *GetItemX_t)(uint32_t item);
typedef float (__fastcall *GetItemY_t)(uint32_t item);
typedef uint32_t (__fastcall *GetItemType_t)(uint32_t item);
typedef bool (__fastcall *IsItemPowerup_t)(uint32_t item);
typedef bool (__fastcall *IsItemOwned_t)(uint32_t item);
typedef float (__fastcall *GetWidgetLife_t)(uint32_t widget);

// JASS type definitions (from jass_types.h)
typedef int integer;
typedef DWORD handle;
typedef DWORD string;

// TextTag function types - CRITICAL: Use __cdecl like 1.27 reference!
// NO dummy parameter - __cdecl uses stack, not registers
typedef handle (__cdecl *CreateTextTag_t)();
typedef void (__cdecl *SetTextTagText_t)(handle textTag, CJassString* text, float* height);
typedef void (__cdecl *SetTextTagPos_t)(handle textTag, float* x, float* y, float* zOffset);
typedef void (__cdecl *SetTextTagColor_t)(handle textTag, integer r, integer g, integer b, integer a);
typedef void (__cdecl *SetTextTagVisibility_t)(handle textTag, bool visible);
typedef void (__cdecl *SetTextTagSuspended_t)(handle textTag, bool suspended);
typedef void (__cdecl *SetTextTagPermanent_t)(handle textTag, bool permanent);
typedef void (__cdecl *SetTextTagLifespan_t)(handle textTag, float* lifespan);
typedef void (__cdecl *SetTextTagFadepoint_t)(handle textTag, float* fadepoint);

//=============================================================================
// War3 1.24e (6387) 偏移量
// 从 DreamDota native_offsets_6387.inc 提取
//=============================================================================

namespace Offsets {
    const DWORD GetItemTypeId  = 0x003C57A0;
    const DWORD GetItemX       = 0x003C58D0;
    const DWORD GetItemY       = 0x003C5910;
    const DWORD GetItemType    = 0x003C59B0;
    const DWORD IsItemPowerup  = 0x003C5B10;
    const DWORD IsItemOwned    = 0x003C5AD0;
    const DWORD GetWidgetLife  = 0x003C51A0;

    // TextTag offsets (from native_offsets_6387.inc)
    const DWORD CreateTextTag        = 0x003BD0C0;
    const DWORD SetTextTagText       = 0x003BD110;
    const DWORD SetTextTagPos        = 0x003BD150;
    const DWORD SetTextTagColor      = 0x003BD1E0;
    const DWORD SetTextTagVisibility = 0x003BD2A0;
    const DWORD SetTextTagSuspended  = 0x003BD2D0;
    const DWORD SetTextTagPermanent  = 0x003BD300;
    const DWORD SetTextTagLifespan   = 0x003BD360;
    const DWORD SetTextTagFadepoint  = 0x003BD390;
}

//=============================================================================
// 全局变量
//=============================================================================

static HMODULE g_hGameDll = nullptr;
static DWORD g_dwGameBase = 0;

// JASS 函数指针
static GetItemTypeId_t g_GetItemTypeId = nullptr;
static GetItemX_t g_GetItemX = nullptr;
static GetItemY_t g_GetItemY = nullptr;
static GetItemType_t g_GetItemType = nullptr;
static IsItemPowerup_t g_IsItemPowerup = nullptr;
static IsItemOwned_t g_IsItemOwned = nullptr;
static GetWidgetLife_t g_GetWidgetLife = nullptr;

// TextTag function pointers
static CreateTextTag_t g_CreateTextTag = nullptr;
static SetTextTagText_t g_SetTextTagText = nullptr;
static SetTextTagPos_t g_SetTextTagPos = nullptr;
static SetTextTagColor_t g_SetTextTagColor = nullptr;
static SetTextTagVisibility_t g_SetTextTagVisibility = nullptr;
static SetTextTagSuspended_t g_SetTextTagSuspended = nullptr;
static SetTextTagPermanent_t g_SetTextTagPermanent = nullptr;
static SetTextTagLifespan_t g_SetTextTagLifespan = nullptr;
static SetTextTagFadepoint_t g_SetTextTagFadepoint = nullptr;

//=============================================================================
// 初始化
//=============================================================================

bool Jass_Initialize() {
    // Get Game.dll
    g_hGameDll = GetModuleHandleA("Game.dll");
    if (!g_hGameDll) {
        MessageBoxA(nullptr,
            "Cannot find Game.dll!\n\n"
            "Make sure:\n"
            "1. Running inside War3 game\n"
            "2. Game version is War3 1.24e (1.24.4.6387)",
            "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    g_dwGameBase = (DWORD)g_hGameDll;

    // 初始化 JASS 函数指针
    g_GetItemTypeId = (GetItemTypeId_t)(g_dwGameBase + Offsets::GetItemTypeId);
    g_GetItemX = (GetItemX_t)(g_dwGameBase + Offsets::GetItemX);
    g_GetItemY = (GetItemY_t)(g_dwGameBase + Offsets::GetItemY);
    g_GetItemType = (GetItemType_t)(g_dwGameBase + Offsets::GetItemType);
    g_IsItemPowerup = (IsItemPowerup_t)(g_dwGameBase + Offsets::IsItemPowerup);
    g_IsItemOwned = (IsItemOwned_t)(g_dwGameBase + Offsets::IsItemOwned);
    g_GetWidgetLife = (GetWidgetLife_t)(g_dwGameBase + Offsets::GetWidgetLife);

    // Initialize TextTag function pointers
    g_CreateTextTag = (CreateTextTag_t)(g_dwGameBase + Offsets::CreateTextTag);
    g_SetTextTagText = (SetTextTagText_t)(g_dwGameBase + Offsets::SetTextTagText);
    g_SetTextTagPos = (SetTextTagPos_t)(g_dwGameBase + Offsets::SetTextTagPos);
    g_SetTextTagColor = (SetTextTagColor_t)(g_dwGameBase + Offsets::SetTextTagColor);
    g_SetTextTagVisibility = (SetTextTagVisibility_t)(g_dwGameBase + Offsets::SetTextTagVisibility);
    g_SetTextTagSuspended = (SetTextTagSuspended_t)(g_dwGameBase + Offsets::SetTextTagSuspended);
    g_SetTextTagPermanent = (SetTextTagPermanent_t)(g_dwGameBase + Offsets::SetTextTagPermanent);
    g_SetTextTagLifespan = (SetTextTagLifespan_t)(g_dwGameBase + Offsets::SetTextTagLifespan);
    g_SetTextTagFadepoint = (SetTextTagFadepoint_t)(g_dwGameBase + Offsets::SetTextTagFadepoint);

    return true;
}

DWORD Jass_GetGameBase() {
    return g_dwGameBase;
}

//=============================================================================
// JASS 函数封装
//=============================================================================

uint32_t Jass_GetItemTypeId(uint32_t item) {
    return g_GetItemTypeId ? g_GetItemTypeId(item) : 0;
}

float Jass_GetItemX(uint32_t item) {
    return g_GetItemX ? g_GetItemX(item) : 0.0f;
}

float Jass_GetItemY(uint32_t item) {
    return g_GetItemY ? g_GetItemY(item) : 0.0f;
}

uint32_t Jass_GetItemType(uint32_t item) {
    return g_GetItemType ? g_GetItemType(item) : 0;
}

bool Jass_IsItemPowerup(uint32_t item) {
    return g_IsItemPowerup ? g_IsItemPowerup(item) : false;
}

bool Jass_IsItemOwned(uint32_t item) {
    return g_IsItemOwned ? g_IsItemOwned(item) : false;
}

float Jass_GetWidgetLife(uint32_t widget) {
    return g_GetWidgetLife ? g_GetWidgetLife(widget) : 0.0f;
}

//=============================================================================
// TextTag functions
//=============================================================================

uint32_t Jass_CreateTextTag() {
    return g_CreateTextTag ? g_CreateTextTag() : 0;
}

void Jass_SetTextTagText(uint32_t textTag, CJassString* jassStr, float height) {
    if (g_SetTextTagText && textTag && jassStr) {
        // Pass CJassString pointer directly - War3 expects this structure, not a cast to DWORD
        g_SetTextTagText(textTag, jassStr, &height);
    }
}

void Jass_SetTextTagPos(uint32_t textTag, float x, float y, float zOffset) {
    if (g_SetTextTagPos && textTag) {
        // JASS expects float pointers (__cdecl calling convention)
        g_SetTextTagPos(textTag, &x, &y, &zOffset);
    }
}

void Jass_SetTextTagColor(uint32_t textTag, int r, int g, int b, int a) {
    if (g_SetTextTagColor && textTag) {
        g_SetTextTagColor(textTag, r, g, b, a);
    }
}

void Jass_SetTextTagVisibility(uint32_t textTag, bool visible) {
    if (g_SetTextTagVisibility && textTag) {
        g_SetTextTagVisibility(textTag, visible);
    }
}

void Jass_SetTextTagSuspended(uint32_t textTag, bool suspended) {
    if (g_SetTextTagSuspended && textTag) {
        g_SetTextTagSuspended(textTag, suspended);
    }
}

void Jass_SetTextTagPermanent(uint32_t textTag, bool permanent) {
    if (g_SetTextTagPermanent && textTag) {
        g_SetTextTagPermanent(textTag, permanent);
    }
}

void Jass_SetTextTagLifespan(uint32_t textTag, float lifespan) {
    if (g_SetTextTagLifespan && textTag) {
        g_SetTextTagLifespan(textTag, &lifespan);
    }
}

void Jass_SetTextTagFadepoint(uint32_t textTag, float fadepoint) {
    if (g_SetTextTagFadepoint && textTag) {
        g_SetTextTagFadepoint(textTag, &fadepoint);
    }
}
