/*
 * Utils.h - 辅助函数
 */

#ifndef UTILS_H
#define UTILS_H

#include <windows.h>
#include <string>

// 将整数 ID 转换为字符串
std::string Utils_IntegerIdToString(uint32_t id);

// 显示文本到游戏屏幕
void Utils_OutputToScreen(const char* text, float duration = 5.0f);

// 初始化 UI 系统
bool Utils_InitializeUI();

#endif // UTILS_H
