<#
drive.ps1 -- PsigMatcher E2E 驱动脚本

真实启动 PsigMatcher.exe，向主窗口注入键盘事件（Ctrl+快捷键）与 WM_CLOSE，
并截取客户区画面到 out/verify/shot_xx.png，供 analyze.py 做像素断言。

注入方式说明：
- 键盘：keybd_event（Win32 原始输入 API），可真实置位修饰键状态（Ctrl/Shift），
  Qt 才能正确识别 Ctrl+快捷键；PostMessage 注入的按键不改变 GetKeyState 状态，
  目标无法识别修饰键组合，故键盘用 keybd_event，仍属 Win32 API、不依赖 UI 自动化框架。
- 关闭：PostMessage(WM_CLOSE) 注入窗口关闭。
- 截图：排除标题栏，仅截客户区，保证稳定性对比。
#>
param(
    [string]$Exe    = "C:\d\project\agent\env\gui\out\bin\Release\PsigMatcher.exe",
    [string]$OutDir = "C:\d\project\agent\env\gui\out\verify",
    [int]$StabilizeMs = 900
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms

Add-Type @"
using System;
using System.Runtime.InteropServices;
public class E2ENative {
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern IntPtr SetActiveWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern IntPtr SetFocus(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, IntPtr lpdwProcessId);
    [DllImport("user32.dll")] public static extern bool AttachThreadInput(uint idAttach, uint idAttachTo, bool fAttach);
    [DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId();
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hWnd, out RECT lpRect);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr hWnd, ref POINT lpPoint);
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int X, int Y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint dwFlags, uint dx, uint dy, uint dwData, UIntPtr dwExtraInfo);
    [DllImport("user32.dll")] public static extern IntPtr PostMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll")] public static extern void keybd_event(byte bVk, byte bScan, uint dwFlags, UIntPtr dwExtraInfo);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
}
"@

$VK_CONTROL = 0x11
$VK_SHIFT   = 0x10
$VK_ESCAPE  = 0x1B
$VK_F1      = 0x70
$KEYEVENTF_KEYUP = 0x0002
$MOUSEEVENTF_LEFTDOWN = 0x0002
$MOUSEEVENTF_LEFTUP   = 0x0004
$WM_CLOSE = 0x0010

# 强制目标窗口成为前台并获得键盘焦点：附加到当前前台线程后调用
# BringWindowToTop/SetForegroundWindow/SetActiveWindow，可绕过 Windows 前台锁定，
# 否则 keybd_event 注入的按键会落到其它窗口，目标收不到。
function Force-Foreground([IntPtr]$hwnd) {
    $fg = [E2ENative]::GetForegroundWindow()
    $fgThread = [E2ENative]::GetWindowThreadProcessId($fg, [IntPtr]::Zero)
    $curThread = [E2ENative]::GetCurrentThreadId()
    if ($fgThread -ne 0 -and $fgThread -ne $curThread) {
        [void][E2ENative]::AttachThreadInput($curThread, $fgThread, $true)
    }
    [void][E2ENative]::ShowWindow($hwnd, 5)          # SW_SHOW
    [void][E2ENative]::BringWindowToTop($hwnd)
    [void][E2ENative]::SetForegroundWindow($hwnd)
    [void][E2ENative]::SetActiveWindow($hwnd)
    [void][E2ENative]::SetFocus($hwnd)
    if ($fgThread -ne 0 -and $fgThread -ne $curThread) {
        [void][E2ENative]::AttachThreadInput($curThread, $fgThread, $false)
    }
    # 点击客户区中心，进一步确保键盘焦点落到本窗口
    $rc = New-Object E2ENative+RECT
    [void][E2ENative]::GetClientRect($hwnd, [ref]$rc)
    $pt = New-Object E2ENative+POINT
    $pt.X = [int](($rc.Right - $rc.Left) / 2)
    $pt.Y = [int](($rc.Bottom - $rc.Top) / 2)
    [void][E2ENative]::ClientToScreen($hwnd, [ref]$pt)
    [void][E2ENative]::SetCursorPos($pt.X, $pt.Y)
    [E2ENative]::mouse_event($MOUSEEVENTF_LEFTDOWN, 0, 0, 0, [UIntPtr]::Zero)
    [E2ENative]::mouse_event($MOUSEEVENTF_LEFTUP, 0, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 60
}

function Press-CtrlKey([byte]$vk) {
    [E2ENative]::keybd_event($VK_CONTROL, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 40
    [E2ENative]::keybd_event($vk, 0, 0, [UIntPtr]::Zero)
    [E2ENative]::keybd_event($vk, 0, $KEYEVENTF_KEYUP, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 40
    [E2ENative]::keybd_event($VK_CONTROL, 0, $KEYEVENTF_KEYUP, [UIntPtr]::Zero)
}

function Press-Key([byte]$vk) {
    [E2ENative]::keybd_event($vk, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 30
    [E2ENative]::keybd_event($vk, 0, $KEYEVENTF_KEYUP, [UIntPtr]::Zero)
}

function Capture-Client([IntPtr]$hwnd, [string]$path) {
    $rc = New-Object E2ENative+RECT
    [void][E2ENative]::GetClientRect($hwnd, [ref]$rc)
    $w = $rc.Right - $rc.Left
    $h = $rc.Bottom - $rc.Top
    if ($w -le 0 -or $h -le 0) { throw "client rect invalid: ${w}x${h}" }
    $origin = New-Object E2ENative+POINT
    $origin.X = $rc.Left; $origin.Y = $rc.Top
    [void][E2ENative]::ClientToScreen($hwnd, [ref]$origin)
    $bmp = New-Object System.Drawing.Bitmap($w, $h)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($origin.X, $origin.Y, 0, 0, (New-Object System.Drawing.Size($w, $h)))
    $g.Dispose()
    $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    Write-Host ("[shot] {0}  {1}x{2}" -f (Split-Path $path -Leaf), $w, $h)
    return @($w, $h)
}

[void][E2ENative]::SetProcessDPIAware()
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

Write-Host "[drive] launching $Exe"
$outLog = Join-Path $OutDir "PsigMatcher_out.log"
$errLog = Join-Path $OutDir "PsigMatcher_err.log"
# 用 System.Diagnostics.Process 直接启动：既支持 MainWindowHandle 轮询与 ExitCode 读取，
# 又支持标准输出/错误重定向（Start-Process -Redirect* 会导致 ExitCode 不可读）
$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $Exe
$psi.UseShellExecute = $false
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true
$psi.CreateNoWindow = $true
$proc = New-Object System.Diagnostics.Process
$proc.StartInfo = $psi
[void]$proc.Start()

# 等待主窗口句柄就绪
$hwnd = [IntPtr]::Zero
for ($i = 0; $i -lt 50; $i++) {
    Start-Sleep -Milliseconds 200
    $proc.Refresh()
    if ($proc.HasExited) { Write-Host "[drive] process exited early code=$($proc.ExitCode)"; exit 1 }
    if ($proc.MainWindowHandle -ne [IntPtr]::Zero) { $hwnd = $proc.MainWindowHandle; break }
}
if ($hwnd -eq [IntPtr]::Zero) { Write-Host "[drive] no main window"; Stop-Process -Id $proc.Id -Force; exit 1 }
Write-Host "[drive] hwnd=$hwnd title='$($proc.MainWindowTitle)'"

Force-Foreground $hwnd
Start-Sleep -Milliseconds $StabilizeMs

# E2E-01 启动主界面（v0.2.1：启动即进程选择页；启动自动枚举进行中）
Force-Foreground $hwnd
Start-Sleep -Milliseconds 200
[void](Capture-Client $hwnd (Join-Path $OutDir "shot_01_start.png"))

# E2E-02 进程表被填充（C-09 启动自动枚举 / Ctrl+R 刷新），等待异步枚举完成
Force-Foreground $hwnd
Press-CtrlKey 0x52
Start-Sleep -Milliseconds 2800
[void](Capture-Client $hwnd (Join-Path $OutDir "shot_02_process_refreshed.png"))

# E2E-03 切换到特征码编辑页 (Ctrl+1)
Force-Foreground $hwnd
Press-CtrlKey 0x31
Start-Sleep -Milliseconds $StabilizeMs
[void](Capture-Client $hwnd (Join-Path $OutDir "shot_03_signature.png"))

# E2E-04 新增特征码行 (Ctrl+N)
Force-Foreground $hwnd
Press-CtrlKey 0x4E
Start-Sleep -Milliseconds $StabilizeMs
[void](Capture-Client $hwnd (Join-Path $OutDir "shot_04_addrow.png"))

# E2E-03 切换到匹配结果页 (Ctrl+3)
Force-Foreground $hwnd
Press-CtrlKey 0x33
Start-Sleep -Milliseconds $StabilizeMs
[void](Capture-Client $hwnd (Join-Path $OutDir "shot_05_result.png"))

# E2E-06 关于弹窗 (F1) —— 模态 AboutDialog 覆盖主窗口
Force-Foreground $hwnd
Press-Key $VK_F1
Start-Sleep -Milliseconds $StabilizeMs
[void](Capture-Client $hwnd (Join-Path $OutDir "shot_06_about.png"))
# 关闭关于弹窗 (Esc)：模态对话框已持有焦点，直接注入
Press-Key $VK_ESCAPE
Start-Sleep -Milliseconds $StabilizeMs

# E2E-07 关闭程序（未保存改动 → 关闭保存提示；选「不保存」后退出）
# 前置已由 E2E-04 (Ctrl+N 增行) 产生未保存改动，故 WM_CLOSE 会弹出 [保存/不保存/取消] 模态提示。
Write-Host "[drive] sending WM_CLOSE"
[void][E2ENative]::PostMessage($hwnd, $WM_CLOSE, [IntPtr]::Zero, [IntPtr]::Zero)
# 注意：此处不可再调 Force-Foreground（其会点击客户区中心，可能误触模态框按钮）——
# 提示框由 WM_CLOSE 触发后已持有焦点，直接截屏并注入按键。
Start-Sleep -Milliseconds 900
[void](Capture-Client $hwnd (Join-Path $OutDir "shot_07_close_prompt.png"))
# 用提示按钮的确定性键盘入口选「不保存」(Ctrl+D，见 design/v0.3.2/gui/05_GUI功能列表与窗口定义.md)
Press-CtrlKey 0x44
$exited = $proc.WaitForExit(8000)
if (-not $exited) {
    Write-Host "[drive] process did not exit in time, killing"
    Stop-Process -Id $proc.Id -Force
    exit 1
}
$proc.Refresh()
$code = $proc.ExitCode
# 落盘重定向的标准输出/错误（GUI 通常无输出，保留以满足可排查性）
try { $so = $proc.StandardOutput.ReadToEnd(); Set-Content -Path $outLog -Value $so -Encoding utf8 } catch {}
try { $se = $proc.StandardError.ReadToEnd();  Set-Content -Path $errLog -Value $se -Encoding utf8 } catch {}
Write-Host "[drive] exit code = $code"
Set-Content -Path (Join-Path $OutDir "exit_code.txt") -Value $code -Encoding ascii
if ($code -ne 0) { exit 1 }
Write-Host "[drive] OK"
exit 0
