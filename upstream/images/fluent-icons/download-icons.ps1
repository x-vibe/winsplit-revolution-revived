# Download Fluent UI System Icons for WinSplit Revolution
# Source: https://github.com/microsoft/fluentui-system-icons (MIT License)
#
# Run this script from PowerShell to download the required Fluent icons.
# After downloading, run convert-icons.ps1 to generate PNG/XPM/ICO files.

$ErrorActionPreference = "Stop"

$baseUrl = "https://raw.githubusercontent.com/microsoft/fluentui-system-icons/main/assets"
$outputDir = $PSScriptRoot

Write-Host ""
Write-Host "=== Fluent UI System Icons Downloader ===" -ForegroundColor Cyan
Write-Host "License: MIT (free for commercial use)" -ForegroundColor Gray
Write-Host ""

# Icon definitions: [folder_name, file_name, output_name, description]
$icons = @(
    # Menu icons (20px)
    @("Settings", "ic_fluent_settings_20_regular.svg", "options.svg", "Options/Settings menu"),
    @("Keyboard", "ic_fluent_keyboard_20_regular.svg", "hotkeys.svg", "Hotkey settings"),
    @("Question%20Circle", "ic_fluent_question_circle_20_regular.svg", "help.svg", "Help menu"),
    @("Info", "ic_fluent_info_20_regular.svg", "about.svg", "About dialog"),
    @("Power", "ic_fluent_power_20_regular.svg", "exit.svg", "Exit application"),
    @("Checkmark%20Circle", "ic_fluent_checkmark_circle_20_filled.svg", "auto_start_true.svg", "Auto-start enabled"),
    @("Circle", "ic_fluent_circle_20_regular.svg", "auto_start_false.svg", "Auto-start disabled"),
    @("Grid", "ic_fluent_grid_20_regular.svg", "layout.svg", "Layout settings"),

    # Tray/App icons (multiple sizes for ICO)
    @("Apps", "ic_fluent_apps_16_regular.svg", "tray_base_16.svg", "Tray icon base (16px)"),
    @("Apps", "ic_fluent_apps_20_regular.svg", "tray_base_20.svg", "Tray icon base (20px)"),
    @("Apps", "ic_fluent_apps_24_regular.svg", "tray_base_24.svg", "Tray icon base (24px)"),
    @("Apps", "ic_fluent_apps_32_regular.svg", "tray_base_32.svg", "App icon (32px)"),
    @("Apps", "ic_fluent_apps_48_regular.svg", "tray_base_48.svg", "App icon (48px)"),

    # Sync animation icon
    @("Arrow%20Sync", "ic_fluent_arrow_sync_16_regular.svg", "tray_sync.svg", "Update check animation")
)

$successCount = 0
$failCount = 0

foreach ($icon in $icons) {
    $folder = $icon[0]
    $filename = $icon[1]
    $output = $icon[2]
    $desc = $icon[3]

    $url = "$baseUrl/$folder/SVG/$filename"
    $outPath = Join-Path $outputDir $output

    Write-Host "  $desc... " -NoNewline

    try {
        $response = Invoke-WebRequest -Uri $url -OutFile $outPath -UseBasicParsing -PassThru
        Write-Host "OK" -ForegroundColor Green
        $successCount++
    }
    catch {
        Write-Host "FAILED" -ForegroundColor Red
        Write-Host "    URL: $url" -ForegroundColor DarkGray
        $failCount++
    }
}

Write-Host ""
Write-Host "Download complete: $successCount succeeded, $failCount failed" -ForegroundColor $(if ($failCount -eq 0) { "Green" } else { "Yellow" })
Write-Host "Files saved to: $outputDir" -ForegroundColor Cyan
Write-Host ""

if ($successCount -gt 0) {
    Write-Host "Next steps:" -ForegroundColor Yellow
    Write-Host "1. Install ImageMagick if not already installed:"
    Write-Host "   https://imagemagick.org/script/download.php#windows" -ForegroundColor DarkCyan
    Write-Host ""
    Write-Host "2. Run the conversion script:"
    Write-Host "   .\convert-icons.ps1" -ForegroundColor DarkCyan
    Write-Host ""
}
