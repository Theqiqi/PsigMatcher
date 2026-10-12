# extract_exe_icon.ps1 -- 用 Windows 自己的接口取出「外壳关联到 exe 的图标」，作为独立于自研 PE 解析器的证据。
#
# 原理：System.Drawing.Icon::ExtractAssociatedIcon 走的是 Windows 外壳用于资源管理器显示的同一套
#       「文件关联图标」逻辑；若 exe 没有图标资源，它会返回通用 exe 图标（与本项目图标明显不同）。
#
# 用法:
#   powershell -NoProfile -ExecutionPolicy Bypass -File extract_exe_icon.ps1 -Exe <exe 绝对路径> -OutDir <输出目录>
# 产物: <OutDir>/exe_associated_icon.png（32x32，供 Python 侧与 res/app_32.png 逐像素比对）
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [Parameter(Mandatory = $true)][string]$OutDir
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $Exe)) {
    Write-Host "[extract_exe_icon] missing exe: $Exe"
    exit 1
}
if (-not (Test-Path -LiteralPath $OutDir)) {
    New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
}

Add-Type -AssemblyName System.Drawing

$icon = [System.Drawing.Icon]::ExtractAssociatedIcon($Exe)
if ($null -eq $icon) {
    Write-Host "[extract_exe_icon] Windows 未返回任何关联图标"
    exit 1
}

$bitmap = $icon.ToBitmap()
$outPath = Join-Path $OutDir 'exe_associated_icon.png'
$bitmap.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Png)

Write-Host "[extract_exe_icon] extracted $($bitmap.Width)x$($bitmap.Height) -> $outPath"
$bitmap.Dispose()
$icon.Dispose()
exit 0
