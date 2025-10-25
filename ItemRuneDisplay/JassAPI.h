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

// TextTag functions for displaying text at world positions
uint32_t Jass_CreateTextTag();
void Jass_SetTextTagText(uint32_t textTag, const char* text, float height);
void Jass_SetTextTagPos(uint32_t textTag, float x, float y, float zOffset);
void Jass_SetTextTagColor(uint32_t textTag, int r, int g, int b, int a);
void Jass_SetTextTagVisibility(uint32_t textTag, bool visible);
void Jass_SetTextTagPermanent(uint32_t textTag, bool permanent);
void Jass_SetTextTagLifespan(uint32_t textTag, float lifespan);
void Jass_SetTextTagFadepoint(uint32_t textTag, float fadepoint);

#endif // JASS_API_H
