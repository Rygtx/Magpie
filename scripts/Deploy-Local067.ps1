#Requires -Version 7.0
param([string]$BuildEnvironmentProps)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$sourceRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$workspaceRoot = Split-Path $sourceRoot -Parent
$releaseRoot = Join-Path $workspaceRoot 'release'
$container = Join-Path $releaseRoot 'v0.6.7-local'
$destination = Join-Path $container 'Magpie-Experimental-x64'
$buildOutput = [IO.Path]::GetFullPath((Join-Path $releaseRoot '.build/v0.6.7-local'))
$version = '0.6.7-local'

function Assert-MagpieClosed {
    if (Get-Process -Name Magpie -ErrorAction SilentlyContinue) {
        throw 'Exit Magpie from its tray menu, then rerun this deployment.'
    }
}

Assert-MagpieClosed
$commit = (& git -C $sourceRoot rev-parse HEAD).Trim()
if ($LASTEXITCODE) { throw 'Read the source commit failed.' }
$dirty = @(& git -C $sourceRoot status --porcelain)
if ($LASTEXITCODE -or $dirty.Count) { throw 'Commit the reviewed source before deployment.' }
$shortCommit = $commit.Substring(0, 12)
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vs = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath |
    Select-Object -First 1
if (!$vs) { throw 'Install Visual Studio with the C++ desktop workload.' }
$msbuild = Join-Path $vs 'MSBuild/Current/Bin/amd64/MSBuild.exe'
$env:Path = (Join-Path $vs 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin') + ';' + $env:Path
if ($BuildEnvironmentProps) { $env:ForceImportBeforeCppTargets = (Resolve-Path -LiteralPath $BuildEnvironmentProps).Path }
$env:MSBUILDDISABLENODEREUSE = '1'

# Only this disposable output is cleared. The installed config/logs stay in place.
$allowedBuildRoot = [IO.Path]::GetFullPath((Join-Path $releaseRoot '.build')) + [IO.Path]::DirectorySeparatorChar
if (!$buildOutput.StartsWith($allowedBuildRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'The build output must remain inside release/.build.'
}
if (Test-Path -LiteralPath $buildOutput) { Remove-Item -LiteralPath $buildOutput -Recurse -Force }
New-Item -ItemType Directory -Path $buildOutput, $container -Force | Out-Null
$outputArg = $buildOutput.Replace('\', '/') + '/'
$logPath = Join-Path $container 'build.log'
$buildArgs = @(
    'Magpie.slnx', '/m:2', '/nr:false', '/v:minimal', '/t:Build',
    '/p:Configuration=Release', '/p:Platform=x64',
    '/p:MajorVersion=0', '/p:MinorVersion=6', '/p:PatchVersion=7',
    "/p:VersionString=$version", "/p:CommitId=$shortCommit",
    '/p:PreferredToolArchitecture=x64', '/p:UseMultiToolTask=true',
    '/p:CL_MPCount=4', '/p:MultiProcMaxCount=4', '/p:EnforceProcessCountAcrossBuilds=true',
    '/p:DisablePDB=false', '/p:ReproducibleBuild=true', '/p:EnableFrameTrace=false',
    "/p:OutDir=$outputArg"
)
Push-Location $sourceRoot
try {
    & $msbuild @buildArgs *> $logPath
    if ($LASTEXITCODE) { throw "Build failed; inspect $logPath" }
} finally { Pop-Location }

foreach ($required in @('Magpie.exe', 'Magpie.pdb', 'resources.pri', 'Microsoft.UI.Xaml.dll', 'TouchHelper.exe', 'Updater.exe', 'effects')) {
    if (!(Test-Path -LiteralPath (Join-Path $buildOutput $required))) { throw "Incomplete build: $required" }
}
if ((Get-Item -LiteralPath (Join-Path $buildOutput 'Magpie.exe')).VersionInfo.FileVersion -ne $version) {
    throw 'The executable version does not match this deployment.'
}
if ((& git -C $sourceRoot rev-parse HEAD).Trim() -ne $commit -or
    @(& git -C $sourceRoot status --porcelain).Count) {
    throw 'Source changed during the build; rerun against the reviewed commit.'
}

Copy-Item -LiteralPath (Join-Path $sourceRoot 'LICENSE') -Destination (Join-Path $buildOutput 'LICENSE-Magpie.txt')
Copy-Item -LiteralPath (Join-Path $sourceRoot 'docs/experimental/testing/SCALING-PREFLIGHT.md') -Destination (Join-Path $buildOutput 'LOCAL-NOTES.md')
$files = @(Get-ChildItem -LiteralPath $buildOutput -File -Recurse | Where-Object {
    $_.Extension -notin @('.lib', '.exp', '.obj', '.ilk')
})
$records = @($files | ForEach-Object {
    [ordered]@{
        path = [IO.Path]::GetRelativePath($buildOutput, $_.FullName).Replace('\', '/')
        bytes = $_.Length
        sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash
    }
})
Assert-MagpieClosed
New-Item -ItemType Directory -Path $destination -Force | Out-Null
foreach ($record in $records) {
    $target = Join-Path $destination $record.path
    New-Item -ItemType Directory -Path (Split-Path $target -Parent) -Force | Out-Null
    Copy-Item -LiteralPath (Join-Path $buildOutput $record.path) -Destination $target -Force
    if ((Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash -ne $record.sha256) {
        throw "Deployed file verification failed: $($record.path)"
    }
}
# No directory-wide deletion or mirroring: retain local config, logs and diagnostics.
[ordered]@{
    schemaVersion = 1; version = $version; commit = $commit; sourceDirty = $false
    configuration = 'Release'; platform = 'x64'; deployedAtUtc = [DateTime]::UtcNow.ToString('o')
    files = $records
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $destination 'build-manifest.json') -Encoding utf8
Write-Output "Deployed $version ($shortCommit): $destination"
