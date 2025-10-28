/*
 * ItemConfig.cpp - 物品配置实现
 */

#include "ItemConfig.h"

//=============================================================================
// 静态成员初始化
//=============================================================================
DebugMode ItemConfig::s_debugMode = DebugMode::ALL;  // 默认显示所有
std::map<std::string, std::string> ItemConfig::s_itemNames;
std::set<std::string> ItemConfig::s_filterList;
float ItemConfig::s_textTagSize = 0.024f;  // 调小字体大小
int ItemConfig::s_textTagColor[4] = {255, 204, 0, 255};  // 默认金黄色 (RGBA)

//=============================================================================
// 初始化配置
//=============================================================================
void ItemConfig::Initialize() {
    // 初始化常见物品的中文名称映射
    // 格式：{"英文ID", u8"中文名称"}
    // 注意：中文字符串前必须加 u8 前缀以确保 UTF-8 编码

    // 示例：常见神符/物品
    s_itemNames["I0HM"] = u8"虚无宝石";
    s_itemNames["I0KK"] = u8"空白物品";
    s_itemNames["afac"] = u8"治疗药膏";
    s_itemNames["desc"] = u8"卷轴";
    s_itemNames["mcou"] = u8"勇气徽章";
    s_itemNames["ratf"] = u8"力量手套";
    s_itemNames["rag1"] = u8"敏捷便鞋";
    s_itemNames["rhe1"] = u8"智力斗篷";
    s_itemNames["rst1"] = u8"铁木枝干";
    s_itemNames["rej1"] = u8"活力球";
    s_itemNames["rej2"] = u8"虚空宝石";
    s_itemNames["rej3"] = u8"魔法恢复";
    s_itemNames["rej4"] = u8"生命恢复";
    s_itemNames["rej5"] = u8"双倍伤害";
    s_itemNames["rej6"] = u8"幻象";

    // 添加更多物品映射...
    // 用户可以根据需要添加自己的物品ID映射
    s_itemNames["Q201"] = u8"速度之靴";
    s_itemNames["rst2"] = u8"铁意志之盔";
    s_itemNames["rhe2"] = u8"贤者面罩";
    s_itemNames["ratc"] = u8"秘银锤";

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
