#Requires -Version 7.0
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
& (Join-Path $repoRoot 'scripts/Generate-EffectCatalog.ps1') -Check
$catalog = Get-Content -LiteralPath (Join-Path $repoRoot 'src/Magpie/EffectCatalog/zh-Hans.json') -Raw | ConvertFrom-Json
$effectRoot = Join-Path $repoRoot 'src/Effects'
$ids = @(Get-ChildItem -LiteralPath $effectRoot -Filter '*.hlsl' -Recurse -File | ForEach-Object { [IO.Path]::GetRelativePath($effectRoot, $_.FullName).Replace('.hlsl', '') })
if (@(Compare-Object $ids @($catalog.effects.id)).Count -or @($catalog.effects.id | Group-Object | Where-Object Count -gt 1).Count) { throw 'Catalog and installed IDs differ or contain duplicates.' }
foreach ($entry in $catalog.effects) {
    foreach ($field in @('name','summary','details','category','search')) { if (!$entry.$field) { throw "Missing $field for $($entry.id)" } }
    if ($entry.category -notin $catalog.categories.id) { throw 'Unknown category' }
}
$dlssnr = $catalog.effects | Where-Object id -eq 'DLSSNR\DLSSNR_AI_Filter'
if ($dlssnr.category -ne 'style' -or $dlssnr.purposes -contains 'cleanup' -or $dlssnr.summary -match '降噪') { throw 'DLSSNR purpose regression' }
$dlss = $catalog.effects | Where-Object id -eq 'DLSS\DLSS_SR'
if ($dlss.category -ne 'antialiasing' -or $dlss.details -notmatch 'J' -or $dlss.details -notmatch 'L／M') { throw 'DLSS SR classification regression' }
$rtx = @($catalog.effects | Where-Object id -like 'RTXVideo\*')
if ($rtx.Count -ne 8 -or @($rtx.name | Sort-Object -Unique).Count -ne 2) { throw 'RTX Video grouping regression' }
foreach ($entry in $rtx) {
    $file = Join-Path $effectRoot ($entry.id + '.hlsl')
    if ((Get-Content -LiteralPath $file -Raw) -match '//!PARAMETER') { throw 'RTX tier parameters changed: review parameter view model compatibility.' }
}
if (@($catalog.effects | Where-Object { $_.id -like 'XeSSFG\*' -and $_.name -like '*ZeroMV*' }).Count) { throw 'XeSS display alias regression' }
"Catalog validated: $($ids.Count) source effects, 155 built-in picker entries; all eight RTX IDs retained."
