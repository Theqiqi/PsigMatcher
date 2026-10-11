<#
drive.ps1 -- psig_gui E2E 驱动脚本

真实启动 psig_gui.exe，向主窗口注入键盘事件（Ctrl+快捷键）与 WM_CLOSE，
并截取客户区画面到 out/verify/shot_xx.png，供 analyze.py 做像素断言。

注入方式说明：
- 键盘：keybd_event（Win32 原始输入 API），可真实置位修饰键状态（Ctrl/Shift），
  Qt 才能正确识别 Ctrl+快捷键；PostMessage 注入的按键不改变 GetKeyState 状态，
  目标无法识别修饰键组合，故键盘用 keybd_event，仍属 Win32 API、不依赖 UI 自动化框架。
- 关闭：PostMessage(WM_CLOSE) 注入窗口关闭。
- 截图：排除标题栏，仅截客户区，保证稳定性对比。
#>
param(
    [string]$Exe    = "C:\d\project\agent\env\gui\out\bin\Release\psig_gui.exe",
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
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hWnd, out RECT lpRect);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr hWnd, ref POINT lpPoint);
    [DllImport("user32.dll")] public static extern IntPtr PostMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll")] public static extern void keybd_event(byte bVk, byte bScan, uint dwFlags, UIntPtr dwExtraInfo);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
}
"@

$VK_CONTROL = 0x11
$VK_SHIFT   = 0x10
$KEYEVENTF_KEYUP = 0x0002
$WM_CLOSE = 0x0010

function Press-CtrlKey([byte]$vk) {
    [E2ENative]::keybd_event($VK_CONTROL, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 40
    [E2ENative]::keybd_event($vk, 0, 0, [UIntPtr]::Zero)
    [E2ENative]::keybd_event($vk, 0, $KEYEVENTF_KEYUP, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 40
    [E2ENative]::keybd_event($VK_CONTROL, 0, $KEYEVENTF_KEYUP, [UIntPtr]::Zero)
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
$outLog = Join-Path $OutDir "psig_gui_out.log"
$errLog = Join-Path $OutDir "psig_gui_err.log"
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

[void][E2ENative]::ShowWindow($hwnd, 5)   # SW_SHOW
[void][E2ENative]::SetForegroundWindow($hwnd)
Start-Sleep -Milliseconds $StabilizeMs

# E2E-01 启动主界面
[void](Capture-Client $hwnd (Join-Path $OutDir "shot_01_start.png"))

# E2E-02 切换到进程选择页 (Ctrl+2)
Press-CtrlKey 0x32
Start-Sleep -Milliseconds $StabilizeMs
[void](Capture-Client $hwnd (Join-Path $OutDir "shot_02_process.png"))

# E2E-04 刷新进程 (Ctrl+R)，等待异步枚举
Press-CtrlKey 0x52
Start-Sleep -Milliseconds 2500
[void](Capture-Client $hwnd (Join-Path $OutDir "shot_03_process_refreshed.png"))

# E2E-02 切回特征码编辑页 (Ctrl+1)
Press-CtrlKey 0x31
Start-Sleep -Milliseconds $StabilizeMs
[void](Capture-Client $hwnd (Join-Path $OutDir "shot_04_signature.png"))

# E2E-03 新增特征码行 (Ctrl+N)
Press-CtrlKey 0x4E
Start-Sleep -Milliseconds $StabilizeMs
[void](Capture-Client $hwnd (Join-Path $OutDir "shot_05_addrow.png"))

# E2E-02 切换到匹配结果页 (Ctrl+3)
Press-CtrlKey 0x33
Start-Sleep -Milliseconds $StabilizeMs
[void](Capture-Client $hwnd (Join-Path $OutDir "shot_06_result.png"))

# E2E-06 关闭程序 (WM_CLOSE)
Write-Host "[drive] sending WM_CLOSE"
[void][E2ENative]::PostMessage($hwnd, $WM_CLOSE, [IntPtr]::Zero, [IntPtr]::Zero)
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
