# ItemRuneDisplay 使用说明

## 编译方法

### 方法1: 使用批处理脚本（推荐）
1. 安装 Visual Studio 2010 或更高版本
2. 编辑 `build.bat`，设置正确的 Visual Studio 路径
3. 双击运行 `build.bat`
4. 成功后会生成 `ItemRuneDisplay.dll`

### 方法2: 使用命令行
```batch
# 设置编译环境
"C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86

# 编译
cl.exe /LD /MT /O2 /EHsc /Fe:ItemRuneDisplay.dll ItemRuneDisplay_Full.cpp /link kernel32.lib user32.lib
```

## 使用方法

### 方案A: 使用 DLL 注入器

1. **准备工具**
   - 下载 DLL 注入工具（如 Extreme Injector, Process Hacker 等）
   - 准备好编译的 `ItemRuneDisplay.dll`

2. **注入步骤**
   - 启动 Warcraft III 1.24e
   - 进入游戏地图
   - 使用注入工具将 `ItemRuneDisplay.dll` 注入到 `war3.exe` 进程
   - 注入成功后会在游戏中显示提示信息

3. **验证**
   - 游戏中会显示 "|cff00ff00物品和神符监控已启动！|r"
   - 地图上生成神符时会自动显示提示

### 方案B: 使用 JASS 脚本（无需 DLL）

如果你不想使用 DLL 注入，可以直接使用 JASS 脚本：

1. 打开地图编辑器
2. 打开 `ItemRuneMonitor.j` 文件
3. 将内容复制到地图的脚本中
4. 在地图初始化函数中添加：
   ```jass
   call InitItemRuneMonitor()
   ```
5. 保存并测试地图

## 功能说明

### 显示内容
- **神符（Powerup）**: 黄色文本显示，持续 10 秒
  - 格式: `[神符] <物品名称> @ (X, Y)`
  - 同时在小地图标记位置

- **普通物品**: 绿色文本显示，持续 5 秒（可选）
  - 格式: `[物品] <物品名称> @ (X, Y)`

### 配置选项

编辑 `ItemRuneDisplay_Full.cpp` 中的代码来自定义：

```cpp
// 扫描间隔（毫秒）
CreateTimerQueueTimer(&g_hTimer, g_hTimerQueue, TimerCallback,
    nullptr, 1000, 1000, WT_EXECUTEDEFAULT);
    //        ^初始延迟  ^间隔时间

// 显示持续时间
OutputToScreen(message, 10.0f);  // 10 秒
```

## 兼容性

- **支持版本**: Warcraft III 1.24e (版本号 6401)
- **操作系统**: Windows XP/Vista/7/8/10 (32位模式)
- **编译器**: Visual Studio 2010 或更高版本

### 其他版本支持

如果需要支持其他版本（如 1.24b, 1.24d 等），需要修改偏移量：

```cpp
namespace War3Offsets {
    // 将这些偏移量改为对应版本的值
    const DWORD GetItemTypeId  = 0x003C4C60;  // 修改这里
    const DWORD GetItemX       = 0x003C4D90;  // 修改这里
    // ...
}
```

偏移量可以在 dreamdota 的 `native_offsets_xxxx.inc` 文件中找到。

## 常见问题

### Q: 注入后没有任何提示
A:
1. 确认游戏版本是 1.24e
2. 确认已进入游戏（不是主菜单）
3. 等待 5 秒钟（DLL 有初始化延迟）
4. 检查是否有防病毒软件阻止

### Q: 显示的是物品ID而不是名称
A:
- 当前简化版本直接显示物品 ID（4字符代码）
- 要显示中文名称，需要从游戏数据库读取，这需要更复杂的实现

### Q: 某些物品不显示
A:
- 已被英雄拾取的物品不会显示
- 某些特殊物品（如 'I0KK', 'I0HM'）被过滤
- 可以修改代码中的过滤列表

### Q: 如何停止监控
A:
- 退出游戏会自动停止
- 或使用 Process Hacker 卸载 DLL

## 安全提醒

- **仅用于学习和研究目的**
- 在线游戏使用可能被检测为作弊
- 建议仅在本地单机游戏中使用
- 使用 DLL 注入有一定风险，请谨慎操作

## 技术支持

如有问题，请参考：
1. DreamDota 项目文档
2. Warcraft III 内存结构文档
3. JASS 脚本编程指南

## 更新日志

### v1.0 (2025-01-24)
- 初始版本
- 支持 War3 1.24e
- 基本的物品和神符显示功能
