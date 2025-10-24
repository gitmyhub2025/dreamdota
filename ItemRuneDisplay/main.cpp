/*
 * ItemRuneDisplay - 简化的魔兽物品和神符显示 DLL
 * 基于 DreamDota 项目简化
 * 支持 Warcraft III 1.24e (6401)
 */

#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <string>
#include <map>

// ==================== 类型定义 ====================
typedef uint32_t item;
typedef uint32_t player;
typedef uint32_t real;  // 实际是 float 的别名

// ==================== JASS 函数指针类型 ====================
typedef uint32_t (*JASS_GetItemTypeId_t)(item i);
typedef real (*JASS_GetItemX_t)(item i);
typedef real (*JASS_GetItemY_t)(item i);
typedef uint32_t (*JASS_GetItemType_t)(item i);
typedef bool (*JASS_IsItemPowerup_t)(item i);
typedef bool (*JASS_IsItemOwned_t)(item i);
typedef void* (*JASS_GetLocalPlayer_t)();

// ==================== 全局变量 ====================
HMODULE g_GameDll = nullptr;
DWORD g_GameBase = 0;

// JASS 函数指针 (1.24e 偏移量)
JASS_GetItemTypeId_t JASS_GetItemTypeId = nullptr;
JASS_GetItemX_t JASS_GetItemX = nullptr;
JASS_GetItemY_t JASS_GetItemY = nullptr;
JASS_GetItemType_t JASS_GetItemType = nullptr;
JASS_IsItemPowerup_t JASS_IsItemPowerup = nullptr;
JASS_IsItemOwned_t JASS_IsItemOwned = nullptr;
JASS_GetLocalPlayer_t JASS_GetLocalPlayer = nullptr;

// GameUI 显示函数指针
typedef void (__fastcall *GameUITextDisplay_t)(void* ui, int dummy, float x, float y, const char* text, float duration, int color);
GameUITextDisplay_t GameUITextDisplay = nullptr;
void** GameGlobalUI = nullptr;

// Timer
HANDLE g_ScanTimer = nullptr;
bool g_Running = false;

// 物品缓存 - 用于避免重复显示
std::map<uint32_t, bool> g_DisplayedItems;

// ==================== 常量定义 ====================
const uint32_t ITEM_TYPE_POWERUP = 2;

// 1.24e (6401) 的关键偏移量
namespace Offsets_6401 {
    const DWORD GetItemTypeId = 0x003C4C60;
    const DWORD GetItemX = 0x003C4D90;
    const DWORD GetItemY = 0x003C4DD0;
    const DWORD GetItemType = 0x003C4E70;
    const DWORD IsItemPowerup = 0x003C4FD0;
    const DWORD IsItemOwned = 0x003C4F90;
    const DWORD GetLocalPlayer = 0x003BF4C0;

    const DWORD GameUI = 0x00AB65F0;  // GameGlobalUI 指针
    const DWORD GameUITextDisplay = 0x0055D7C0;  // 文本显示函数
}

// ==================== 辅助函数 ====================

// 将整数 ID 转换为4字符字符串 (例如 'I0KK')
const char* IntegerIdToChar(uint32_t id) {
    static char buffer[5] = {0};
    buffer[0] = (char)((id >> 0) & 0xFF);
    buffer[1] = (char)((id >> 8) & 0xFF);
    buffer[2] = (char)((id >> 16) & 0xFF);
    buffer[3] = (char)((id >> 24) & 0xFF);
    buffer[4] = '\0';
    return buffer;
}

// 从游戏数据库获取物品名称
const char* GetItemName(uint32_t typeId) {
    // 简化版：直接返回 ID 字符串
    // 完整版需要从 Game.dll 的数据结构中读取名称
    return IntegerIdToChar(typeId);
}

// 显示文本到游戏屏幕
void OutputScreen(const char* text, float duration = 5.0f) {
    if (!GameGlobalUI || !*GameGlobalUI || !GameUITextDisplay) {
        return;
    }

    GameUITextDisplay(*GameGlobalUI, 0, 0.0f, 0.0f, text, duration, -1);
}

// ==================== 物品扫描逻辑 ====================

// 注意：这是简化版本
// 完整版需要使用 EnumItemsInRect 来枚举所有物品
// 这里我们使用事件监听或其他方式

// 定时器回调 - 检查物品
VOID CALLBACK TimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired) {
    if (!g_Running) return;

    // TODO: 这里需要实现物品枚举逻辑
    // 由于 JASS 的 EnumItemsInRect 需要回调机制，这部分比较复杂
    // 简化实现：通过 hook 游戏事件来监听物品创建

    // 示例输出
    static int counter = 0;
    if (counter++ % 10 == 0) {
        OutputScreen("|cffffcc00ItemRuneDisplay|r is running...");
    }
}

// ==================== 游戏事件 Hook ====================

// 物品创建事件结构
namespace war3 {
    struct CItem {
        char _padding[0x14];
        uint32_t typeId;  // 偏移可能不准确
        // ... 其他字段
    };
}

// Hook 物品创建 - 这需要找到游戏内部的物品创建函数
// 简化版：我们假设能够从某处获取物品列表

typedef void* (__fastcall *ItemCreate_t)(void* thisptr, void* edx, uint32_t typeId, float x, float y);
ItemCreate_t g_OrigItemCreate = nullptr;

void* __fastcall Hook_ItemCreate(void* thisptr, void* edx, uint32_t typeId, float x, float y) {
    // 调用原函数
    void* result = g_OrigItemCreate(thisptr, edx, typeId, x, y);

    // 检查是否为 Powerup
    if (result) {
        item itemHandle = reinterpret_cast<item>(result);

        if (JASS_IsItemPowerup && JASS_IsItemPowerup(itemHandle)) {
            // 获取信息
            uint32_t itemTypeId = JASS_GetItemTypeId ? JASS_GetItemTypeId(itemHandle) : 0;
            float itemX = JASS_GetItemX ? *(float*)&JASS_GetItemX(itemHandle) : 0.0f;
            float itemY = JASS_GetItemY ? *(float*)&JASS_GetItemY(itemHandle) : 0.0f;

            // 过滤无用道具
            if (itemTypeId == 'I0KK' || itemTypeId == 'I0HM') {
                return result;
            }

            // 显示信息
            char message[256];
            sprintf_s(message, sizeof(message),
                "|cffffcc00[神符生成]|r %s 位置: (%.0f, %.0f)",
                GetItemName(itemTypeId), itemX, itemY);
            OutputScreen(message, 10.0f);
        }
    }

    return result;
}

// ==================== 初始化函数 ====================

bool InitializeJassAPI() {
    g_GameDll = GetModuleHandleA("Game.dll");
    if (!g_GameDll) {
        MessageBoxA(nullptr, "无法找到 Game.dll", "Error", MB_OK);
        return false;
    }

    g_GameBase = (DWORD)g_GameDll;

    // 初始化 JASS 函数指针
    JASS_GetItemTypeId = (JASS_GetItemTypeId_t)(g_GameBase + Offsets_6401::GetItemTypeId);
    JASS_GetItemX = (JASS_GetItemX_t)(g_GameBase + Offsets_6401::GetItemX);
    JASS_GetItemY = (JASS_GetItemY_t)(g_GameBase + Offsets_6401::GetItemY);
    JASS_GetItemType = (JASS_GetItemType_t)(g_GameBase + Offsets_6401::GetItemType);
    JASS_IsItemPowerup = (JASS_IsItemPowerup_t)(g_GameBase + Offsets_6401::IsItemPowerup);
    JASS_IsItemOwned = (JASS_IsItemOwned_t)(g_GameBase + Offsets_6401::IsItemOwned);
    JASS_GetLocalPlayer = (JASS_GetLocalPlayer_t)(g_GameBase + Offsets_6401::GetLocalPlayer);

    // 初始化 UI 显示函数
    GameGlobalUI = (void**)(g_GameBase + Offsets_6401::GameUI);
    GameUITextDisplay = (GameUITextDisplay_t)(g_GameBase + Offsets_6401::GameUITextDisplay);

    return true;
}

bool InitializeHooks() {
    // TODO: 实现 Hook 逻辑
    // 需要使用 Detours 或 MinHook 等库来 Hook 物品创建函数
    // 这里暂时省略，因为需要找到准确的函数地址

    return true;
}

void StartMonitoring() {
    g_Running = true;

    // 创建定时器 - 每2秒执行一次
    CreateTimerQueueTimer(&g_ScanTimer, nullptr, TimerCallback,
        nullptr, 2000, 2000, WT_EXECUTEDEFAULT);

    OutputScreen("|cff00ff00ItemRuneDisplay Started!|r");
}

void StopMonitoring() {
    g_Running = false;

    if (g_ScanTimer) {
        DeleteTimerQueueTimer(nullptr, g_ScanTimer, nullptr);
        g_ScanTimer = nullptr;
    }
}

// ==================== DLL 入口点 ====================

DWORD WINAPI MainThread(LPVOID lpParam) {
    // 等待游戏初始化
    Sleep(3000);

    // 初始化
    if (!InitializeJassAPI()) {
        return 1;
    }

    if (!InitializeHooks()) {
        MessageBoxA(nullptr, "Hook 初始化失败", "Warning", MB_OK);
    }

    // 开始监控
    StartMonitoring();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
        break;

    case DLL_PROCESS_DETACH:
        StopMonitoring();
        break;
    }
    return TRUE;
}
