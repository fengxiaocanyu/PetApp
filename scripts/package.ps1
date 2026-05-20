<#
.SYNOPSIS
    PetApp 打包脚本 - 生成可移植 ZIP 包
.DESCRIPTION
    1. 编译 Release 版本
    2. 使用 windeployqt 拷贝 Qt 运行时
    3. 复制 VC++ 运行时 DLL
    4. 复制 plugins/、data/、config/ 目录
    5. 打包为 ZIP
.NOTES
    需要：Visual Studio 2022, Qt 6.8.3, CMake, Ninja, 7-Zip
#>

param(
    [string]$BuildDir = "build_release",
    [string]$DeployDir = "deploy",
    [string]$QtDir = "E:/work/QT/6.8.3/msvc2022_64",
    [string]$OutputZip = "PetApp_v1.0.0.zip"
)

# 错误处理
$ErrorActionPreference = "Stop"

# 获取脚本所在目录（项目根目录）
$ProjectRoot = Split-Path -Parent $PSScriptRoot
Write-Host "=== PetApp 打包脚本 ===" -ForegroundColor Cyan
Write-Host "项目目录: $ProjectRoot"

# ============================================================
# 步骤 1：编译 Release 版本
# ============================================================
Write-Host "`n[1/7] 编译 Release 版本..." -ForegroundColor Yellow

# 设置 VS 环境
$vsPath = "D:\software\work\VS2022\Common7\Tools\VsDevCmd.bat"
$cmakePath = "D:\software\work\VS2022\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
$ninjaPath = "D:\software\work\VS2022\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"

$env:Path = "$cmakePath;$ninjaPath;$env:Path"

# 配置
Push-Location $ProjectRoot
try {
    # 创建构建目录
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
    }
    New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

    # 运行 VsDevCmd 并执行 CMake
    $cmakeCmd = "`"$vsPath`" -arch=x64 -host_arch=x64 -no_logo && cmake -B `"$BuildDir`" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=`"$QtDir`""
    cmd /c "`"$vsPath`" -arch=x64 -host_arch=x64 -no_logo && cmake -B `"$BuildDir`" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=`"$QtDir`""
    if ($LASTEXITCODE -ne 0) {
        throw "CMake 配置失败"
    }

    # 编译
    cmd /c "`"$vsPath`" -arch=x64 -host_arch=x64 -no_logo && cmake --build `"$BuildDir`" --config Release"
    if ($LASTEXITCODE -ne 0) {
        throw "编译失败"
    }

    Write-Host "  编译成功" -ForegroundColor Green
}
finally {
    Pop-Location
}

# ============================================================
# 步骤 2：创建部署目录
# ============================================================
Write-Host "`n[2/7] 创建部署目录..." -ForegroundColor Yellow

# 清理旧的部署目录
if (Test-Path $DeployDir) {
    Remove-Item -Recurse -Force $DeployDir
}

# 创建目录结构
$dirs = @(
    "$DeployDir/plugins",
    "$DeployDir/data/memory",
    "$DeployDir/data/models",
    "$DeployDir/config",
    "$DeployDir/logs/crash",
    "$DeployDir/platforms",
    "$DeployDir/styles",
    "$DeployDir/iconengines",
    "$DeployDir/imageformats"
)

foreach ($dir in $dirs) {
    New-Item -ItemType Directory -Force -Path "$ProjectRoot/$dir" | Out-Null
}

Write-Host "  目录结构创建完成" -ForegroundColor Green

# ============================================================
# 步骤 3：复制主程序和技能插件
# ============================================================
Write-Host "`n[3/7] 复制主程序和技能插件..." -ForegroundColor Yellow

# 复制主程序
Copy-Item "$ProjectRoot/$BuildDir/PetApp.exe" "$ProjectRoot/$DeployDir/PetApp.exe" -Force
Write-Host "  复制 PetApp.exe"

# 复制技能插件
$pluginExes = @(
    "skill_echo.exe",
    "skill_translate.exe",
    "skill_ocr.exe",
    "skill_chat.exe"
)

foreach ($exe in $pluginExes) {
    $src = "$ProjectRoot/plugins/$exe"
    if (Test-Path $src) {
        Copy-Item $src "$ProjectRoot/$DeployDir/plugins/$exe" -Force
        Write-Host "  复制 plugins/$exe"
    }
}

# 复制 manifest 文件
$manifests = @(
    "skill_echo.manifest.json",
    "skill_translate.manifest.json",
    "skill_ocr.manifest.json",
    "skill_chat.manifest.json"
)

foreach ($mf in $manifests) {
    $src = "$ProjectRoot/plugins/$mf"
    if (Test-Path $src) {
        Copy-Item $src "$ProjectRoot/$DeployDir/plugins/$mf" -Force
        Write-Host "  复制 plugins/$mf"
    }
}

# 复制 Q&A 数据
$qaData = "$ProjectRoot/plugins/qa_data.json"
if (Test-Path $qaData) {
    Copy-Item $qaData "$ProjectRoot/$DeployDir/plugins/qa_data.json" -Force
    Write-Host "  复制 plugins/qa_data.json"
}

# ============================================================
# 步骤 4：使用 windeployqt 拷贝 Qt 运行时
# ============================================================
Write-Host "`n[4/7] 使用 windeployqt 拷贝 Qt 运行时..." -ForegroundColor Yellow

$windeployqt = "$QtDir/bin/windeployqt.exe"
if (Test-Path $windeployqt) {
    Push-Location $ProjectRoot/$DeployDir
    try {
        & $windeployqt --release --no-translations --no-system-d3d-compiler `
            --dir . PetApp.exe 2>&1 | Out-Null
        Write-Host "  windeployqt 完成" -ForegroundColor Green
    }
    finally {
        Pop-Location
    }
} else {
    Write-Warning "  未找到 windeployqt，跳过 Qt 运行时部署"
}

# ============================================================
# 步骤 5：复制 VC++ 运行时 DLL
# ============================================================
Write-Host "`n[5/7] 复制 VC++ 运行时 DLL..." -ForegroundColor Yellow

$vcRedistDir = "D:\software\work\VS2022\VC\Redist\MSVC\14.44.35112\x64\Microsoft.VC143.CRT"
$systemDir = "C:\Windows\System32"

$vcDlls = @(
    "vcruntime140.dll",
    "vcruntime140_1.dll",
    "msvcp140.dll",
    "msvcp140_1.dll",
    "msvcp140_2.dll",
    "msvcp140_atomic_wait.dll",
    "msvcp140_codecvt_ids.dll",
    "concrt140.dll"
)

foreach ($dll in $vcDlls) {
    $src = "$vcRedistDir/$dll"
    if (Test-Path $src) {
        Copy-Item $src "$ProjectRoot/$DeployDir/$dll" -Force
        Write-Host "  复制 $dll"
    } else {
        # 尝试从系统目录复制
        $sysSrc = "$systemDir/$dll"
        if (Test-Path $sysSrc) {
            Copy-Item $sysSrc "$ProjectRoot/$DeployDir/$dll" -Force
            Write-Host "  复制 $dll (系统目录)"
        } else {
            Write-Warning "  未找到 $dll"
        }
    }
}

# ============================================================
# 步骤 6：复制配置和数据文件
# ============================================================
Write-Host "`n[6/7] 复制配置和数据文件..." -ForegroundColor Yellow

# 复制默认配置
$configSrc = "$ProjectRoot/config/default_settings.json"
if (Test-Path $configSrc) {
    Copy-Item $configSrc "$ProjectRoot/$DeployDir/config/default_settings.json" -Force
    Write-Host "  复制 config/default_settings.json"
}

# 创建默认 memory.json
$defaultMemory = @"
{
  "version": "1.0.0",
  "created_at": "$(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')",
  "data": {}
}
"@
Set-Content -Path "$ProjectRoot/$DeployDir/data/memory/memory.json" -Value $defaultMemory
Write-Host "  创建 data/memory/memory.json"

# ============================================================
# 创建 run.bat（解决双击运行工作目录问题）
# ============================================================
Write-Host "`n[7/7] 创建 run.bat..." -ForegroundColor Yellow

$runBatContent = @"
@echo off
cd /d "%~dp0"
start "" "%~dp0PetApp.exe"
"@
Set-Content -Path "$ProjectRoot/$DeployDir/run.bat" -Value $runBatContent
Write-Host "  创建 run.bat"

# ============================================================
# 打包为 ZIP
# ============================================================
Write-Host "`n=== 打包为 ZIP ===" -ForegroundColor Cyan

# 查找 7-Zip
$sevenZipPaths = @(
    "C:\Program Files\7-Zip\7z.exe",
    "C:\Program Files (x86)\7-Zip\7z.exe",
    "${env:ProgramFiles}\7-Zip\7z.exe",
    "${env:ProgramFiles(x86)}\7-Zip\7z.exe"
)

$sevenZip = $null
foreach ($path in $sevenZipPaths) {
    if (Test-Path $path) {
        $sevenZip = $path
        break
    }
}

if ($sevenZip) {
    Push-Location $ProjectRoot
    try {
        # 删除旧的 ZIP
        if (Test-Path $OutputZip) {
            Remove-Item $OutputZip -Force
        }

        # 打包
        & $sevenZip a -tzip $OutputZip ./$DeployDir/* -mx9
        if ($LASTEXITCODE -eq 0) {
            Write-Host "`n打包成功!" -ForegroundColor Green
            Write-Host "输出文件: $ProjectRoot/$OutputZip" -ForegroundColor Green

            # 显示文件大小
            $fileInfo = Get-Item "$ProjectRoot/$OutputZip"
            $sizeMB = [math]::Round($fileInfo.Length / 1MB, 2)
            Write-Host "文件大小: ${sizeMB} MB" -ForegroundColor Green
        } else {
            Write-Warning "打包可能失败 (退出码: $LASTEXITCODE)"
        }
    }
    finally {
        Pop-Location
    }
} else {
    Write-Warning "未找到 7-Zip，跳过 ZIP 打包"
    Write-Host "部署目录已准备完成: $ProjectRoot/$DeployDir" -ForegroundColor Yellow
    Write-Host "请手动使用 7-Zip 或其他工具打包此目录" -ForegroundColor Yellow
}

Write-Host "`n=== 打包完成 ===" -ForegroundColor Cyan
Write-Host "部署目录: $ProjectRoot/$DeployDir" -ForegroundColor White
Write-Host "ZIP 文件: $ProjectRoot/$OutputZip" -ForegroundColor White