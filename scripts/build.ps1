# Builds the K10 firmware on Windows with QMK MSYS.
#
# Copies this repo's keyboards\keychron\k10 into a SonixQMK firmware checkout, builds the
# targets and collects the .bin files (plus the VIA definition) in .\dist.
#
#   .\scripts\build.ps1 -FirmwareDir D:\qmk\sonix_qmk_firmware -MsysDir D:\QMK_MSYS
#
# The firmware checkout must sit on a path WITHOUT spaces (QMK's build breaks on them) and
# be SonixQMK/qmk_firmware, branch sn32_develop, with submodules. Tested at the commit in
# README.md. This repo itself can live anywhere.
param(
    [string]$FirmwareDir = 'D:\qmk\sonix_qmk_firmware',
    [string]$MsysDir = 'D:\QMK_MSYS',
    [string[]]$Targets = @('keychron/k10/rgb/iso:default', 'keychron/k10/rgb/iso:via', 'keychron/k10/probe:default')
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$src = Join-Path $repo 'keyboards\keychron\k10'
$dst = Join-Path $FirmwareDir 'keyboards\keychron\k10'
$bash = Join-Path $MsysDir 'usr\bin\bash.exe'
$dist = Join-Path $repo 'dist'

if ($FirmwareDir -match '\s') { throw "FirmwareDir must not contain spaces: $FirmwareDir" }
if (-not (Test-Path (Join-Path $FirmwareDir 'builddefs'))) { throw "Not a QMK firmware checkout: $FirmwareDir" }
if (-not (Test-Path $bash)) { throw "QMK MSYS not found at $MsysDir" }

# Mirror the keyboard folder into the firmware tree (the copy there is disposable).
robocopy $src $dst /MIR /NJH /NJS /NDL /NFL /NP | Out-Null
if ($LASTEXITCODE -ge 8) { throw "robocopy failed with exit code $LASTEXITCODE" }

$env:MSYSTEM = 'MINGW64'
$env:CHERE_INVOKING = '1'
$env:MSYS2_PATH_TYPE = 'minimal'
$fwPosix = '/' + $FirmwareDir.Substring(0, 1).ToLower() + ($FirmwareDir.Substring(2) -replace '\\', '/')

New-Item -ItemType Directory -Force $dist | Out-Null
foreach ($target in $Targets) {
    "Building $target"
    $out = & $bash -l -c "cd $fwPosix && make -j8 $target 2>&1"
    if ($LASTEXITCODE -ne 0) {
        $out | Select-Object -Last 25
        throw "Build failed: $target"
    }
    $name = ($target -replace '[/:]', '_') + '.bin'
    Copy-Item (Join-Path $FirmwareDir $name) $dist -Force
    "  -> dist\$name"
}
Copy-Item (Join-Path $repo 'keyboards\keychron\k10\rgb\via_json\k10_iso.json') $dist -Force
Get-ChildItem $dist | Select-Object Name, Length, @{ n = 'SHA256'; e = { (Get-FileHash $_.FullName -Algorithm SHA256).Hash.ToLower() } } | Format-Table -AutoSize
