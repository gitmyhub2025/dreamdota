/*
 * ItemRuneDisplay_Full.cpp - 完整的物品和神符显示DLL
 * 支持 Warcraft III 1.24e (版本号 6401)
 * 编译：使用 Visual Studio 2010+ (32位)
 */

#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <string>
#include <set>
#include <map>

//=============================================================================
// 类型定义
//=============================================================================

typedef uint32_t item;
typedef uint32_t player;

// JASS 函数指针类型
typedef uint32_t (__fastcall *GetItemTypeId_t)(item i);
typedef float (__fastcall *GetItemX_t)(item i);
typedef float (__fastcall *GetItemY_t)(item i);
typedef uint32_t (__fastcall *GetItemType_t)(item i);
typedef bool (__fastcall *IsItemPowerup_t)(item i);
typedef bool (__fastcall *IsItemOwned_t)(item i);
typedef float (__fastcall *GetWidgetLife_t)(item widget);

//=============================================================================
// 全局变量
//=============================================================================

// 模块句柄
HMODULE g_hGameDll = nullptr;
DWORD g_dwGameBase = 0;

// JASS API 函数指针
GetItemTypeId_t g_GetItemTypeId = nullptr;
GetItemX_t g_GetItemX = nullptr;
GetItemY_t g_GetItemY = nullptr;
GetItemType_t g_GetItemType = nullptr;
IsItemPowerup_t g_IsItemPowerup = nullptr;
IsItemOwned_t g_IsItemOwned = nullptr;
GetWidgetLife_t g_GetWidgetLife = nullptr;

// UI 函数
typedef void (__fastcall *GameUI_DisplayText_t)(void* ui, int, float x, float y, const char* text, float duration, int color);
GameUI_DisplayText_t g_GameUI_DisplayText = nullptr;
void** g_ppGameGlobalUI = nullptr;

// 监控状态
bool g_bRunning = false;
HANDLE g_hTimerQueue = nullptr;
HANDLE g_hTimer = nullptr;

// 已显示物品缓存
std::set<uint32_t> g_DisplayedItems;

// 常量
const uint32_t ITEM_TYPE_POWERUP = 2;

//=============================================================================
// War3 1.24e (6401) 偏移量
//=============================================================================

namespace War3Offsets {
    // JASS Native 函数
    const DWORD GetItemTypeId  = 0x003C4C60;
    const DWORD GetItemX       = 0x003C4D90;
    const DWORD GetItemY       = 0x003C4DD0;
    const DWORD GetItemType    = 0x003C4E70;
    const DWORD IsItemPowerup  = 0x003C4FD0;
    const DWORD IsItemOwned    = 0x003C4F90;
    const DWORD GetWidgetLife  = 0x003C30A0;

    // UI 相关
    const DWORD GameGlobalUI        = 0x00AB65F0;  // 指向 UI 对象的指针
    const DWORD GameUI_DisplayText  = 0x0055D7C0;  // 显示文本函数

    // 物品哈希表 (用于枚举所有物品)
    const DWORD ItemHashTable  = 0x00AB4F84;  // CItemHashTable*
}

//=============================================================================
// 辅助函数
//=============================================================================

// 将 4字节整数 ID 转换为字符串 (如 'afac' -> "afac")
std::string IntegerIdToString(uint32_t id) {
    char buffer[5] = {0};
    buffer[0] = (char)((id >> 0) & 0xFF);
    buffer[1] = (char)((id >> 8) & 0xFF);
    buffer[2] = (char)((id >> 16) & 0xFF);
    buffer[3] = (char)((id >> 24) & 0xFF);
    return std::string(buffer);
}

// 显示文本到游戏屏幕
void OutputToScreen(const char* text, float duration = 5.0f) {
    if (!g_ppGameGlobalUI || !*g_ppGameGlobalUI || !g_GameUI_DisplayText) {
        return;
    }

    __try {
        g_GameUI_DisplayText(*g_ppGameGlobalUI, 0, 0.0f, 0.0f, text, duration, -1);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        // 忽略错误
    }
}

// 获取物品名称（从数据库）
// 简化版本：直接返回 ID 字符串
std::string GetItemName(uint32_t typeId) {
    // TODO: 可以从游戏的 profile 数据库中读取真实名称
    // 这需要调用 Game.dll 中的数据查询函数

    // 简化实现：直接返回 ID
    return IntegerIdToString(typeId);
}

//=============================================================================
// 物品监控核心
//=============================================================================

// 物品哈希表结构 (简化)
struct ItemHashNode {
    uint32_t key;           // 物品句柄
    void* pItem;            // CItem*
    ItemHashNode* next;     // 链表
};

struct ItemHashTable {
    ItemHashNode** buckets; // 哈希桶数组
    uint32_t capacity;      // 容量
    uint32_t count;         // 物品数量
    // ...
};

// 枚举所有物品
void EnumerateAllItems() {
    if (!g_dwGameBase) return;

    // 获取物品哈希表
    ItemHashTable** ppItemHashTable = (ItemHashTable**)(g_dwGameBase + War3Offsets::ItemHashTable);
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
            uint32_t itemHandle = node->key;

            // 检查是否已显示
            if (g_DisplayedItems.count(itemHandle)) {
                node = node->next;
                continue;
            }

            // 获取物品信息
            __try {
                uint32_t typeId = g_GetItemTypeId ? g_GetItemTypeId(itemHandle) : 0;
                if (typeId == 0) {
                    node = node->next;
                    continue;
                }

                // 检查物品是否存活
                float life = g_GetWidgetLife ? g_GetWidgetLife(itemHandle) : 0.0f;
                if (life <= 0.0f) {
                    node = node->next;
                    continue;
                }

                // 检查是否被拾取
                bool isOwned = g_IsItemOwned ? g_IsItemOwned(itemHandle) : false;
                if (isOwned) {
                    node = node->next;
                    continue;
                }

                // 检查是否为神符
                bool isPowerup = g_IsItemPowerup ? g_IsItemPowerup(itemHandle) : false;
                if (isPowerup) {
                    // 过滤无用道具
                    if (typeId == 'I0KK' || typeId == 'I0HM' ||
                        typeId == 'KK0I' || typeId == 'MH0I') {
                        g_DisplayedItems.insert(itemHandle);
                        node = node->next;
                        continue;
                    }

                    // 获取位置
                    float x = g_GetItemX ? g_GetItemX(itemHandle) : 0.0f;
                    float y = g_GetItemY ? g_GetItemY(itemHandle) : 0.0f;

                    // 显示神符信息
                    char message[512];
                    sprintf_s(message, sizeof(message),
                        "|cffffcc00[神符]|r %s @ (%.0f, %.0f)",
                        GetItemName(typeId).c_str(), x, y);

                    OutputToScreen(message, 10.0f);

                    // 标记为已显示
                    g_DisplayedItems.insert(itemHandle);
                }
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                // 忽略错误
            }

            node = node->next;
        }
    }
}

// 定时器回调
VOID CALLBACK TimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired) {
    if (!g_bRunning) return;

    EnumerateAllItems();
}

//=============================================================================
// 初始化和清理
//=============================================================================

bool InitializeAPI() {
    // 获取 Game.dll
    g_hGameDll = GetModuleHandleA("Game.dll");
    if (!g_hGameDll) {
        MessageBoxA(nullptr, "无法找到 Game.dll!\n请确保在游戏中运行。", "错误", MB_OK | MB_ICONERROR);
        return false;
    }

    g_dwGameBase = (DWORD)g_hGameDll;

    // 初始化 JASS 函数指针
    g_GetItemTypeId = (GetItemTypeId_t)(g_dwGameBase + War3Offsets::GetItemTypeId);
    g_GetItemX = (GetItemX_t)(g_dwGameBase + War3Offsets::GetItemX);
    g_GetItemY = (GetItemY_t)(g_dwGameBase + War3Offsets::GetItemY);
    g_GetItemType = (GetItemType_t)(g_dwGameBase + War3Offsets::GetItemType);
    g_IsItemPowerup = (IsItemPowerup_t)(g_dwGameBase + War3Offsets::IsItemPowerup);
    g_IsItemOwned = (IsItemOwned_t)(g_dwGameBase + War3Offsets::IsItemOwned);
    g_GetWidgetLife = (GetWidgetLife_t)(g_dwGameBase + War3Offsets::GetWidgetLife);

    // 初始化 UI 函数
    g_ppGameGlobalUI = (void**)(g_dwGameBase + War3Offsets::GameGlobalUI);
    g_GameUI_DisplayText = (GameUI_DisplayText_t)(g_dwGameBase + War3Offsets::GameUI_DisplayText);

    return true;
}

void StartMonitoring() {
    if (g_bRunning) return;

    g_bRunning = true;
    g_DisplayedItems.clear();

    // 创建定时器队列
    g_hTimerQueue = CreateTimerQueue();
    if (!g_hTimerQueue) {
        OutputToScreen("|cffff0000启动监控失败！|r");
        return;
    }

    // 创建定时器 - 每1秒执行一次
    if (!CreateTimerQueueTimer(&g_hTimer, g_hTimerQueue, TimerCallback,
        nullptr, 1000, 1000, WT_EXECUTEDEFAULT)) {
        DeleteTimerQueue(g_hTimerQueue);
        g_hTimerQueue = nullptr;
        OutputToScreen("|cffff0000创建定时器失败！|r");
        return;
    }

    OutputToScreen("|cff00ff00物品和神符监控已启动！|r");
}

void StopMonitoring() {
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

    OutputToScreen("|cffffff00物品和神符监控已停止。|r");
}

//=============================================================================
// DLL 主线程
//=============================================================================

DWORD WINAPI MainThread(LPVOID lpParam) {
    // 等待游戏初始化完成
    Sleep(5000);

    // 初始化 API
    if (!InitializeAPI()) {
        return 1;
    }

    // 开始监控
    StartMonitoring();

    return 0;
}

//=============================================================================
// DLL 入口点
//=============================================================================

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        break;

    case DLL_PROCESS_DETACH:
        StopMonitoring();
        break;
    }

    return TRUE;
}
