# ItemRuneDisplay - 魔兽争霸III 物品和神符显示工具

基于 **DreamDota** 项目简化而来，专注于地图上物品和神符的实时显示。

## ✨ 功能特性

- 🎯 **实时监控** - 自动检测地图上新生成的物品和神符
- 📍 **位置显示** - 在游戏内显示物品的精确坐标
- 🎨 **彩色提示** - 使用颜色区分神符和普通物品
- 🗺️ **小地图标记** - 神符生成时在小地图上标记位置
- ⚡ **轻量高效** - 极小的性能占用，不影响游戏体验
- 🔧 **易于定制** - 简洁的代码结构，方便修改和扩展

## 🎮 支持版本

- **Warcraft III 1.24e** (版本号 6387 = 1.24.4.6387)
- 理论上支持其他 1.24 系列版本（需修改偏移量）

## 📦 项目文件

```
ItemRuneDisplay/
├── README.md                      # 项目说明
├── USAGE.md                       # 详细使用教程
├── build.bat                      # Windows 编译脚本
├── exports.def                    # DLL 导出定义
├── ItemRuneDisplay_6387.cpp       # War3 1.24e (6387) DLL 实现 ⭐推荐
├── ItemRuneDisplay_Full.cpp       # 早期版本（6401）
├── ItemRuneMonitor.j              # JASS 脚本实现（无需 DLL）
├── ItemMonitor.cpp                # 物品监控模块
└── main.cpp                       # 早期版本（参考）
```

## 🚀 快速开始

### 方案 A: 使用 DLL（推荐）

**1. 编译 DLL**
```batch
# Windows 下双击运行
build.bat
```

**2. 注入游戏**
- 启动 Warcraft III 1.24e
- 进入游戏地图
- 使用 DLL 注入工具（如 Extreme Injector）注入 `ItemRuneDisplay.dll`

**3. 开始使用**
- 成功注入后游戏中会显示绿色提示
- 地图上生成神符时会自动显示黄色提示

### 方案 B: 使用 JASS 脚本（无需注入）

**1. 编辑地图**
- 在地图编辑器中打开你的地图
- 打开触发编辑器 -> 转换为自定义文本

**2. 添加脚本**
- 将 `ItemRuneMonitor.j` 的内容复制到地图脚本中
- 在地图初始化函数 `main()` 或 `config()` 中添加：
```jass
call InitItemRuneMonitor()
```

**3. 测试地图**
- 保存地图并测试

## 📖 核心实现原理

### 从 DreamDota 学到的关键技术

1. **JASS 函数调用**
```cpp
// 使用游戏内置的 JASS Native 函数
g_GetItemTypeId = (GetItemTypeId_t)(GameBase + 0x003C4C60);
g_IsItemPowerup = (IsItemPowerup_t)(GameBase + 0x003C4FD0);

// 调用示例
uint32_t typeId = g_GetItemTypeId(itemHandle);
bool isPowerup = g_IsItemPowerup(itemHandle);
```

2. **物品枚举**
```cpp
// 通过游戏的物品哈希表遍历所有物品
ItemHashTable* table = *(ItemHashTable**)(GameBase + ItemHashTableOffset);
for (uint32_t i = 0; i < table->capacity; i++) {
    ItemHashNode* node = table->buckets[i];
    while (node) {
        ProcessItem(node->key);  // key 是物品句柄
        node = node->next;
    }
}
```

3. **游戏内文本显示**
```cpp
// 使用游戏的 UI 系统显示文本
GameUI_DisplayText(*GameGlobalUI, 0, 0.0f, 0.0f,
    "|cffffcc00提示文本|r", duration, -1);
```

4. **神符判断**
```cpp
// 判断物品是否为神符/Powerup
if (IsItemPowerup(item)) {
    uint32_t itemType = GetItemType(item);
    // ITEM_TYPE_POWERUP = 2
    if (itemType == 2) {
        // 这是神符
    }
}
```

### 显示效果示例

```
游戏中显示：
[神符] afac @ (1234, 5678)  <-- 黄色文本，持续 10 秒
[物品] ward @ (2345, 6789)  <-- 绿色文本，持续 5 秒
```

## 🔧 自定义配置

### 修改扫描间隔
```cpp
// ItemRuneDisplay_Full.cpp 中
CreateTimerQueueTimer(&g_hTimer, g_hTimerQueue, TimerCallback,
    nullptr, 1000, 1000, WT_EXECUTEDEFAULT);
    //        ^初始  ^间隔（毫秒）
```

### 修改显示时长
```cpp
OutputToScreen(message, 10.0f);  // 持续 10 秒
```

### 添加物品过滤
```cpp
// 过滤特定物品
if (typeId == 'I0KK' || typeId == 'I0HM') {
    return;  // 不显示这些物品
}
```

## 📚 相关资源

### DreamDota 项目关键文件
- `Item.h/cpp` - 物品类封装
- `RuneNotify.h/cpp` - 神符通知实现
- `Tools.h/cpp` - 辅助函数（OutputScreen、IntegerIdToChar）
- `native_offsets_6401.inc` - War3 1.24e 函数偏移量

### 使用的 JASS 函数
```jass
native GetItemTypeId   takes item i returns integer
native GetItemX        takes item i returns real
native GetItemY        takes item i returns real
native GetItemType     takes item whichItem returns itemtype
native IsItemPowerup   takes item whichItem returns boolean
native IsItemOwned     takes item whichItem returns boolean
native GetWidgetLife   takes widget whichWidget returns real
native EnumItemsInRect takes rect r, boolexpr filter, code actionFunc returns nothing
```

## ⚠️ 注意事项

1. **版本兼容性** - 当前仅支持 1.24e，其他版本需修改偏移量
2. **使用风险** - 在线游戏可能被检测为作弊，建议仅用于学习
3. **性能影响** - 极小，但在物品超多的地图可能需优化扫描频率
4. **安全性** - DLL 注入需要管理员权限，请确保来源可信

## 🛠️ 故障排查

### 问题：注入后没反应
- 检查游戏版本是否为 1.24e
- 确认已进入游戏（不是主菜单）
- 等待 5 秒初始化时间
- 查看防火墙/杀毒软件是否拦截

### 问题：显示的是 ID 而不是名称
- 当前简化版本直接显示物品 ID（4字符代码）
- 完整实现需要从游戏数据库读取名称字符串

### 问题：某些神符不显示
- 检查是否被过滤（'I0KK', 'I0HM' 等）
- 确认物品未被英雄拾取
- 查看物品是否存活（life > 0）

## 🤝 贡献

基于 **DreamDota** 项目，感谢原作者的开源贡献！

如果你有改进建议或发现 Bug，欢迎：
- 提交 Issue
- 发起 Pull Request
- 分享你的修改版本

## 📄 许可证

本项目仅用于学习和研究目的。请遵守游戏服务条款，不要用于破坏游戏平衡。

## 🔗 相关链接

- [DreamDota 原项目](https://github.com/actboy168/dreamdota)
- [Warcraft III 内存结构文档](https://www.hiveworkshop.com/)
- [JASS 编程指南](http://jass.sourceforge.net/)

---

**编译环境**: Visual Studio 2010+ (32位)
**测试环境**: Windows 10 x64 + Warcraft III 1.24e
**最后更新**: 2025-01-24
