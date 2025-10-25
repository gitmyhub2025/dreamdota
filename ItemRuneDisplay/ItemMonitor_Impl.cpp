/*
 * ItemMonitor_Impl.cpp - 物品监控核心实现
 * 基于 DreamDota 的物品枚举和显示逻辑
 */

#include "ItemMonitor.h"
#include "JassAPI.h"
#include "Utils.h"
#include <set>
#include <map>
#include <memory>

//=============================================================================
// 全局变量
//=============================================================================

static bool g_bRunning = false;
static bool g_bInGame = false;  // 游戏状态标志
static HANDLE g_hTimerQueue = nullptr;
static HANDLE g_hTimer = nullptr;
static std::set<uint32_t> g_DisplayedItems;

// Store TextTag handles for each item (objPtr -> textTag handle)
static std::map<DWORD, uint32_t> g_ItemTextTags;

// Store JassString objects for each item (must stay alive with TextTag!)
static std::map<DWORD, std::unique_ptr<JassString>> g_ItemJassStrings;

//=============================================================================
// 物品对象结构（从 Python 代码提取）
//=============================================================================

// 实际上 War3 使用数组结构，数组中存储的是对象指针，不是 handle！
// 需要直接从内存读取属性

// War3 1.24e (6387) 物品数组偏移（从 Python 代码验证）
const DWORD OFFSET_GlobalClass   = 0x00ACBDD8;  // GlobalClass
const DWORD OFFSET_UnitClass     = 0x3BC;       // unitClass 偏移
const DWORD OFFSET_UnitDataStart = 0x604;       // unitDataStart 偏移

// 物品对象内部偏移（从 Python 代码提取）
const DWORD OBJ_FLAGS    = 0x20;  // flags (& 1 = deleted)
const DWORD OBJ_INFO     = 0x28;  // info pointer
const DWORD OBJ_TYPEID   = 0x30;  // 4-char type ID
const DWORD OBJ_HP       = 0x58;  // health points

// Info 结构偏移
const DWORD INFO_X       = 0x88;  // x coordinate
const DWORD INFO_Y       = 0x8C;  // y coordinate

//=============================================================================
// 物品枚举和处理
//=============================================================================

// POD structure for item data (no C++ objects)
struct ItemObjectData {
    uint32_t typeId;
    float x;
    float y;
    float hp;
    DWORD flags;
    bool valid;
};

// Safe memory read with SEH (no C++ objects)
static bool ReadItemObjectSafe(DWORD objPtr, ItemObjectData* pData) {
    __try {
        // Read flags (Python: flags = self.pm.read_int(ptr + 0x20))
        pData->flags = *(DWORD*)(objPtr + OBJ_FLAGS);
        if ((pData->flags & 1) != 0) {
            pData->valid = false;
            return false;  // Item is deleted
        }

        // Read HP (Python: hp = self.pm.read_float(ptr + 0x58))
        pData->hp = *(float*)(objPtr + OBJ_HP);
        if (pData->hp <= 0.0f) {
            pData->valid = false;
            return false;  // Item is dead
        }

        // Read typeId (Python: id4 = self.pm.read_uint(ptr + 0x30))
        pData->typeId = *(uint32_t*)(objPtr + OBJ_TYPEID);
        if (pData->typeId == 0) {
            pData->valid = false;
            return false;
        }

        // Read position (Python: info + 0x88/0x8C)
        DWORD infoPtr = *(DWORD*)(objPtr + OBJ_INFO);
        if (infoPtr == 0) {
            pData->valid = false;
            return false;
        }

        pData->x = *(float*)(infoPtr + INFO_X);
        pData->y = *(float*)(infoPtr + INFO_Y);
        pData->valid = true;
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        pData->valid = false;
        return false;
    }
}

// Process item object (can use C++ objects safely)
void ProcessItemObject(DWORD objPtr) {
    // Check if already displayed
    if (g_DisplayedItems.count(objPtr)) {
        return;
    }

    // Read item data safely
    ItemObjectData data = {0};
    if (!ReadItemObjectSafe(objPtr, &data) || !data.valid) {
        return;
    }

    // Filter useless items (from DreamDota RuneNotify.cpp)
    if (data.typeId == 'I0KK' || data.typeId == 'I0HM' ||
        data.typeId == 'KK0I' || data.typeId == 'MH0I') {
        g_DisplayedItems.insert(objPtr);
        return;
    }

    // Display powerup info
    char message[512];
    sprintf_s(message, sizeof(message),
        "|cffffcc00[Powerup]|r %s @ (%.0f, %.0f)",
        Utils_IntegerIdToString(data.typeId).c_str(), data.x, data.y);

    Utils_OutputToScreen(message, 10.0f);

    // Create TextTag at item position to display item ID
    // Use JassString structure like working reference code
    uint32_t textTag = Jass_CreateTextTag();
    if (textTag != 0) {
        // Get item ID string with color code
        std::string itemId = Utils_IntegerIdToString(data.typeId);
        char textBuffer[64];
        sprintf_s(textBuffer, sizeof(textBuffer), "|cffffcc00%s|r", itemId.c_str());

        // Create and store JassString (must stay alive with TextTag!)
        g_ItemJassStrings[objPtr] = std::make_unique<JassString>(textBuffer);

        // Follow reference code order exactly
        float textSize = 0.046f;
        Jass_SetTextTagText(textTag, g_ItemJassStrings[objPtr]->GetJassStr(), textSize);

        Jass_SetTextTagVisibility(textTag, true);
        Jass_SetTextTagSuspended(textTag, false);

        Jass_SetTextTagPos(textTag, data.x, data.y, 10.0f);

        // Store TextTag handle for later cleanup
        g_ItemTextTags[objPtr] = textTag;

        // Debug: Confirm TextTag creation
        char debugMsg[256];
        sprintf_s(debugMsg, sizeof(debugMsg),
            "|cff00ff00[DEBUG] TextTag %u: %s at (%.0f,%.0f,+10)|r",
            textTag, itemId.c_str(), data.x, data.y);
        Utils_OutputToScreen(debugMsg, 3.0f);
    } else {
        // Debug: TextTag creation failed
        Utils_OutputToScreen("|cffff0000[DEBUG] TextTag creation failed!|r", 3.0f);
    }

    // Mark as displayed
    g_DisplayedItems.insert(objPtr);
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

            // itemPtr is the object pointer, not handle!
            // Process this item by reading memory directly
            ProcessItemObject(itemPtr);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        // Ignore errors silently
    }
}

//=============================================================================
// Update TextTag positions
//=============================================================================

void UpdateTextTags() {
    // Update positions for all tracked TextTags
    for (auto& pair : g_ItemTextTags) {
        DWORD objPtr = pair.first;
        uint32_t textTag = pair.second;

        if (!textTag) continue;

        // Read current item position from memory
        __try {
            DWORD infoPtr = *(DWORD*)(objPtr + OBJ_INFO);
            if (infoPtr == 0) continue;

            float x = *(float*)(infoPtr + INFO_X);
            float y = *(float*)(infoPtr + INFO_Y);

            // Update TextTag position
            Jass_SetTextTagPos(textTag, x, y, 100.0f);
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            // Ignore errors
        }
    }
}

//=============================================================================
// 定时器回调
//=============================================================================

VOID CALLBACK TimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired) {
    if (!g_bRunning) return;
    EnumerateAllItems();
    UpdateTextTags();  // Update all TextTag positions every frame
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
    g_ItemTextTags.clear();
    g_ItemJassStrings.clear();

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

    // Clean up all TextTags and JassStrings
    g_ItemTextTags.clear();
    g_ItemJassStrings.clear();

    g_DisplayedItems.clear();

    Utils_OutputToScreen("|cffffff00ItemRuneDisplay stopped.|r", 3.0f);
}

void ItemMonitor_Cleanup() {
    ItemMonitor_Stop();
}

bool ItemMonitor_IsInGame() {
    return g_bInGame;
}
