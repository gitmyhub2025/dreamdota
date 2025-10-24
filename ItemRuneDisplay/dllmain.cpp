/*
 * dllmain.cpp - DLL 入口点
 * ItemRuneDisplay for Warcraft III 1.24e (6387)
 */

#include <windows.h>
#include "ItemMonitor.h"

// 主线程
DWORD WINAPI MainThread(LPVOID lpParam) {
    // 等待游戏初始化完成
    Sleep(5000);

    // 初始化物品监控系统
    if (!ItemMonitor_Initialize()) {
        MessageBoxA(nullptr, "ItemRuneDisplay 初始化失败！", "错误", MB_OK | MB_ICONERROR);
        return 1;
    }

    // 开始监控
    ItemMonitor_Start();

    return 0;
}

// DLL 入口点
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        break;

    case DLL_PROCESS_DETACH:
        ItemMonitor_Stop();
        ItemMonitor_Cleanup();
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
