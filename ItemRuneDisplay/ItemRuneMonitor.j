//===================================================================================
// ItemRuneMonitor.j - 物品和神符监控脚本
// 用于 Warcraft III 1.24e
// 使用方法：将此脚本注入到地图的 war3map.j 中
//===================================================================================

globals
    // 配置
    constant boolean SHOW_ALL_ITEMS = false      // 是否显示所有物品
    constant boolean SHOW_POWERUPS = true        // 是否显示神符
    constant real SCAN_INTERVAL = 0.5            // 扫描间隔（秒）

    // 内部变量
    timer g_ScanTimer = null
    rect g_MapBounds = null
    item g_EnumItem = null

    // 已显示物品缓存（避免重复显示）
    integer array g_DisplayedItems
    integer g_DisplayedCount = 0
    constant integer MAX_TRACKED = 200
endglobals

//===================================================================================
// 辅助函数
//===================================================================================

// 检查物品是否已显示
function IsItemDisplayed takes item whichItem returns boolean
    local integer i = 0
    local integer itemId = GetHandleId(whichItem)

    loop
        exitwhen i >= g_DisplayedCount
        if g_DisplayedItems[i] == itemId then
            return true
        endif
        set i = i + 1
    endloop

    return false
endfunction

// 标记物品为已显示
function MarkItemDisplayed takes item whichItem returns nothing
    local integer itemId = GetHandleId(whichItem)

    if g_DisplayedCount < MAX_TRACKED then
        set g_DisplayedItems[g_DisplayedCount] = itemId
        set g_DisplayedCount = g_DisplayedCount + 1
    endif
endfunction

// 将整数ID转换为字符串（4字符）
function IntegerToFourCC takes integer id returns string
    local string result = ""
    local integer i
    local integer char

    // 简化版本：直接显示整数
    return I2S(id)
endfunction

// 获取物品类型名称
function GetItemTypeName takes integer typeId returns string
    // 这里可以添加常见物品的名称映射
    // 简化版本：返回类型ID

    // 常见神符
    if typeId == 'afac' then  // 加速神符示例
        return "加速神符"
    elseif typeId == 'rnsp' then  // 恢复神符示例
        return "恢复神符"
    endif

    // 默认返回ID
    return IntegerToFourCC(typeId)
endfunction

// 显示物品信息
function DisplayItemInfo takes item whichItem, string prefix, string color returns nothing
    local integer typeId = GetItemTypeId(whichItem)
    local real x = GetItemX(whichItem)
    local real y = GetItemY(whichItem)
    local string message

    set message = color + "[" + prefix + "]|r " + GetItemTypeName(typeId) + " @ (" + I2S(R2I(x)) + ", " + I2S(R2I(y)) + ")"

    call DisplayTimedTextToPlayer(GetLocalPlayer(), 0, 0, 10.0, message)
    call PingMinimap(x, y, 3.0)
endfunction

//===================================================================================
// 核心逻辑
//===================================================================================

// 物品枚举回调
function EnumItemCallback takes nothing returns nothing
    local item i = GetEnumItem()

    // 跳过已显示的物品
    if IsItemDisplayed(i) then
        return
    endif

    // 跳过被拾取的物品
    if IsItemOwned(i) then
        return
    endif

    // 检查物品是否存活
    if GetWidgetLife(i) <= 0 then
        return
    endif

    // 检查是否为神符
    if SHOW_POWERUPS and IsItemPowerup(i) then
        local integer typeId = GetItemTypeId(i)

        // 过滤无用道具
        if typeId == 'I0KK' or typeId == 'I0HM' or typeId == 'KK0I' or typeId == 'MH0I' then
            return
        endif

        call DisplayItemInfo(i, "神符", "|cffffcc00")
        call MarkItemDisplayed(i)
        return
    endif

    // 显示普通物品
    if SHOW_ALL_ITEMS and not IsItemOwned(i) then
        call DisplayItemInfo(i, "物品", "|cff00ff00")
        call MarkItemDisplayed(i)
    endif

    set i = null
endfunction

// 定时扫描
function ScanItems takes nothing returns nothing
    call EnumItemsInRect(g_MapBounds, null, function EnumItemCallback)
endfunction

// 定时器回调
function TimerCallback takes nothing returns nothing
    call ScanItems()
endfunction

//===================================================================================
// 初始化
//===================================================================================

function InitItemRuneMonitor takes nothing returns nothing
    // 创建地图边界矩形
    set g_MapBounds = GetWorldBounds()

    // 创建定时器
    set g_ScanTimer = CreateTimer()
    call TimerStart(g_ScanTimer, SCAN_INTERVAL, true, function TimerCallback)

    // 显示启动消息
    call DisplayTimedTextToPlayer(GetLocalPlayer(), 0, 0, 5.0, "|cff00ff00物品和神符监控已启动|r")
endfunction

//===================================================================================
// 地图初始化时调用（需要在地图脚本中添加）
//===================================================================================
// 在地图的初始化函数中添加以下行：
// call InitItemRuneMonitor()
//===================================================================================
