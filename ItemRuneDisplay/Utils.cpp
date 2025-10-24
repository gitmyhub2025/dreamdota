/*
 * Utils.cpp - 辅助函数实现
 * 基于 DreamDota 的 Tools.cpp
 */

#include "Utils.h"
#include "JassAPI.h"
#include "ItemMonitor.h"

//=============================================================================
// UI 显示函数类型
//=============================================================================

typedef void (__fastcall *GameUI_DisplayText_t)(void* ui, int, float x, float y, const char* text, float duration, int color);

//=============================================================================
// War3 1.24e (6387) UI 偏移量
// 从 DreamDota Offsets.cpp 提取
//=============================================================================

namespace UIOffsets {
    const DWORD GameGlobalUI        = 0x00ACBDD8;  // 全局 UI 对象指针 (修正)
    const DWORD GameUI_DisplayText  = 0x002F9980;  // UI 文本显示函数
}

//=============================================================================
// 全局变量
//=============================================================================

static void** g_ppGameGlobalUI = nullptr;
static GameUI_DisplayText_t g_GameUI_DisplayText = nullptr;

//=============================================================================
// ID 转换
//=============================================================================

std::string Utils_IntegerIdToString(uint32_t id) {
    char buffer[5] = {0};
    buffer[0] = (char)((id >> 0) & 0xFF);
    buffer[1] = (char)((id >> 8) & 0xFF);
    buffer[2] = (char)((id >> 16) & 0xFF);
    buffer[3] = (char)((id >> 24) & 0xFF);
    return std::string(buffer);
}

//=============================================================================
// UI 初始化
//=============================================================================

bool Utils_InitializeUI() {
    DWORD gameBase = Jass_GetGameBase();
    if (!gameBase) return false;

    g_ppGameGlobalUI = (void**)(gameBase + UIOffsets::GameGlobalUI);
    g_GameUI_DisplayText = (GameUI_DisplayText_t)(gameBase + UIOffsets::GameUI_DisplayText);

    return (g_ppGameGlobalUI != nullptr && g_GameUI_DisplayText != nullptr);
}

//=============================================================================
// 屏幕输出
//=============================================================================

void Utils_OutputToScreen(const char* text, float duration) {
    // 检查游戏状态和 UI 初始化
    if (!ItemMonitor_IsInGame() || !g_ppGameGlobalUI || !g_GameUI_DisplayText) {
        return;
    }

    __try {
        if (*g_ppGameGlobalUI) {
            g_GameUI_DisplayText(*g_ppGameGlobalUI, 0, 0.0f, 0.0f, text, duration, -1);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        // 忽略错误
    }
}
