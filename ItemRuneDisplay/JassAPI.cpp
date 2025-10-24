/*
 * JassAPI.cpp - JASS Native 函数实现
 * 基于 DreamDota 的 native_offsets_6387.inc
 */

#include "JassAPI.h"

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
