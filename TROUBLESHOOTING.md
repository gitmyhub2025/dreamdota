# 故障排查指南

## 问题：切换分支后看不到 ItemRuneDisplay

如果你切换到分支后看不到 `ItemRuneDisplay` 文件夹，请按以下步骤操作：

### 步骤 1: 检查当前状态

```bash
# 查看当前分支
git branch

# 应该显示：* claude/analyze-item-symbol-logic-011CUQ21HjyXwDUTf1Ff5xA9

# 查看当前目录
ls -la

# 查看 git 日志
git log --oneline -3
# 应该看到：
# d1079b8 Add quick start guide for ItemRuneDisplay
# 9eaae83 Add ItemRuneDisplay - 简化的物品和神符显示工具
```

### 步骤 2: 拉取最新更改

```bash
# 从远程拉取最新代码
git pull origin claude/analyze-item-symbol-logic-011CUQ21HjyXwDUTf1Ff5xA9

# 或者重置到远程分支
git reset --hard origin/claude/analyze-item-symbol-logic-011CUQ21HjyXwDUTf1Ff5xA9
```

### 步骤 3: 验证文件

```bash
# 查看 ItemRuneDisplay 目录
ls -la ItemRuneDisplay/

# 应该看到：
# ItemRuneDisplay_6387.cpp
# ItemRuneMonitor.j
# README.md
# QUICK_START.md
# 等等...
```

### 步骤 4: 如果还是看不到 - 重新克隆

```bash
# 删除旧仓库
cd ..
rm -rf dreamdota

# 重新克隆并切换分支
git clone https://github.com/gitmyhub2025/dreamdota.git
cd dreamdota
git checkout claude/analyze-item-symbol-logic-011CUQ21HjyXwDUTf1Ff5xA9

# 查看文件
ls -la ItemRuneDisplay/
```

### 步骤 5: 确认提交历史

```bash
# 查看分支的文件树
git ls-tree -r HEAD --name-only | grep ItemRuneDisplay

# 应该输出：
# ItemRuneDisplay/ItemMonitor.cpp
# ItemRuneDisplay/ItemRuneDisplay_6387.cpp
# ItemRuneDisplay/ItemRuneDisplay_Full.cpp
# ItemRuneDisplay/ItemRuneMonitor.j
# ItemRuneDisplay/PROJECT_SUMMARY.md
# ItemRuneDisplay/QUICK_START.md
# ItemRuneDisplay/README.md
# ItemRuneDisplay/USAGE.md
# ItemRuneDisplay/build.bat
# ItemRuneDisplay/exports.def
# ItemRuneDisplay/main.cpp
```

## 可能的原因

1. **本地未拉取最新代码**
   - 解决：`git pull origin <分支名>`

2. **在错误的目录**
   - 解决：确保在 `dreamdota` 根目录下，而不是 `DreamWarcraft` 子目录

3. **分支未正确切换**
   - 解决：`git checkout claude/analyze-item-symbol-logic-011CUQ21HjyXwDUTf1Ff5xA9`

4. **本地文件冲突**
   - 解决：`git reset --hard origin/claude/analyze-item-symbol-logic-011CUQ21HjyXwDUTf1Ff5xA9`

## 验证最终结果

执行以下命令，你应该看到所有文件：

```bash
cd dreamdota/ItemRuneDisplay
cat README.md | head -5
```

应该输出：
```
# ItemRuneDisplay - 魔兽争霸III 物品和神符显示工具

基于 **DreamDota** 项目简化而来，专注于地图上物品和神符的实时显示。

## ✨ 功能特性
```

## 快速验证脚本

复制粘贴运行：

```bash
#!/bin/bash
echo "=== 检查 Git 状态 ==="
git status
echo ""
echo "=== 当前分支 ==="
git branch | grep "*"
echo ""
echo "=== 最近提交 ==="
git log --oneline -3
echo ""
echo "=== ItemRuneDisplay 文件 ==="
ls -la ItemRuneDisplay/ 2>/dev/null || echo "❌ ItemRuneDisplay 目录不存在！"
echo ""
echo "=== Git 文件树中的 ItemRuneDisplay ==="
git ls-tree -r HEAD --name-only | grep ItemRuneDisplay | head -5
```

---

如果执行以上所有步骤后仍然看不到文件，请提供以下信息：

1. `git remote -v` 的输出
2. `git branch -a` 的输出
3. `pwd` 的输出
4. `ls -la` 的输出

我会帮你进一步诊断问题！
