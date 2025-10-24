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

    // 显示神符信息
    char message[512];
    sprintf_s(message, sizeof(message),
        "|cffffcc00[神符生成]|r %s @ (%.0f, %.0f)",
        Utils_IntegerIdToString(data.typeId).c_str(), data.x, data.y);

    Utils_OutputToScreen(message, 10.0f);

    // 标记为已显示
    g_DisplayedItems.insert(itemHandle);
}

void EnumerateAllItems() {
    DWORD gameBase = Jass_GetGameBase();
    if (!gameBase) return;

    // 获取物品哈希表
    ItemHashTable** ppItemHashTable = (ItemHashTable**)(gameBase + OFFSET_ItemHashTable);
    if (!ppItemHashTable || !*ppItemHashTable) {
        return;
    }

    ItemHashTable* pHashTable = *ppItemHashTable;
    if (!pHashTable->buckets || pHashTable->count == 0) {
        return;
    }

    // 遍历哈希表
    for (uint32_t i = 0; i < pHashTable->capacity; i++) {
        ItemHashNode* node = pHashTable->buckets[i];
        while (node) {
            ProcessItem(node->key);
            node = node->next;
        }
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

    return true;
}

void ItemMonitor_Start() {
    if (g_bRunning) return;

    g_bRunning = true;
    g_DisplayedItems.clear();

    // 创建定时器队列
    g_hTimerQueue = CreateTimerQueue();
    if (!g_hTimerQueue) {
        Utils_OutputToScreen("|cffff0000启动监控失败！|r");
        return;
    }

    // 创建定时器 - 每1秒执行一次
    if (!CreateTimerQueueTimer(&g_hTimer, g_hTimerQueue, TimerCallback,
        nullptr, 1000, 1000, WT_EXECUTEDEFAULT)) {
        DeleteTimerQueue(g_hTimerQueue);
        g_hTimerQueue = nullptr;
        Utils_OutputToScreen("|cffff0000创建定时器失败！|r");
        return;
    }

    Utils_OutputToScreen("|cff00ff00ItemRuneDisplay 已启动！War3 1.24e (6387)|r", 5.0f);
}

void ItemMonitor_Stop() {
    if (!g_bRunning) return;

    g_bRunning = false;

    if (g_hTimer && g_hTimerQueue) {
        DeleteTimerQueueTimer(g_hTimerQueue, g_hTimer, INVALID_HANDLE_VALUE);
        g_hTimer = nullptr;
    }

    if (g_hTimerQueue) {
        DeleteTimerQueue(g_hTimerQueue);
        g_hTimerQueue = nullptr;
    }

    g_DisplayedItems.clear();

    Utils_OutputToScreen("|cffffff00ItemRuneDisplay 已停止。|r", 3.0f);
}

void ItemMonitor_Cleanup() {
    ItemMonitor_Stop();
}
