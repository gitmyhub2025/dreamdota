@echo off
REM ========================================
REM ItemRuneDisplay 编译脚本
REM 要求: Visual Studio 2010 或更高版本
REM ========================================

echo 正在编译 ItemRuneDisplay.dll...

REM 设置 Visual Studio 路径（根据你的安装路径修改）
REM VS2010
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"

REM VS2015
REM set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"

REM VS2019
REM set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"

REM 初始化编译环境
if exist %VCVARS% (
    call %VCVARS% x86
) else (
    echo 错误: 找不到 Visual Studio!
    echo 请修改 build.bat 中的 VCVARS 路径
    pause
    exit /b 1
)

REM 编译选项
REM /LD      - 生成 DLL
REM /MT      - 静态链接 CRT
REM /O2      - 优化
REM /EHsc    - C++ 异常处理
REM /Fe      - 输出文件名

echo.
echo 开始编译...
echo.

cl.exe /LD /MT /O2 /EHsc /Fe:ItemRuneDisplay.dll ItemRuneDisplay_6387.cpp /link /DEF:exports.def kernel32.lib user32.lib

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo 编译成功！
    echo 输出: ItemRuneDisplay.dll
    echo ========================================

    REM 清理临时文件
    del *.obj *.exp *.lib 2>nul
) else (
    echo.
    echo ========================================
    echo 编译失败！
    echo ========================================
)

pause
