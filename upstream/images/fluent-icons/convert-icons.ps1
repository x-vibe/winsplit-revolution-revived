# Convert Fluent UI SVG icons to PNG/XPM/ICO for WinSplit Revolution
# Requires: ImageMagick v7+ (magick.exe in PATH)
#
# Run download-icons.ps1 first to download the SVG source files.

$scriptDir = $PSScriptRoot
$outputDir = Split-Path $scriptDir -Parent  # ../images/

Write-Host ""
Write-Host "=== Fluent UI Icon Converter ===" -ForegroundColor Cyan
Write-Host ""

# Check for ImageMagick
try {
    $null = Get-Command magick -ErrorAction Stop
    Write-Host "ImageMagick found." -ForegroundColor Green
}
catch {
    Write-Host "ERROR: ImageMagick not found in PATH." -ForegroundColor Red
    Write-Host "Install from: https://imagemagick.org/script/download.php#windows" -ForegroundColor Yellow
    exit 1
}

# Helper function to fix XPM variable names for C++ compilation
# ImageMagick generates "static char *filename[]" but wxWidgets expects "static char *filename_xpm[]"
function Fix-XpmVariableName {
    param([string]$xpmPath, [string]$expectedName)

    if (Test-Path $xpmPath) {
        $content = Get-Content $xpmPath -Raw
        # Replace "static char *<name>[]" with "static char *<name>_xpm[]"
        # The variable name is typically the filename without extension
        $content = $content -replace 'static char \*(\w+)\[\]', "static char *${expectedName}_xpm[]"
        Set-Content $xpmPath $content -NoNewline
    }
}

Write-Host ""
Write-Host "Converting menu icons (PNG 20x20)..." -ForegroundColor Cyan

# Menu icons (20x20 PNG)
$menuIcons = @("options", "hotkeys", "help", "about", "exit", "auto_start_true", "auto_start_false", "layout")

foreach ($icon in $menuIcons) {
    $svgPath = Join-Path $scriptDir "$icon.svg"
    $pngPath = Join-Path $outputDir "$icon.png"

    if (Test-Path $svgPath) {
        Write-Host "  $icon.svg -> $icon.png... " -NoNewline
        # Two-step conversion for reliability:
        # 1. Render SVG at high density to intermediate PNG
        # 2. Resize to final size
        $tempPng = Join-Path $scriptDir "temp_menu_$icon.png"
        & magick $svgPath -background none -density 300 $tempPng 2>$null
        & magick $tempPng -resize 20x20 $pngPath 2>$null
        Remove-Item $tempPng -ErrorAction SilentlyContinue
        if ($LASTEXITCODE -eq 0 -and (Test-Path $pngPath)) {
            Write-Host "OK" -ForegroundColor Green
        } else {
            Write-Host "FAILED" -ForegroundColor Red
        }
    } else {
        Write-Host "  Skipping $icon.svg (not found)" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "Converting tray icon (XPM 16x16)..." -ForegroundColor Cyan

# Tray icon (16x16 XPM) - icone.xpm
$trayBase16 = Join-Path $scriptDir "tray_base_16.svg"
if (Test-Path $trayBase16) {
    Write-Host "  tray_base_16.svg -> icone.xpm... " -NoNewline
    $tempPng = Join-Path $scriptDir "temp_tray.png"
    $xpmPath = Join-Path $outputDir "icone.xpm"

    & magick $trayBase16 -background none -density 96 $tempPng 2>$null
    & magick $tempPng -colors 256 $xpmPath 2>$null

    if ($LASTEXITCODE -eq 0 -and (Test-Path $xpmPath)) {
        Remove-Item $tempPng -ErrorAction SilentlyContinue
        # Fix variable name for C++ compilation
        Fix-XpmVariableName -xpmPath $xpmPath -expectedName "icone"
        Write-Host "OK" -ForegroundColor Green
    } else {
        Write-Host "FAILED" -ForegroundColor Red
    }
} else {
    Write-Host "  tray_base_16.svg not found" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Generating tray animation frames (XPM)..." -ForegroundColor Cyan

# Sync animation frames (tray2-6.xpm)
$traySync = Join-Path $scriptDir "tray_sync.svg"
if (Test-Path $traySync) {
    Write-Host "  tray_sync.svg -> tray2-6.xpm... " -NoNewline
    $tempPng = Join-Path $scriptDir "temp_sync.png"

    & magick $traySync -background none -density 96 -resize 16x16 $tempPng 2>$null

    # Create rotated versions for animation effect
    $success = $true
    for ($i = 2; $i -le 6; $i++) {
        $rotation = ($i - 2) * 72  # 0, 72, 144, 216, 288 degrees
        $xpmPath = Join-Path $outputDir "tray$i.xpm"
        & magick $tempPng -rotate $rotation -colors 256 $xpmPath 2>$null
        if (Test-Path $xpmPath) {
            # Fix variable name for C++ compilation
            Fix-XpmVariableName -xpmPath $xpmPath -expectedName "tray$i"
        } else {
            $success = $false
        }
    }

    Remove-Item $tempPng -ErrorAction SilentlyContinue
    if ($success) {
        Write-Host "OK" -ForegroundColor Green
    } else {
        Write-Host "PARTIAL" -ForegroundColor Yellow
    }
} else {
    Write-Host "  tray_sync.svg not found" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Creating application ICO (multi-resolution)..." -ForegroundColor Cyan

# Main application icon (multi-resolution ICO)
# Use the largest available SVG for best quality
$trayBase48 = Join-Path $scriptDir "tray_base_48.svg"
$trayBase32 = Join-Path $scriptDir "tray_base_32.svg"
$trayBase = if (Test-Path $trayBase48) { $trayBase48 } elseif (Test-Path $trayBase32) { $trayBase32 } else { $trayBase16 }

if ($trayBase -and (Test-Path $trayBase)) {
    Write-Host "  Creating winsplit.ico (16, 32, 48, 256)... " -NoNewline

    $tempLarge = Join-Path $scriptDir "ico_temp_large.png"
    $ico16 = Join-Path $scriptDir "ico_16.png"
    $ico32 = Join-Path $scriptDir "ico_32.png"
    $ico48 = Join-Path $scriptDir "ico_48.png"
    $ico256 = Join-Path $scriptDir "ico_256.png"
    $icoPath = Join-Path $outputDir "winsplit.ico"

    # Two-step: render SVG at high density first, then resize to each target
    & magick $trayBase -background none -density 300 $tempLarge 2>$null
    & magick $tempLarge -resize 16x16 $ico16 2>$null
    & magick $tempLarge -resize 32x32 $ico32 2>$null
    & magick $tempLarge -resize 48x48 $ico48 2>$null
    & magick $tempLarge -resize 256x256 $ico256 2>$null

    & magick $ico16 $ico32 $ico48 $ico256 $icoPath 2>$null

    Remove-Item $tempLarge, $ico16, $ico32, $ico48, $ico256 -ErrorAction SilentlyContinue

    if ($LASTEXITCODE -eq 0 -and (Test-Path $icoPath)) {
        Write-Host "OK" -ForegroundColor Green
    } else {
        Write-Host "FAILED" -ForegroundColor Red
    }
} else {
    Write-Host "  No tray_base SVG found" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Conversion complete!" -ForegroundColor Green
Write-Host ""
Write-Host "Generated files in: $outputDir" -ForegroundColor Cyan
Write-Host "  - Menu icons: *.png (20x20)"
Write-Host "  - Tray icons: icone.xpm, tray2-6.xpm (16x16)"
Write-Host "  - App icon: winsplit.ico (16, 32, 48, 256)"
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Review the generated icons visually"
Write-Host "  2. Rebuild WinSplit Revolution to use new icons"
Write-Host "  3. XPM files may need manual color adjustment for transparency"
Write-Host ""
