$ProgressPreference = 'SilentlyContinue'
# Updated URL - using direct from Mlocati releases page
$url = "https://github.com/mlocati/gettext-iconv-windows/releases/download/v0.21-v1.16/gettext0.21-iconv1.16-static-64.zip"
$outDir = $PSScriptRoot
$zipFile = Join-Path $outDir "gettext-tools.zip"
$extractDir = Join-Path $outDir "gettext-tools"

# Remove old broken file
if (Test-Path $zipFile) { Remove-Item $zipFile -Force }
if (Test-Path $extractDir) { Remove-Item $extractDir -Recurse -Force }

Write-Host "Downloading gettext tools..."
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

try {
    Invoke-WebRequest -Uri $url -OutFile $zipFile -UseBasicParsing
} catch {
    # Try alternate URL
    $url2 = "https://github.com/vslavik/gettext-tools-windows/releases/download/v0.21/gettext-tools-windows-0.21.zip"
    Write-Host "Primary URL failed, trying alternate..."
    Invoke-WebRequest -Uri $url2 -OutFile $zipFile -UseBasicParsing
}

Write-Host "Extracting to $extractDir..."
Expand-Archive -Path $zipFile -DestinationPath $extractDir -Force

# Find msgfmt.exe location
$msgfmt = Get-ChildItem -Path $extractDir -Recurse -Filter "msgfmt.exe" | Select-Object -First 1
if ($msgfmt) {
    Write-Host "Done! msgfmt.exe found at: $($msgfmt.FullName)"
    # Update compile script with correct path
    $binDir = $msgfmt.DirectoryName
    $compileScript = @"
`$msgfmt = "$binDir\msgfmt.exe"
`$langDir = Join-Path `$PSScriptRoot "upstream\bin\languages"

Get-ChildItem -Path `$langDir -Directory | ForEach-Object {
    `$poFile = Join-Path `$_.FullName "winsplit.po"
    `$moFile = Join-Path `$_.FullName "winsplit.mo"
    if (Test-Path `$poFile) {
        Write-Host "Compiling: `$(`$_.Name)"
        & `$msgfmt -o `$moFile `$poFile
    }
}
Write-Host "All translations compiled!"
"@
    Set-Content -Path (Join-Path $outDir "compile-translations.ps1") -Value $compileScript
    Write-Host "Updated compile-translations.ps1 with correct path"
} else {
    Write-Host "Warning: msgfmt.exe not found in extracted files"
    Write-Host "Contents of $extractDir :"
    Get-ChildItem -Path $extractDir -Recurse | Select-Object FullName
}
