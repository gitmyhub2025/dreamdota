/*
 * ItemMonitor_Impl.cpp - 物品监控核心实现
 * 基于 DreamDota 的物品枚举和显示逻辑
 */

#include "ItemMonitor.h"
#include "JassAPI.h"
#include "Utils.h"
#include <set>

//=============================================================================
// 全局变量
//=============================================================================

static bool g_bRunning = false;
static bool g_bInGame = false;  // 游戏状态标志
static HANDLE g_hTimerQueue = nullptr;
static HANDLE g_hTimer = nullptr;
static std::set<uint32_t> g_DisplayedItems;

//=============================================================================
// 物品哈希表结构（从 DreamDota 提取）
//=============================================================================

// 实际上 War3 使用数组结构，不是哈希表！
// 参考成功的 Python 实现

// War3 1.24e (6387) 物品数组偏移（从 Python 代码验证）
const DWORD OFFSET_GlobalClass = 0x00ACBDD8;  // GlobalClass
const DWORD OFFSET_UnitClass    = 0x3BC;      // unitClass 偏移
const DWORD OFFSET_UnitDataStart = 0x604;     // unitDataStart 偏移

//=============================================================================
// 物品枚举和处理
//=============================================================================

// 物品数据结构（用于在 SEH 保护区域和 C++ 对象区域之间传递数据）
struct ItemData {
    uint32_t typeId;
    float life;
    float x;
    float y;
    bool isOwned;
    bool isPowerup;
    bool valid;
};

// 安全获取物品数据（使用 SEH 保护，无 C++ 对象）
static bool GetItemDataSafe(uint32_t itemHandle, ItemData* pData) {
    __try {
        pData->typeId = Jass_GetItemTypeId(itemHandle);
        if (pData->typeId == 0) {
            pData->valid = false;
            return false;
        }

        pData->life = Jass_GetWidgetLife(itemHandle);
        if (pData->life <= 0.0f) {
            pData->valid = false;
            return false;
        }

        pData->isOwned = Jass_IsItemOwned(itemHandle);
        if (pData->isOwned) {
            pData->valid = false;
            return false;
        }

        pData->isPowerup = Jass_IsItemPowerup(itemHandle);
        if (!pData->isPowerup) {
            pData->valid = false;
            return false;
        }

        pData->x = Jass_GetItemX(itemHandle);
        pData->y = Jass_GetItemY(itemHandle);
        pData->valid = true;
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        pData->valid = false;
        return false;
    }
}

void ProcessItem(uint32_t itemHandle) {
    // 检查是否已显示
    if (g_DisplayedItems.count(itemHandle)) {
        return;
    }

    // 安全获取物品数据
    ItemData data = {0};
    if (!GetItemDataSafe(itemHandle, &data) || !data.valid) {
        return;
    }

    // 过滤无用道具（从 DreamDota RuneNotify.cpp）
    if (data.typeId == 'I0KK' || data.typeId == 'I0HM' ||
        data.typeId == 'KK0I' || data.typeId == 'MH0I') {
        g_DisplayedItems.insert(itemHandle);
        return;
    }

    // Display powerup info
    char message[512];
    sprintf_s(message, sizeof(message),
        "|cffffcc00[Powerup]|r %s @ (%.0f, %.0f)",
        Utils_IntegerIdToString(data.typeId).c_str(), data.x, data.y);

    Utils_OutputToScreen(message, 10.0f);

    // 标记为已显示
    g_DisplayedItems.insert(itemHandle);
}

void EnumerateAllItems() {
    // Based on successful Python implementation
    // War3 uses array structure, not hash table!

    DWORD gameBase = Jass_GetGameBase();
    if (!gameBase) return;

    __try {
        // Step 1: Get GlobalClass pointer
        DWORD gcAddr = gameBase + OFFSET_GlobalClass;
        DWORD gc = *(DWORD*)gcAddr;
        if (gc == 0) return;

        // Step 2: Get unitClass pointer
        DWORD ucAddr = gc + OFFSET_UnitClass;
        DWORD uc = *(DWORD*)ucAddr;
        if (uc == 0) return;

        // Step 3: Add 0x10 offset
        DWORD ua = uc + 0x10;

        // Step 4: Read count and array pointer
        DWORD countAddr = ua + OFFSET_UnitDataStart;
        DWORD count = *(DWORD*)countAddr;
        DWORD arrayAddr = ua + OFFSET_UnitDataStart + 4;
        DWORD array = *(DWORD*)arrayAddr;

        if (array == 0 || count == 0 || count > 10000) {
            return;  // Safety check
        }

        // Step 5: Iterate through array
        for (DWORD i = 0; i < count; i++) {
            DWORD itemPtrAddr = array + 4 * i;
            DWORD itemPtr = *(DWORD*)itemPtrAddr;

            if (itemPtr == 0) continue;

            // itemPtr is actually the handle we need!
            // Process this item using JASS functions
            ProcessItem(itemPtr);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        // Ignore errors silently
    }
}

//=============================================================================
// 定时器回调
//=============================================================================

VOID CALLBACK TimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired) {
    if (!g_bRunning) return;
    EnumerateAllItems();
}

//=============================================================================
// 公共接口实现
//=============================================================================

bool ItemMonitor_Initialize() {
    // 初始化 JASS API
    if (!Jass_Initialize()) {
        return false;
    }

    // Initialize UI system
    if (!Utils_InitializeUI()) {
        MessageBoxA(nullptr,
            "UI system initialization failed!\n\n"
            "Possible reasons:\n"
            "1. Game.dll not loaded\n"
            "2. Wrong War3 version (need 1.24e)",
            "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

void ItemMonitor_Start() {
    if (g_bRunning) return;

    g_bRunning = true;
    g_bInGame = true;  // 标记进入游戏
    g_DisplayedItems.clear();

    // Create timer queue
    g_hTimerQueue = CreateTimerQueue();
    if (!g_hTimerQueue) {
        Utils_OutputToScreen("|cffff0000Monitor start failed!|r");
        return;
    }

    // Create timer - runs every 1 second
    if (!CreateTimerQueueTimer(&g_hTimer, g_hTimerQueue, TimerCallback,
        nullptr, 1000, 1000, WT_EXECUTEDEFAULT)) {
        DeleteTimerQueue(g_hTimerQueue);
        g_hTimerQueue = nullptr;
        Utils_OutputToScreen("|cffff0000Timer creation failed!|r");
        return;
    }

    Utils_OutputToScreen("|cff00ff00ItemRuneDisplay started! War3 1.24e (6387)|r", 5.0f);
}

void ItemMonitor_Stop() {
    if (!g_bRunning) return;

    g_bRunning = false;
    g_bInGame = false;  // 标记离开游戏

    if (g_hTimer && g_hTimerQueue) {
        DeleteTimerQueueTimer(g_hTimerQueue, g_hTimer, INVALID_HANDLE_VALUE);
        g_hTimer = nullptr;
    }

    if (g_hTimerQueue) {
        DeleteTimerQueue(g_hTimerQueue);
        g_hTimerQueue = nullptr;
    }

    g_DisplayedItems.clear();

    Utils_OutputToScreen("|cffffff00ItemRuneDisplay stopped.|r", 3.0f);
}

void ItemMonitor_Cleanup() {
    ItemMonitor_Stop();
}

bool ItemMonitor_IsInGame() {
    return g_bInGame;
}
