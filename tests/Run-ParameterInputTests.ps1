#Requires -Version 7.0
param([switch]$NativePrototype)
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$workspace = Split-Path $repo -Parent
$output = Join-Path $workspace '.tools/067-beta4/parameter-input'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vs = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath | Select-Object -First 1
Import-Module (Join-Path $vs 'Common7/Tools/Microsoft.VisualStudio.DevShell.dll')
Enter-VsDevShell -VsInstallPath $vs -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64' | Out-Null
[xml]$props = Get-Content -LiteralPath (Join-Path $workspace 'release/v0.6.7-local/obj/_ConanDeps/Magpie/conan_imgui_vars_release_x64.props')
$imgui = $props.Project.PropertyGroup.ConanimguiRootFolder
New-Item -ItemType Directory -Force $output | Out-Null
foreach ($test in @('parameter_input', 'overlay_window_layout')) {
    & python (Join-Path $repo "scripts/tests/test_$test.py") $output
    if ($LASTEXITCODE) { throw "Extract $test failed" }
    & cl.exe /nologo /std:c++20 /EHsc /utf-8 /MT /O2 /DNOMINMAX "/I$repo/src/Magpie.Core/include" "/I$imgui/include" "$output/$test.cpp" "$imgui/lib/imgui.lib" user32.lib imm32.lib "/Fe:$output/$test.exe" "/Fo:$output/$test.obj"
    if ($LASTEXITCODE) { throw "Compile $test failed" }
    & "$output/$test.exe"
    if ($LASTEXITCODE) { throw "$test failed" }
}
if ($NativePrototype) {
    # Creates two small test-owned windows, tests real input routing and restores
    # the previous cursor/focus. No game or Magpie configuration is modified.
    & cl.exe /nologo /std:c++20 /EHsc /utf-8 /MT /O2 "$repo/scripts/tests/parameter_input_host_prototype.cpp" "/Fe:$output/prototype.exe" "/Fo:$output/prototype.obj"
    if ($LASTEXITCODE) { throw 'Compile native input prototype failed' }
    & "$output/prototype.exe"
    if ($LASTEXITCODE) { throw 'Native input prototype failed' }
}
