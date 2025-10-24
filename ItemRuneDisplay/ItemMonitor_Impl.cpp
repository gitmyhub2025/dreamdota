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

void ProcessItem(uint32_t itemHandle) {
    // 检查是否已显示
    if (g_DisplayedItems.count(itemHandle)) {
        return;
    }

    __try {
        // 获取物品类型ID
        uint32_t typeId = Jass_GetItemTypeId(itemHandle);
        if (typeId == 0) {
            return;
        }

        // 检查物品是否存活
        float life = Jass_GetWidgetLife(itemHandle);
        if (life <= 0.0f) {
            return;
        }

        // 检查是否被拾取
        bool isOwned = Jass_IsItemOwned(itemHandle);
        if (isOwned) {
            return;
        }

        // 检查是否为神符
        bool isPowerup = Jass_IsItemPowerup(itemHandle);
        if (isPowerup) {
            // 过滤无用道具（从 DreamDota RuneNotify.cpp）
            if (typeId == 'I0KK' || typeId == 'I0HM' ||
                typeId == 'KK0I' || typeId == 'MH0I') {
                g_DisplayedItems.insert(itemHandle);
                return;
            }

            // 获取位置
            float x = Jass_GetItemX(itemHandle);
            float y = Jass_GetItemY(itemHandle);

            // 显示神符信息
            char message[512];
            sprintf_s(message, sizeof(message),
                "|cffffcc00[神符生成]|r %s @ (%.0f, %.0f)",
                Utils_IntegerIdToString(typeId).c_str(), x, y);

            Utils_OutputToScreen(message, 10.0f);

            // 标记为已显示
            g_DisplayedItems.insert(itemHandle);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        // 忽略错误
    }
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
