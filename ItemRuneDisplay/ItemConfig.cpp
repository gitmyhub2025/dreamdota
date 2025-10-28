/*
 * ItemConfig.cpp - 物品配置实现
 */

#include "ItemConfig.h"

//=============================================================================
// 静态成员初始化
//=============================================================================
DebugMode ItemConfig::s_debugMode = DebugMode::FILTERED;  // ALL 默认显示所有// 或 FILTERED, NONE
std::map<std::string, std::string> ItemConfig::s_itemNames;
std::set<std::string> ItemConfig::s_filterList;
float ItemConfig::s_textTagSize = 0.024f;  // 调小字体大小
int ItemConfig::s_textTagColor[4] = {255, 204, 0, 255};  // 默认金黄色 (RGBA)

//=============================================================================
// UTF-8 到 GBK 编码转换（War3 1.24e 使用 GBK 编码）
//=============================================================================
std::string ItemConfig::UTF8ToGBK(const char* utf8Str) {
    if (!utf8Str || utf8Str[0] == '\0') {
        return std::string();
    }

    // UTF-8 -> Unicode (Wide Char)
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, nullptr, 0);
    if (wideLen <= 0) {
        return std::string(utf8Str);  // 转换失败，返回原字符串
    }

    wchar_t* wideBuf = new wchar_t[wideLen];
    MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, wideBuf, wideLen);

    // Unicode (Wide Char) -> GBK
    int gbkLen = WideCharToMultiByte(CP_ACP, 0, wideBuf, -1, nullptr, 0, nullptr, nullptr);
    if (gbkLen <= 0) {
        delete[] wideBuf;
        return std::string(utf8Str);  // 转换失败，返回原字符串
    }

    char* gbkBuf = new char[gbkLen];
    WideCharToMultiByte(CP_ACP, 0, wideBuf, -1, gbkBuf, gbkLen, nullptr, nullptr);

    std::string result(gbkBuf);
    delete[] wideBuf;
    delete[] gbkBuf;

    return result;
}

//=============================================================================
// 初始化配置
//=============================================================================
void ItemConfig::Initialize() {
    // 注意：u8"中文" 是 UTF-8 编码，需要转换为 GBK 才能在 War3 中正确显示
    // UTF8ToGBK 函数会自动处理转换

    s_itemNames["Q20I"] = UTF8ToGBK(u8"速度之靴");
    s_itemNames["V30I"] = UTF8ToGBK(u8"敏捷便携");
    s_itemNames["B30I"] = UTF8ToGBK(u8"力量手套");

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
