# 快速开始指南

## ✅ 已推送到 GitHub

**仓库**: `gitmyhub2025/dreamdota`
**分支**: `claude/analyze-item-symbol-logic-011CUQ21HjyXwDUTf1Ff5xA9`
**提交**: 9eaae83

## 🚀 拉取到本地

```bash
# 克隆仓库（如果还没有）
git clone https://github.com/gitmyhub2025/dreamdota.git
cd dreamdota

# 切换到分支
git checkout claude/analyze-item-symbol-logic-011CUQ21HjyXwDUTf1Ff5xA9

# 进入项目目录
cd ItemRuneDisplay
```

## 🔨 编译 DLL

### ⭐ 方法 1: 使用 Visual Studio（最简单）
```
1. 双击打开 ItemRuneDisplay.sln
2. 选择 Release | Win32
3. 按 F7 或 菜单 → 生成 → 生成解决方案
4. 输出: bin\Release\ItemRuneDisplay.dll
```

### 方法 2: 使用批处理（Windows）
```batch
build.bat
```

### 方法 3: 使用 MSBuild 命令行
```batch
# 打开 Developer Command Prompt for VS
msbuild ItemRuneDisplay.sln /p:Configuration=Release /p:Platform=Win32
```

### 方法 4: 手动编译
```batch
# 设置 Visual Studio 环境
"C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86

# 编译
cl.exe /LD /MT /O2 /EHsc /Fe:ItemRuneDisplay.dll ItemRuneDisplay_6387.cpp /link kernel32.lib user32.lib
```

**详细编译指南**: 参考 `BUILD.md`

输出文件：`ItemRuneDisplay.dll` (约 50KB)

## 🎮 使用方法

### 选项 A: DLL 注入（推荐）

1. **启动游戏**
   - 运行 Warcraft III 1.24e (1.24.4.6387)
   - 进入游戏地图

2. **注入 DLL**
   - 使用 DLL 注入工具（如 [Extreme Injector](https://github.com/master131/ExtremeInjector)）
   - 选择 `war3.exe` 进程
   - 注入 `ItemRuneDisplay.dll`

3. **验证**
   - 游戏中应显示：`|cff00ff00ItemRuneDisplay 已启动！War3 1.24e (6387)|r`
   - 地图上生成神符时会显示黄色提示

### 选项 B: JASS 脚本（无需 DLL）

1. **打开地图编辑器**
   - World Editor → 触发编辑器

2. **添加脚本**
   - 转换为自定义文本
   - 将 `ItemRuneMonitor.j` 的内容复制进去

3. **初始化**
   - 在地图初始化函数中添加：
     ```jass
     call InitItemRuneMonitor()
     ```

4. **测试地图**

## 📊 显示效果

```
游戏中会显示：
|cffffcc00[神符生成]|r afac @ (1234, 5678)  ← 黄色文本，持续 10 秒
|cff00ff00[物品]|r ward @ (2345, 6789)    ← 绿色文本，持续 5 秒
```

## ⚙️ 配置

### 修改扫描间隔
编辑 `ItemRuneDisplay_6387.cpp` 第 239 行：
```cpp
CreateTimerQueueTimer(&g_hTimer, g_hTimerQueue, TimerCallback,
    nullptr, 1000, 1000, WT_EXECUTEDEFAULT);
    //        ^初始  ^间隔（毫秒）
```

### 修改显示时长
编辑第 113 行：
```cpp
OutputToScreen(message, 10.0f);  // 持续秒数
```

## ⚠️ 重要说明

1. **版本确认**
   - 必须是 War3 1.24e (1.24.4.6387)
   - 检查方法：游戏主菜单右下角显示版本号

2. **安全性**
   - 仅用于学习和研究
   - 在线游戏可能被检测为作弊
   - 建议仅在单机地图使用

3. **故障排查**
   - 注入后没反应？等待 5 秒初始化
   - 显示 ID 而非名称？这是正常的（简化版本）
   - 某些物品不显示？可能被过滤或已被拾取

## 📁 项目文件说明

| 文件 | 用途 |
|------|------|
| `ItemRuneDisplay.sln` | **VS 解决方案** - 双击即可打开 |
| `ItemRuneDisplay.vcxproj` | **VS 项目文件** - 项目配置 |
| `ItemRuneDisplay_6387.cpp` | **主要源码** - War3 1.24e DLL |
| `ItemRuneMonitor.j` | JASS 脚本版本（备选方案） |
| `BUILD.md` | 详细编译指南 ⭐ |
| `README.md` | 完整项目文档 |
| `USAGE.md` | 详细使用教程 |
| `PROJECT_SUMMARY.md` | 技术细节和学习要点 |
| `build.bat` | Windows 批处理编译脚本 |
| `exports.def` | DLL 导出定义 |

## 🔗 核心技术

从 **DreamDota** 学习的关键实现：

```cpp
// War3 1.24e (6387) 偏移量
GetItemTypeId:  0x003C57A0
GetItemX:       0x003C58D0
GetItemY:       0x003C5910
IsItemPowerup:  0x003C5B10
IsItemOwned:    0x003C5AD0
GameUI_Display: 0x002F9980
```

## 💡 提示

- 第一次使用建议先看 `README.md` 了解原理
- 遇到问题查看 `USAGE.md` 的故障排查
- 想学习技术细节参考 `PROJECT_SUMMARY.md`

## 📞 反馈

如有问题或建议：
- 提交 GitHub Issue
- 查看 DreamDota 原项目文档
- 参考 War3 社区资源

---

**编译环境**: Visual Studio 2010+ (x86)
**测试环境**: Windows 10 x64 + War3 1.24e
**创建日期**: 2025-01-24
