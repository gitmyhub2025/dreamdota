/*
 * ItemMonitor.h - 物品监控系统头文件
 */

#ifndef ITEM_MONITOR_H
#define ITEM_MONITOR_H

#include <windows.h>

// 初始化物品监控系统
bool ItemMonitor_Initialize();

// 开始监控
void ItemMonitor_Start();

// 停止监控
void ItemMonitor_Stop();

// 清理资源
void ItemMonitor_Cleanup();

// 检查是否在游戏中
bool ItemMonitor_IsInGame();

#endif // ITEM_MONITOR_H
