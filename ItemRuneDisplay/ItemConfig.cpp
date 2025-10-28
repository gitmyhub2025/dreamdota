/*
 * ItemConfig.cpp - 物品配置实现
 */

#include "ItemConfig.h"

//=============================================================================
// 静态成员初始化
//=============================================================================
DebugMode ItemConfig::s_debugMode = DebugMode::ALL;  // 默认显示所有// 或 FILTERED, NONE
std::map<std::string, std::string> ItemConfig::s_itemNames;
std::set<std::string> ItemConfig::s_filterList;
float ItemConfig::s_textTagSize = 0.024f;  // 调小字体大小
int ItemConfig::s_textTagColor[4] = {255, 204, 0, 255};  // 默认金黄色 (RGBA)

//=============================================================================
// 初始化配置
//=============================================================================
void ItemConfig::Initialize() {

    s_itemNames["Q201"] = u8"速度之靴";
    

    // 初始化过滤列表（在FILTERED模式下，只显示这些物品）
    // 默认包含所有有中文名称的物品
    for (const auto& pair : s_itemNames) {
        s_filterList.insert(pair.first);
    }

    // 也可以手动添加特定物品ID到过滤列表
    // s_filterList.insert("afac");
    // s_filterList.insert("Q201");
}

//=============================================================================
// 获取/设置调试模式
//=============================================================================
DebugMode ItemConfig::GetDebugMode() {
    return s_debugMode;
}

void ItemConfig::SetDebugMode(DebugMode mode) {
    s_debugMode = mode;
}

//=============================================================================
// 物品名称相关
//=============================================================================
std::string ItemConfig::GetItemDisplayName(const std::string& itemId) {
    auto it = s_itemNames.find(itemId);
    if (it != s_itemNames.end()) {
        return it->second;  // 返回中文名称
    }
    return itemId;  // 没有映射则返回原始ID
}

bool ItemConfig::IsItemInFilterList(const std::string& itemId) {
    return s_filterList.count(itemId) > 0;
}

//=============================================================================
// 显示控制
//=============================================================================
bool ItemConfig::ShouldShowDebugInfo() {
    // 只有 ALL 模式显示 debug 信息
    return s_debugMode == DebugMode::ALL;
}

bool ItemConfig::ShouldShowItemNotification(const std::string& itemId) {
    switch (s_debugMode) {
    case DebugMode::ALL:
        // 显示所有物品
        return true;

    case DebugMode::FILTERED:
        // 只显示在过滤列表中的物品
        return IsItemInFilterList(itemId);

    case DebugMode::NONE:
        // 不显示任何物品通知
        return false;

    default:
        return false;
    }
}

//=============================================================================
// TextTag 配置
//=============================================================================
float ItemConfig::GetTextTagSize() {
    return s_textTagSize;
}

void ItemConfig::GetTextTagColor(int& r, int& g, int& b, int& a) {
    r = s_textTagColor[0];
    g = s_textTagColor[1];
    b = s_textTagColor[2];
    a = s_textTagColor[3];
}
