# ItemRuneDisplay 配置说明

## 功能概述

ItemRuneDisplay 现在支持：
1. ✅ **TextTag 显示物品名称**（浮动文字，始终显示）
2. ✅ **屏幕通知显示**（可配置3种模式）
3. ✅ **中文名称映射**（支持自定义物品ID→中文名称）
4. ✅ **自动清理**（物品消失后 TextTag 自动隐藏）
5. ✅ **可配置的 TextTag 大小和颜色**

---

## 调试模式说明

在 `ItemConfig.cpp` 中可以修改调试模式：

```cpp
DebugMode ItemConfig::s_debugMode = DebugMode::ALL;  // 默认模式
```

### 模式 1: ALL（显示所有）
```cpp
s_debugMode = DebugMode::ALL;
```
- ✅ 显示所有 DEBUG 信息
- ✅ 显示所有物品的屏幕通知
- ✅ TextTag 显示所有物品（带中文名称）

**适用场景**：开发调试、测试新物品

---

### 模式 2: FILTERED（过滤显示）
```cpp
s_debugMode = DebugMode::FILTERED;
```
- ❌ 不显示 DEBUG 信息
- ✅ 只显示过滤列表中的物品屏幕通知
- ✅ TextTag 显示所有物品（带中文名称）
- ✅ 屏幕通知使用中文名称

**适用场景**：只关心重要物品（神符、高级装备等）

**如何添加物品到过滤列表：**
在 `ItemConfig::Initialize()` 中：
```cpp
// 方法1：自动包含所有有中文名称的物品
for (const auto& pair : s_itemNames) {
    s_filterList.insert(pair.first);
}

// 方法2：手动添加特定物品
s_filterList.insert("afac");  // 治疗药膏
s_filterList.insert("Q201");  // 速度之靴
```

---

### 模式 3: NONE（禁止显示）
```cpp
s_debugMode = DebugMode::NONE;
```
- ❌ 不显示 DEBUG 信息
- ❌ 不显示物品屏幕通知
- ✅ TextTag 仍然显示所有物品（带中文名称）

**适用场景**：只用 TextTag，不想要屏幕通知干扰

---

## 添加中文物品名称

在 `ItemConfig::Initialize()` 中添加映射：

```cpp
void ItemConfig::Initialize() {
    // 添加你的物品映射
    // 注意：中文字符串前必须加 u8 前缀，并使用 UTF8ToGBK() 转换！
    // War3 1.24e 使用 GBK 编码，必须转换才能正确显示

    s_itemNames["Q201"] = UTF8ToGBK(u8"速度之靴");
    s_itemNames["afac"] = UTF8ToGBK(u8"治疗药膏");
    s_itemNames["ratf"] = UTF8ToGBK(u8"力量手套");
    s_itemNames["rag1"] = UTF8ToGBK(u8"敏捷便鞋");

    // 添加更多...
    s_itemNames["YOUR_ITEM_ID"] = UTF8ToGBK(u8"中文名称");
}
```

**重要提示：**
- ⚠️ 中文字符串前**必须**加 `u8` 前缀（如 `u8"中文"`）
- ⚠️ **必须**使用 `UTF8ToGBK()` 函数转换（War3 使用 GBK 编码）
- 正确格式：`s_itemNames["ID"] = UTF8ToGBK(u8"中文名");`

**查找物品ID的方法：**
1. 在 `DebugMode::ALL` 模式下运行
2. 观察 DEBUG 输出：`[DEBUG] NEW item found: 0x..., typeId=XXXX`
3. 将 `XXXX` 作为 ID，添加到映射表

---

## TextTag 大小和颜色配置

在 `ItemConfig.cpp` 中修改：

```cpp
// TextTag 大小（默认 0.024f，比之前的 0.046f 小）
float ItemConfig::s_textTagSize = 0.024f;

// TextTag 颜色 RGBA（默认金黄色）
int ItemConfig::s_textTagColor[4] = {255, 204, 0, 255};  // 金黄色
```

**常用颜色：**
```cpp
// 金黄色（默认）
{255, 204, 0, 255}

// 红色
{255, 0, 0, 255}

// 绿色
{0, 255, 0, 255}

// 蓝色
{0, 100, 255, 255}

// 白色
{255, 255, 255, 255}

// 橙色
{255, 165, 0, 255}
```

**调整大小：**
```cpp
s_textTagSize = 0.018f;  // 更小
s_textTagSize = 0.024f;  // 默认（推荐）
s_textTagSize = 0.030f;  // 更大
```

---

## 示例配置

### 配置 1：调试模式（看所有信息）
```cpp
DebugMode ItemConfig::s_debugMode = DebugMode::ALL;
```

### 配置 2：只显示重要神符
```cpp
DebugMode ItemConfig::s_debugMode = DebugMode::FILTERED;

void ItemConfig::Initialize() {
    // 添加重要神符（必须使用 UTF8ToGBK 转换）
    s_itemNames["rej1"] = UTF8ToGBK(u8"活力球");
    s_itemNames["rej2"] = UTF8ToGBK(u8"虚空宝石");
    s_itemNames["rej5"] = UTF8ToGBK(u8"双倍伤害");
    s_itemNames["rej6"] = UTF8ToGBK(u8"幻象");

    // 清空默认过滤列表
    s_filterList.clear();

    // 只添加这些重要神符
    s_filterList.insert("rej1");
    s_filterList.insert("rej2");
    s_filterList.insert("rej5");
    s_filterList.insert("rej6");
}
```

### 配置 3：只用 TextTag，不要屏幕通知
```cpp
DebugMode ItemConfig::s_debugMode = DebugMode::NONE;

// TextTag 仍会显示所有物品的中文名称
```

### 配置 4：自定义颜色和大小
```cpp
// 红色、大号 TextTag
float ItemConfig::s_textTagSize = 0.030f;
int ItemConfig::s_textTagColor[4] = {255, 0, 0, 255};  // 红色
```

---

## 文件修改位置

要修改配置，只需编辑 **`ItemConfig.cpp`** 文件：

1. **修改调试模式**：找到 `s_debugMode` 行
2. **添加物品名称**：在 `Initialize()` 函数中添加：
   ```cpp
   s_itemNames["ID"] = UTF8ToGBK(u8"名称");
   ```
   - ⚠️ **重要**：必须使用 `UTF8ToGBK()` 函数转换
   - ⚠️ **重要**：中文字符串前必须加 `u8` 前缀
3. **修改过滤列表**：在 `Initialize()` 函数中添加 `s_filterList.insert("ID");`
4. **修改 TextTag 大小**：找到 `s_textTagSize` 行
5. **修改 TextTag 颜色**：找到 `s_textTagColor` 行

修改后重新编译 DLL 即可生效。

---

## 常见问题

**Q: TextTag 不显示？**
A: 检查编译是否成功，War3 版本是否为 1.24e (6387)

**Q: 中文名称显示乱码？**
A: 确保使用了正确的格式：`s_itemNames["ID"] = UTF8ToGBK(u8"中文名");`
   - 必须使用 `UTF8ToGBK()` 函数（War3 使用 GBK 编码）
   - 必须使用 `u8` 前缀

**Q: 中文名称不显示？**
A: 确保在 `s_itemNames` 中添加了对应的物品ID映射，并使用了正确的格式

**Q: 想要更小/更大的文字？**
A: 修改 `s_textTagSize`，范围建议 0.015 ~ 0.040

**Q: TextTag 消失太慢？**
A: 定时器周期为 1 秒，物品消失后最多 1 秒内会清理 TextTag

**Q: 如何找到特定物品的ID？**
A: 使用 `DebugMode::ALL` 模式，观察 DEBUG 输出

---

## 构建说明

修改 `ItemConfig.cpp` 后，重新编译：

```batch
# 使用 Visual Studio
打开 ItemRuneDisplay.sln
选择 Release + Win32
构建 -> 重新生成解决方案

# 输出位置
bin\Release\ItemRuneDisplay.dll
```

---

## 技术说明

- **TextTag 显示**：不受调试模式影响，始终显示所有物品
- **屏幕通知**：受调试模式控制（ALL=全部，FILTERED=过滤，NONE=禁用）
- **中文名称**：自动应用到 TextTag 和屏幕通知
- **自动清理**：每秒检查物品是否还存在，自动隐藏消失物品的 TextTag
- **性能优化**：使用静态缓存，避免重复检测同一物品

---

欢迎根据需要调整配置！
