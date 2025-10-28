/*
 * ItemConfig.h - 物品显示配置和中文名称映射
 */

#ifndef ITEM_CONFIG_H
#define ITEM_CONFIG_H

#include <windows.h>
#include <string>
#include <map>
#include <set>

//=============================================================================
// 调试显示模式
//=============================================================================
enum class DebugMode {
    ALL,            // 显示所有debug信息和所有物品
    FILTERED,       // 只显示指定物品列表的信息，支持中文名称
    NONE            // 禁止显示所有debug和物品通知
};

//=============================================================================
// 物品配置类
//=============================================================================
class ItemConfig {
public:
    // 初始化配置（从文件读取或使用默认配置）
    static void Initialize();

    // 获取调试模式
    static DebugMode GetDebugMode();

    // 设置调试模式
    static void SetDebugMode(DebugMode mode);

    // 获取物品的中文名称（如果没有映射则返回英文ID）
    static std::string GetItemDisplayName(const std::string& itemId);

    // 检查物品是否在显示列表中（仅在FILTERED模式下有意义）
    static bool IsItemInFilterList(const std::string& itemId);

    // 是否显示Debug信息
    static bool ShouldShowDebugInfo();

    // 是否显示物品通知（屏幕文本）
    static bool ShouldShowItemNotification(const std::string& itemId);

    // TextTag 配置
    static float GetTextTagSize();
    static void GetTextTagColor(int& r, int& g, int& b, int& a);

private:
    // UTF-8 到 GBK 编码转换（War3 需要 GBK 编码）
    static std::string UTF8ToGBK(const char* utf8Str);

    static DebugMode s_debugMode;
    static std::map<std::string, std::string> s_itemNames;
    static std::set<std::string> s_filterList;
    static float s_textTagSize;
    static int s_textTagColor[4];  // RGBA
};

#endif // ITEM_CONFIG_H
