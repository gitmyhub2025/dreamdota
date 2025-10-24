# ItemRuneDisplay 项目总结

## 📋 项目概述

本项目是基于 **DreamDota** 简化而来的 War3 物品和神符显示工具，专注于实时监控地图上的物品和神符生成。

**创建时间**: 2025-01-24
**基于项目**: DreamDota (dreamdota)
**目标版本**: Warcraft III 1.24e (6401)

## 📁 已创建的文件

### 核心实现文件

1. **ItemRuneDisplay_Full.cpp** (完整 C++ DLL 实现)
   - 大小: ~10KB
   - 功能: 完整的物品和神符监控 DLL
   - 特点:
     - 使用游戏的物品哈希表枚举所有物品
     - 定时器扫描机制（1秒间隔）
     - 游戏内文本显示
     - 物品缓存机制避免重复显示

2. **ItemRuneMonitor.j** (JASS 脚本实现)
   - 大小: ~6KB
   - 功能: 纯 JASS 脚本实现，无需 DLL 注入
   - 特点:
     - 使用 EnumItemsInRect 枚举物品
     - 定时器回调机制
     - 可直接集成到地图脚本

3. **ItemMonitor.cpp** (物品监控模块)
   - 大小: ~5KB
   - 功能: 物品监控和事件处理的独立模块
   - 特点:
     - 事件驱动架构
     - 可扩展的 Hook 接口

4. **main.cpp** (早期版本)
   - 大小: ~4KB
   - 功能: 早期简化实现
   - 用途: 参考和学习

### 配置和文档

5. **README.md** (项目说明)
   - 完整的项目介绍
   - 功能特性说明
   - 快速开始指南
   - 实现原理解析
   - 故障排查指南

6. **USAGE.md** (使用教程)
   - 详细的编译步骤
   - 两种使用方案（DLL / JASS）
   - 配置选项说明
   - 常见问题解答

7. **build.bat** (编译脚本)
   - Windows 批处理脚本
   - 自动化编译流程
   - 支持多个 Visual Studio 版本

8. **exports.def** (DLL 导出定义)
   - DLL 导出符号定义
   - 用于 Windows DLL 链接

9. **PROJECT_SUMMARY.md** (本文件)
   - 项目总结文档
   - 文件清单
   - 技术要点

## 🔑 核心技术要点

### 从 DreamDota 学习到的关键技术

#### 1. JASS Native 函数调用
```cpp
// 获取函数指针 (1.24e 偏移量)
g_GetItemTypeId = (GetItemTypeId_t)(GameBase + 0x003C4C60);
g_IsItemPowerup = (IsItemPowerup_t)(GameBase + 0x003C4FD0);

// 调用
uint32_t typeId = g_GetItemTypeId(itemHandle);
bool isPowerup = g_IsItemPowerup(itemHandle);
```

**关键 JASS 函数**:
- `GetItemTypeId` - 获取物品类型ID
- `GetItemX` / `GetItemY` - 获取物品坐标
- `IsItemPowerup` - 判断是否为神符
- `IsItemOwned` - 判断是否被拾取
- `GetWidgetLife` - 获取物品生命值

#### 2. 物品枚举方法

**方法 A: 遍历物品哈希表 (C++)**
```cpp
// 游戏内部维护了一个物品哈希表
ItemHashTable* table = *(ItemHashTable**)(GameBase + 0x00AB4F84);

// 遍历所有桶
for (uint32_t i = 0; i < table->capacity; i++) {
    ItemHashNode* node = table->buckets[i];

    // 遍历链表
    while (node) {
        uint32_t itemHandle = node->key;
        // 处理物品
        node = node->next;
    }
}
```

**方法 B: 使用 JASS 枚举 (JASS)**
```jass
// 创建地图边界矩形
set g_MapBounds = GetWorldBounds()

// 枚举矩形内所有物品
call EnumItemsInRect(g_MapBounds, null, function EnumItemCallback)
```

#### 3. 游戏内文本显示

```cpp
// UI 显示函数指针
GameUI_DisplayText_t DisplayText = (GameUI_DisplayText_t)(GameBase + 0x0055D7C0);
void** GameGlobalUI = (void**)(GameBase + 0x00AB65F0);

// 显示文本
DisplayText(*GameGlobalUI, 0, 0.0f, 0.0f,
    "|cffffcc00黄色文本|r", 10.0f, -1);
```

**颜色代码**:
- `|cffffcc00` - 黄色（神符）
- `|cff00ff00` - 绿色（普通物品）
- `|cffff0000` - 红色（错误）
- `|r` - 重置颜色

#### 4. 神符识别逻辑

```cpp
// 检查是否为神符
if (IsItemPowerup(item)) {
    uint32_t itemType = GetItemType(item);
    // ITEM_TYPE_POWERUP = 2

    // 过滤无用道具
    if (typeId == 'I0KK' || typeId == 'I0HM') {
        return;  // 忽略
    }

    // 显示神符信息
    DisplayPowerupInfo(item);
}
```

#### 5. ID 转换

```cpp
// 整数 ID -> 4字符字符串
const char* IntegerIdToChar(uint32_t id) {
    static char buffer[5];
    buffer[0] = (id >> 0) & 0xFF;
    buffer[1] = (id >> 8) & 0xFF;
    buffer[2] = (id >> 16) & 0xFF;
    buffer[3] = (id >> 24) & 0xFF;
    buffer[4] = '\0';
    return buffer;
}

// 示例: 'afac' = 0x63616661
// buffer = ['a', 'f', 'a', 'c', '\0']
```

## 📊 关键偏移量表 (War3 1.24e / 6401)

| 功能 | 偏移量 | 说明 |
|------|--------|------|
| GetItemTypeId | 0x003C4C60 | 获取物品类型ID |
| GetItemX | 0x003C4D90 | 获取物品X坐标 |
| GetItemY | 0x003C4DD0 | 获取物品Y坐标 |
| GetItemType | 0x003C4E70 | 获取物品类型 |
| IsItemPowerup | 0x003C4FD0 | 判断是否为神符 |
| IsItemOwned | 0x003C4F90 | 判断是否被拥有 |
| GetWidgetLife | 0x003C30A0 | 获取生命值 |
| GameGlobalUI | 0x00AB65F0 | 游戏UI指针 |
| GameUI_DisplayText | 0x0055D7C0 | 显示文本函数 |
| ItemHashTable | 0x00AB4F84 | 物品哈希表 |

**其他版本偏移量**:
- 1.24b (6374): 见 `native_offsets_6374.inc`
- 1.24c (6384): 见 `native_offsets_6384.inc`
- 1.24d (6397): 见 `native_offsets_6397.inc`

## 🎯 实现的功能

### ✅ 已实现

1. **物品实时监控**
   - 定时扫描地图上的所有物品
   - 识别神符（Powerup）类型
   - 过滤已拾取和无用道具

2. **游戏内显示**
   - 彩色文本提示
   - 显示物品ID和坐标
   - 可配置显示时长

3. **物品缓存**
   - 避免重复显示同一物品
   - 使用 `std::set` 记录已显示物品

4. **定时器系统**
   - 使用 Windows Timer Queue
   - 可配置扫描间隔

5. **JASS 脚本方案**
   - 纯脚本实现
   - 无需 DLL 注入
   - 可直接集成到地图

### ❌ 未实现 (可扩展)

1. **物品名称显示**
   - 当前显示 4字符 ID
   - 需要从游戏数据库读取名称

2. **小地图标记**
   - 需要调用 `PingMinimap` 函数
   - 偏移量: 待确定

3. **事件驱动监控**
   - Hook 物品创建事件
   - 需要找到准确的事件分发函数地址

4. **物品图标显示**
   - 从 Profile 读取图标路径
   - 需要 UI 框架支持

5. **配置界面**
   - 游戏内设置菜单
   - 开关监控功能

## 🔨 编译和使用

### 编译要求

- **编译器**: Visual Studio 2010 或更高版本
- **平台**: x86 (32位)
- **SDK**: Windows SDK 7.0+
- **C++ 标准**: C++03 或更高

### 编译命令

```batch
cl.exe /LD /MT /O2 /EHsc /Fe:ItemRuneDisplay.dll ItemRuneDisplay_Full.cpp /link kernel32.lib user32.lib
```

### 使用方法

**方案 1: DLL 注入**
1. 编译生成 `ItemRuneDisplay.dll`
2. 启动 War3 1.24e，进入游戏
3. 使用注入工具注入 DLL
4. 游戏中会显示启动提示

**方案 2: JASS 脚本**
1. 将 `ItemRuneMonitor.j` 复制到地图脚本
2. 在初始化函数中调用 `InitItemRuneMonitor()`
3. 保存地图并测试

## 📖 学习要点

### DreamDota 项目结构

```
DreamWarcraft/
├── Item.h/cpp              # 物品类封装
├── RuneNotify.h/cpp        # 神符通知（本项目参考）
├── Tools.h/cpp             # 工具函数
├── Jass.h/cpp              # JASS API 初始化
├── native_offsets_*.inc    # 版本偏移量定义
├── GameStructs.h           # 游戏内部结构体
└── ...
```

### 关键代码片段参考

1. **OutputScreen 实现** (Tools.cpp:73-88)
```cpp
int OutputScreen(float duration, const char *format, ...) {
    char buffer[1024];
    vsprintf_s(buffer, 1024, format, args);
    GameUITextDisplay(*GameGlobalUI, 0, 0.0, 0.0, buffer, duration, -1);
    return rv;
}
```

2. **神符检测** (RuneNotify.cpp:7-26)
```cpp
void onItemCreated(const Event *evt) {
    Item *createdItem = GetItem(data->createdItem);
    if (createdItem->itemType() == Jass::ITEM_TYPE_POWERUP) {
        // 过滤无用道具
        if (createdItem->typeId() == 'I0KK') return;

        // 显示信息
        OutputScreen(10, "神符生成: %s @ %s",
            createdItem->name(),
            pos.toString().c_str());
    }
}
```

3. **物品信息获取** (Item.cpp:13-26)
```cpp
float Item::x() const { return Jass::GetItemX(handleId_); }
uint32_t Item::typeId() const { return Jass::GetItemTypeId(handleId_); }
bool Item::isPowerup() const { return Jass::IsItemPowerup(handleId_); }
```

## 🎓 适用场景

1. **学习逆向工程**
   - 理解游戏内存结构
   - 学习函数 Hook 技术
   - 研究游戏引擎

2. **地图制作辅助**
   - 测试物品生成
   - 调试神符机制
   - 验证游戏逻辑

3. **自定义游戏体验**
   - 增强游戏可视化
   - 提供额外信息
   - 改善用户体验

## ⚠️ 免责声明

- 本项目仅用于**学习和研究**目的
- 不得用于破坏游戏平衡或在线作弊
- 使用 DLL 注入有风险，请确保了解相关法律法规
- 请遵守游戏服务条款

## 🙏 致谢

- **DreamDota 项目** - 提供了完整的参考实现
- **Warcraft III 社区** - 丰富的逆向工程资源
- **所有贡献者** - 感谢开源精神

## 📈 未来改进方向

1. **支持更多版本** - 1.26, 1.27, 1.28+
2. **物品名称本地化** - 支持中文显示
3. **小地图标记** - 实现 Ping 效果
4. **事件驱动** - 更高效的监控机制
5. **配置文件** - INI/JSON 配置支持
6. **UI 界面** - 游戏内设置菜单

## 📞 联系方式

如有问题或建议，欢迎通过以下方式联系：
- 提交 GitHub Issue
- 发起 Pull Request
- 参与社区讨论

---

**项目状态**: ✅ 完成
**版本**: 1.0
**日期**: 2025-01-24
