Magpie Experimental v0.6.5 x64

安装与升级
- 完全退出 Magpie，将完整主包解压到新目录后运行 Magpie.exe；不要只替换 EXE 或复制旧效果目录。
- 本版移除深度估算与旧 SR 实验入口，不再需要旧 DirectML/TensorRT 深度组件。旧配置自动迁移，升级前请备份。
- 普通配置：%LOCALAPPDATA%\Magpie\config\v4\config.json。便携配置：程序目录内 config\config.json。
- 默认组为 Lanczos、FSR、RTX Video VSR Ultra、DLSSFG、XeSSFG、DLSSNR，默认选择 Lanczos；已有自定义组保留。

主要变化
- 缩放模式改称效果组；参数支持分组、中文和下拉选项，双击滑条（170 ms）恢复效果器自身默认值。
- 参数自动保存，保留有效备份。Live 实时生效，Restart 等待手动应用；DLSSNR 核心/上游原实时参数会自动停用效果组，等待 500 ms 再重新启用。
- 工具栏支持性能监测、参数、截图、固定、对比快捷键（Alt+Shift+P/E/S/F/C）。对比期间继续计算，角标停留 2 秒后 500 ms 淡出。
- Front Edge Sync 默认开启、60 FPS；FG 时控制基础帧输入，可能增加延迟。目标程序需配合限帧，FrameRate Filter 默认跟随。VRR 暂时停用，HDR 未接入。
- SR/NR/FG 共享光流请求；NVOF 优先，再 AMD OF，采用实际请求中的较高档位。高质量档位会增加 GPU 开销。
- 修复捕获恢复、参数保存和分析器/FG 呈现的已定位问题，不保证解决所有机器的卡顿。

完整参数、迁移、工具用途与排错步骤请阅读 RELEASE-NOTES.md；帧同步见 FRAME_SYNC_GUIDE.md。
发生问题时查看主页“最近一次问题”的详情与日志。CPU/编译/包体检查不等于目标 GPU 画质和性能验收。
保留 LICENSE-Magpie.txt、THIRD-PARTY-NOTICES.md、各组件许可证与 build-manifest.json。

English
- Fully exit Magpie, extract the complete package into a new directory, then run Magpie.exe. Do not replace only the EXE or copy old effects/depth components.
- Depth estimation and legacy SR variants are removed; DirectML/TensorRT depth packages are no longer needed. Old settings migrate; back them up first.
- Normal settings: %LOCALAPPDATA%\Magpie\config\v4\config.json. Portable settings: config\config.json beside Magpie.exe.
- Default groups: Lanczos, FSR, RTX Video VSR Ultra, DLSSFG, XeSSFG, DLSSNR. Lanczos is selected; custom groups are preserved.
- Scaling modes are now Effect groups. Parameters support groups, Chinese display and choices; double-click within 170 ms to restore the effect's own default.
- Edits save automatically with valid backups. Live applies immediately; Restart needs manual application. Previously live DLSSNR core/upstream changes stop the group and restart after 500 ms.
- Toolbar defaults: Alt+Shift+P/E/S/F/C for profiler/parameters/screenshot/pin/comparison. Comparison keeps effects running; its badge stays two seconds and fades over 500 ms.
- Front Edge Sync defaults to enabled at 60 FPS, pacing base input with FG. Latency may increase; limit the source application too. FrameRate Filter follows by default. VRR is disabled and HDR is not integrated.
- Consumers share NVOF first, then AMD OF, at the highest actually requested quality. Higher quality costs GPU time.
- Identified capture-recovery, saving and profiler/FG presentation paths are fixed; this is not a guarantee against all stalls.
- Read RELEASE-NOTES.md for full parameters, migration, tools and troubleshooting; FRAME_SYNC_GUIDE.md explains pacing.
- Use Home's recent-issue details and logs. Build/CPU/package checks do not replace visual and performance acceptance on target hardware.
- Retain LICENSE-Magpie.txt, THIRD-PARTY-NOTICES.md, vendor licenses and build-manifest.json.
