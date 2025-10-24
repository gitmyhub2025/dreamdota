/*
 * ItemMonitor.cpp - 物品监控实现
 * 使用游戏事件系统来监听物品创建
 */

#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <set>

// ==================== 游戏结构体定义 ====================

namespace war3 {
    // 游戏事件基类
    struct CEvent {
        uint32_t vtable;
        uint32_t id;
        // ... 其他字段
    };

    // 物品对象
    struct CItem {
        uint32_t vtable;
        char padding1[0x0C];
        uint32_t handleId;      // 0x10
        char padding2[0x04];
        uint32_t typeId;        // 0x18
        float x;                // 0x1C
        float y;                // 0x20
        // ... 其他字段
    };

    // 游戏对象哈希表
    struct AgentHashTable {
        void** data;
        uint32_t capacity;
        uint32_t count;
        // ...
    };
}

// ==================== JASS API ====================

extern DWORD g_GameBase;

// JASS 函数类型
typedef uint32_t (*Func_GetItemTypeId)(uint32_t item);
typedef float (*Func_GetItemX)(uint32_t item);
typedef float (*Func_GetItemY)(uint32_t item);
typedef uint32_t (*Func_GetItemType)(uint32_t item);
typedef bool (*Func_IsItemPowerup)(uint32_t item);
typedef bool (*Func_IsItemOwned)(uint32_t item);
typedef float (*Func_GetWidgetLife)(uint32_t widget);

// 函数指针
Func_GetItemTypeId GetItemTypeId = nullptr;
Func_GetItemX GetItemX = nullptr;
Func_GetItemY GetItemY = nullptr;
Func_GetItemType GetItemType = nullptr;
Func_IsItemPowerup IsItemPowerup = nullptr;
Func_IsItemOwned IsItemOwned = nullptr;
Func_GetWidgetLife GetWidgetLife = nullptr;

// 外部函数声明
extern void OutputScreen(const char* text, float duration);
extern const char* GetItemNameFromDB(uint32_t typeId);
extern const char* IntegerIdToChar(uint32_t id);

// ==================== 物品管理 ====================

std::set<uint32_t> g_TrackedItems;  // 已追踪的物品

// 物品信息
struct ItemInfo {
    uint32_t handleId;
    uint32_t typeId;
    float x;
    float y;
    bool isPowerup;
    bool isOwned;
};

// 从物品句柄获取信息
bool GetItemInfo(uint32_t itemHandle, ItemInfo& info) {
    if (!itemHandle) return false;

    info.handleId = itemHandle;
    info.typeId = GetItemTypeId ? GetItemTypeId(itemHandle) : 0;
    info.x = GetItemX ? GetItemX(itemHandle) : 0.0f;
    info.y = GetItemY ? GetItemY(itemHandle) : 0.0f;
    info.isPowerup = IsItemPowerup ? IsItemPowerup(itemHandle) : false;
    info.isOwned = IsItemOwned ? IsItemOwned(itemHandle) : false;

    return info.typeId != 0;
}

// 处理物品创建事件
void OnItemCreated(uint32_t itemHandle) {
    // 检查是否已处理
    if (g_TrackedItems.count(itemHandle)) {
        return;
    }

    ItemInfo info;
    if (!GetItemInfo(itemHandle, info)) {
        return;
    }

    // 标记为已处理
    g_TrackedItems.insert(itemHandle);

    // 检查是否为神符
    if (info.isPowerup && !info.isOwned) {
        // 过滤无用道具
        if (info.typeId == 'I0KK' || info.typeId == 'I0HM' ||
            info.typeId == 'KK0I' || info.typeId == 'MH0I') {
            return;
        }

        // 显示神符信息
        char message[512];
        sprintf_s(message, sizeof(message),
            "|cffffcc00[神符生成]|r %s (ID:%s) @ (%.0f, %.0f)",
            GetItemNameFromDB(info.typeId),
            IntegerIdToChar(info.typeId),
            info.x, info.y);

        OutputScreen(message, 10.0f);
    }
    // 显示普通物品（可选）
    else if (!info.isOwned) {
        char message[512];
        sprintf_s(message, sizeof(message),
            "|cff00ff00[物品]|r %s @ (%.0f, %.0f)",
            GetItemNameFromDB(info.typeId),
            info.x, info.y);

        // 只显示5秒
        OutputScreen(message, 5.0f);
    }
}

// ==================== 游戏事件 Hook ====================

// 事件分发器
typedef void (__fastcall *EventDispatch_t)(void* dispatcher, void* edx, war3::CEvent* evt);
EventDispatch_t g_OrigEventDispatch = nullptr;

void __fastcall Hook_EventDispatch(void* dispatcher, void* edx, war3::CEvent* evt) {
    if (evt) {
        // 物品创建事件 ID (这个需要通过调试确定)
        const uint32_t EVENT_ITEM_CREATED = 0x???;  // TODO: 需要确定准确的事件ID

        if (evt->id == EVENT_ITEM_CREATED) {
            // 从事件数据中提取物品句柄
            // 注意：事件数据结构需要通过逆向确定
            uint32_t* eventData = (uint32_t*)((char*)evt + 0x08);
            uint32_t itemHandle = eventData[0];

            OnItemCreated(itemHandle);
        }
    }

    // 调用原函数
    g_OrigEventDispatch(dispatcher, edx, evt);
}

// ==================== 初始化 ====================

bool InitializeItemMonitor(DWORD gameBase) {
    g_GameBase = gameBase;

    // 初始化 JASS 函数
    GetItemTypeId = (Func_GetItemTypeId)(gameBase + 0x003C4C60);
    GetItemX = (Func_GetItemX)(gameBase + 0x003C4D90);
    GetItemY = (Func_GetItemY)(gameBase + 0x003C4DD0);
    GetItemType = (Func_GetItemType)(gameBase + 0x003C4E70);
    IsItemPowerup = (Func_IsItemPowerup)(gameBase + 0x003C4FD0);
    IsItemOwned = (Func_IsItemOwned)(gameBase + 0x003C4F90);
    GetWidgetLife = (Func_GetWidgetLife)(gameBase + 0x003C30A0);

    return true;
}

// 安装 Hook
bool InstallItemMonitorHook() {
    // TODO: 使用 MinHook 或 Detours 来 Hook EventDispatch
    // 这需要找到 EventDispatch 函数的准确地址

    return false;  // 暂时未实现
}

void CleanupItemMonitor() {
    g_TrackedItems.clear();
}
