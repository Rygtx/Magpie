<p align="center"><img src="./src/Magpie/Icons/SVG/Magpie Icon Full Disabled.svg" width="150" height="150" alt="Magpie"></p>
<h1 align="center">Magpie Experimental</h1>

🌍 **English** | [简体中文](./README_ZH.md)

An unofficial experimental fork of [Blinue/Magpie](https://github.com/Blinue/Magpie), providing captured-window DLSS, DLSS Frame Generation, DLSSNR, XeSS, FSR and RTX Video effects. This is not an official Magpie release and is not supported by upstream.

## Versions and Installation

0.6.5 is in local release-package preparation. See the complete [0.6.5 Release Note](docs/RELEASE_NOTES_v0.6.5-experimental.md); publicly available builds are listed on [GitHub Releases](https://github.com/SAOG0721/Magpie/releases).

Fully exit Magpie, extract all of `Magpie-Experimental-x64.zip` into a new directory, and run `Magpie.exe`. Do not replace only the EXE or copy old effects/depth components into the new package. Normal settings use `%LOCALAPPDATA%\Magpie\config\v4\config.json`; portable settings use `config\config.json` beside the program. Back up your configuration and previous installation before upgrading.

## Effect Groups and Parameters

Scaling modes are now called **Effect groups**, combining multiple effects. Fresh configurations contain Lanczos, FSR, RTX Video VSR Ultra, DLSSFG, XeSSFG and DLSSNR, with Lanczos selected. Existing custom groups are preserved.

DLSSFG, XeSSFG and DLSSNR are included among the built-in effect groups. Reset on the Effect groups page restores the defaults.

Parameter edits save automatically. The toolbar editor labels controls **Live**, **Restart**, or **Auto restart**, distinguishing manual application from automatic rebuilding after editing. Previously live DLSSNR core/upstream image changes fully stop the group, wait 500 ms and restart it; residual composition retains its existing live behavior. Consecutive edits coalesce, and manually stopping cancels a pending restart.

Double-click a slider within 170 ms to restore the effect's own default. Parameters support groups, drop-down choices, and Simplified/Traditional Chinese display. The editor distinguishes desired values from applied values when saving fails or a change has not taken effect.

## Toolbar and Frame Pacing

| Action | Default Shortcut |
| --- | --- |
| Profiler | Alt+Shift+P |
| Effect parameters | Alt+Shift+E |
| Screenshot | Alt+Shift+S |
| Pin toolbar | Alt+Shift+F |
| Comparison | Alt+Shift+C |

Configure shortcuts in Home's collapsible toolbar section. Comparison continues effect processing while showing the original; its badge stays for two seconds and fades over 500 ms. With FG, frame rates support an output/real-frame display.

Front Edge Sync defaults to enabled at 60 FPS and controls base-frame pacing before FG. Apply a matching limiter in the source application too; latency may increase. FrameRate Filter follows this setting by default and offers Custom only with synchronization disabled. See the [frame-sync guide](docs/FRAME_SYNC_GUIDE.md). VRR is currently hidden and disabled; HDR remains unimplemented.

## Optical Flow and Compatibility

DLSS SR, FSR 2/3/4 and XeSS SR each have one entry with None / AMD OF / NVOF. Standalone Zero MV, Optical Flow and metadata-only jitter variants are merged, with automatic migration of old settings. DLSSNR and DLSSFG use NVOF; XeSSFG x2 offers AMD OF/NVOF, while XeSS Multi-FG currently supports AMD OF or None.

Consumers share a requested provider: NVOF first, then AMD OF, at the highest quality actually requested for that source. NVOF Highest Quality is 2×2 Slow and can be very expensive; Balanced remains marked Recommended.

Magpie lacks engine-native depth, motion vectors, exposure and UI separation. Depth contracts use zero-filled textures and motion is estimated from captured color, so this is not equivalent to native DLSS/FSR/XeSS integration. Use only one FG per group and avoid stacking it with Smooth Motion or other frame-generation systems.

## Troubleshooting and Development

Start with Home's recent-issue details and logs. Release tools include the NGX OTA switch and DLSSNR DLL choices; see the Release Note for usage and limitations. Optional proprietary backends default to disabled in source builds; local SDK/runtime paths belong in the untracked `src/BuildOptions.props.user`.

- [Dependencies, notices and build boundaries](docs/THIRD_PARTY_AND_REDISTRIBUTION.md)
- [Experimental development documentation](docs/experimental/README.md)
- [Build and packaging script](scripts/Build-Release.ps1)
- [MagpieFX format](<docs/MagpieFX (EN).md>)

Source retains [GPLv3](LICENSE). Third-party runtimes, SDKs, models and local configuration are excluded from the source repository.
