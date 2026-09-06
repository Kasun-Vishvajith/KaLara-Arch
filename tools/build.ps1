param([ValidateSet('windows-debug','windows-release','core-tests')][string]$Preset = 'windows-debug')
$ErrorActionPreference = 'Stop'
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$installation = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$installation) { throw 'MSVC x64 Build Tools missing' }
$vcvars = Join-Path $installation 'VC\Auxiliary\Build\vcvars64.bat'
$environmentLines = & cmd /c "`"$vcvars`" >nul && set"
$seenEnvironment = @{}
foreach ($line in $environmentLines) {
    if ($line -match '^([^=]+)=(.*)$' -and !$seenEnvironment.ContainsKey($matches[1])) { $seenEnvironment[$matches[1]] = $true; [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process') }
}
$ninjaDir = Join-Path $installation 'Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja'
$env:PATH = "$ninjaDir;$env:PATH"
if (!$env:QTDIR) {
    $qtCandidate = 'C:\Qt\6.7.2\msvc2019_64'
    if (Test-Path "$qtCandidate\lib\cmake\Qt6\Qt6Config.cmake") { $env:QTDIR = $qtCandidate }
}
if ($env:QTDIR) { $env:PATH = "$env:QTDIR\bin;$env:PATH" }
cmake --preset $Preset
if ($LASTEXITCODE) { exit $LASTEXITCODE }
cmake --build --preset $Preset
if ($LASTEXITCODE) { exit $LASTEXITCODE }
ctest --preset $Preset
exit $LASTEXITCODE



