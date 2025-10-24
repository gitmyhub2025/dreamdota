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

struct ItemHashNode {
    uint32_t key;           // 物品句柄
    void* pItem;            // CItem*
    ItemHashNode* next;     // 链表
};

struct ItemHashTable {
    ItemHashNode** buckets; // 哈希桶数组
    uint32_t capacity;      // 容量
    uint32_t count;         // 物品数量
};

// War3 1.24e (6387) 物品哈希表偏移
const DWORD OFFSET_ItemHashTable = 0x00AB4F84;

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
    // DISABLED: Hash table enumeration causes crashes
    // The hash table offset (0xAB4F84) cannot be confirmed from dreamdota source
    //
    // Alternative solutions:
    // 1. Use the JASS script version (ItemRuneMonitor.j) which uses EnumItemsInRect
    // 2. Wait for future implementation using JASS API callbacks
    //
    // For now, this function does nothing to prevent crashes

    /* Original implementation - DISABLED
    DWORD gameBase = Jass_GetGameBase();
    if (!gameBase) return;

    ItemHashTable** ppItemHashTable = (ItemHashTable**)(gameBase + OFFSET_ItemHashTable);
    if (!ppItemHashTable || !*ppItemHashTable) {
        return;
    }

    ItemHashTable* pHashTable = *ppItemHashTable;
    if (!pHashTable->buckets || pHashTable->count == 0) {
        return;
    }

    for (uint32_t i = 0; i < pHashTable->capacity; i++) {
        ItemHashNode* node = pHashTable->buckets[i];
        while (node) {
            ProcessItem(node->key);
            node = node->next;
        }
    }
    */
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
