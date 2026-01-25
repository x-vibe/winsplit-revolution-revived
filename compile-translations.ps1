$msgfmt = "E:\VIBECODING\DEV\OTHER\winsplit-revolution-revamped\gettext-tools\bin\msgfmt.exe"
$langDir = Join-Path $PSScriptRoot "upstream\bin\languages"

Get-ChildItem -Path $langDir -Directory | ForEach-Object {
    $poFile = Join-Path $_.FullName "winsplit.po"
    $moFile = Join-Path $_.FullName "winsplit.mo"
    if (Test-Path $poFile) {
        Write-Host "Compiling: $($_.Name)"
        & $msgfmt -o $moFile $poFile
    }
}
Write-Host "All translations compiled!"
