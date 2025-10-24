# ItemRuneDisplay 编译指南

## 方法 1: 使用 Visual Studio（推荐）

### 步骤：

1. **打开项目**
   - 双击 `ItemRuneDisplay.sln`
   - 或在 Visual Studio 中打开

2. **选择配置**
   - 工具栏选择 `Release | Win32`

3. **编译**
   - 菜单：生成 → 生成解决方案
   - 或按 `F7`

4. **输出位置**
   - `bin\Release\ItemRuneDisplay.dll`

### 支持的 Visual Studio 版本：
- Visual Studio 2010
- Visual Studio 2012
- Visual Studio 2013
- Visual Studio 2015
- Visual Studio 2017
- Visual Studio 2019
- Visual Studio 2022

## 方法 2: 使用 MSBuild 命令行

```batch
# 打开 "Developer Command Prompt for VS"
# 或者先运行 vcvarsall.bat

# 编译 Release 版本
msbuild ItemRuneDisplay.sln /p:Configuration=Release /p:Platform=Win32

# 输出: bin\Release\ItemRuneDisplay.dll
```

## 方法 3: 使用批处理脚本

```batch
# 双击运行
build.bat
```

这会自动查找 Visual Studio 并编译。

## 方法 4: 使用 cl.exe 直接编译

```batch
# 设置环境
"C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86

# 编译
cl.exe /LD /MT /O2 /EHsc /Fe:ItemRuneDisplay.dll ItemRuneDisplay_6387.cpp /link /DEF:exports.def kernel32.lib user32.lib

# 清理
del *.obj *.exp *.lib
```

## 编译选项说明

- `/LD` - 生成 DLL
- `/MT` - 静态链接 CRT（无需 msvcr*.dll）
- `/O2` - 优化代码
- `/EHsc` - C++ 异常处理
- `/Fe:` - 指定输出文件名

## 输出文件

**Release 版本**:
- `bin\Release\ItemRuneDisplay.dll` - 主 DLL（约 50KB）

**Debug 版本**:
- `bin\Debug\ItemRuneDisplay.dll` - Debug 版本
- `bin\Debug\ItemRuneDisplay.pdb` - 调试符号

## 故障排查

### 错误: 找不到 Windows SDK
**解决**: 安装 Windows SDK 或修改项目平台工具集

在 Visual Studio 中:
1. 右键项目 → 属性
2. 配置属性 → 常规
3. 平台工具集 → 选择已安装的版本

### 错误: 链接器错误 LNK2001
**解决**: 确保 `exports.def` 文件存在

### 错误: C1083: 无法打开包括文件
**解决**: 确保安装了 Windows SDK

## 验证编译结果

```batch
# 检查文件大小（应该约 50KB）
dir bin\Release\ItemRuneDisplay.dll

# 使用 dumpbin 查看导出（可选）
dumpbin /exports bin\Release\ItemRuneDisplay.dll
```

## 下一步

编译完成后，参考 `USAGE.md` 了解如何注入和使用 DLL。

---

**推荐配置**: Visual Studio 2010+ | Release | Win32
