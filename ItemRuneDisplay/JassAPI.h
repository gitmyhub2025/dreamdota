/*
 * JassAPI.h - JASS Native 函数接口
 * War3 1.24e (6387) 偏移量
 */

#ifndef JASS_API_H
#define JASS_API_H

#include <windows.h>
#include <cstdint>

// 初始化 JASS API
bool Jass_Initialize();

// 获取 Game.dll 基址
DWORD Jass_GetGameBase();

// JASS Native 函数
uint32_t Jass_GetItemTypeId(uint32_t item);
float Jass_GetItemX(uint32_t item);
float Jass_GetItemY(uint32_t item);
uint32_t Jass_GetItemType(uint32_t item);
bool Jass_IsItemPowerup(uint32_t item);
bool Jass_IsItemOwned(uint32_t item);
float Jass_GetWidgetLife(uint32_t widget);

#endif // JASS_API_H
