# Magpie Experimental v0.6.5

## Magpie 本体更新

- **效果组管理**：“缩放模式”更名为“效果组”，用于组合多个效果器；默认提供 Lanczos、FSR、RTX Video VSR Ultra、DLSSFG、XeSSFG 和 DLSSNR。已有自定义效果组会保留。
- **参数调节**：可在工具栏中边看画面边调节参数，修改自动保存，并提供中文翻译和分组显示。在 170 毫秒内双击滑条，可恢复该效果器的默认值。
- **工具栏与对比**：性能监测、效果参数、截屏、固定工具栏和对比均可设置快捷键。一键切换原图与处理后画面，效果持续运行，切回无需重新加载；状态角标会自动淡出。
- **Front Edge Sync**：默认开启并限制为 60 FPS，用于稳定画面节奏；启用补帧时控制补帧前的真实帧率。目标程序需要配合限帧，开启后可能增加延迟。
- **性能监测**：可查看各效果器的处理耗时，方便判断哪些效果占用较多性能。启用 FG 后，以 `120/60` 这样的形式显示“输出帧率／真实帧率”。
- **[暂缓] VRR 与 HDR**：本版暂不提供 VRR 开关和 HDR 功能。

## 效果器更新

- **DLSSNR 画面调整**：新增残差饱和度、亮度、阴影／结构、反射／辉光控制，并改善降低输入分辨率后的细节表现。残差参数可实时调整，核心参数或前置效果参数改变后会自动重新启用效果组。
- **超分辨率效果**：DLSS SR、FSR 2/3/4、XeSS SR 各保留一个入口，在参数中选择光流方法即可。**旧 Zero MV、jitter、Optical Flow 独立版本及内置光流已移除**，旧配置会自动迁移。
- **光流质量与共享**：SR、NR、FG 可选择各自支持的 AMD OF／NVOF，并共用光流估算以减少重复开销。NVOF 新增“最高质量（极高开销）”，不同效果器设置不一致时会提示实际采用的方法。
- **帧率过滤器**：FrameRate Filter 默认跟随 Front Edge Sync，开启同步时无需单独设置。关闭同步后可选择“自定义”，使用滑条调整帧率。
- **[已移除] 深度估算**：移除深度估算效果及配套 DirectML／TensorRT 组件，旧深度选项会自动清理。

## 错误与稳定性修复

- **参数与配置**：修复修改参数后闪退、配置损坏导致无法启动，以及控件值与实际效果不一致的问题。同步修正部分参数范围、默认值和滑条步进。
- **全屏切屏**：使用 Alt+Tab、Alt+Shift+Tab 或 Win+Tab 时自动停用全屏效果组，返回后需手动再次启用，以避开切屏花屏问题。窗口效果组不受此规则影响。
- **浮窗与补帧**：修复拖动性能面板可能导致 DLSSFG 异常、各效果器耗时不显示，以及下拉框点击外部不收起等问题。改善低帧率和静止画面下的参数窗口操作。
- **故障提示**：报错增加原因说明和处理建议；主页“最近一次问题”可查看详情、复制诊断信息、打开日志目录，也可手动关闭。

## 使用说明

### 安装或从旧版升级

**建议删除此前所有版本的 Magpie 程序目录（包括旧内测版），再安装本版；不要直接覆盖旧目录。**

1. 如需保留设置或截图，先备份到程序目录之外。
2. 从托盘完全退出 Magpie，删除所有旧版程序目录。
3. 将 `Magpie-Experimental-x64.zip` 完整解压到新目录，运行其中的 `Magpie.exe`。

普通配置位于 `%LOCALAPPDATA%\Magpie\config\v4\config.json`；便携配置位于原程序目录的 `config\config.json`。需要沿用设置时可保留或恢复配置，**不要复制旧版效果目录、DLL 或深度组件到新目录**。

### 参数与帧率设置

- **参数生效方式**：Live／实时立即生效；Restart／重启需点击“应用并重新启用”；Auto restart／自动重启会在编辑结束后重新启用效果组。
- **DLSSNR 残差控制**：先开启“调整输入分辨率”，即使比例为 100% 也能使用残差调整。降低比例可减轻性能压力，但会损失部分画面信息。
- **Front Edge Sync**：手动目标表示补帧前帧率，`0` 表示按显示器刷新率自动设置，有 FG 时按倍率折算。例如 80 FPS 配合 2 倍补帧，名义输出为 160 FPS；请同步限制目标程序帧率。
- **补帧搭配**：一个效果组只使用一种 FG；出现延迟或性能压力时，可降低光流质量，或关闭 Front Edge Sync 比较表现。

### 默认工具栏快捷键

| 功能 | 快捷键 |
| --- | --- |
| 性能监测 | Alt+Shift+P |
| 效果参数 | Alt+Shift+E |
| 截屏 | Alt+Shift+S |
| 固定工具栏 | Alt+Shift+F |
| 对比 | Alt+Shift+C |

可在主页“工具栏快捷键设置”中修改。持续异常可在主页查看问题详情和日志；更多帧率设置说明见包内 `FRAME_SYNC_GUIDE.md`。

## 附件的作用与使用

| 附件 | 用途与使用方法 |
| --- | --- |
| `Magpie-Experimental-x64.zip` | **必选主包**，包含程序和所需运行组件；按上面的安装步骤完整解压使用。其余附件均为可选。 |
| `ScalingModes-v0.6.5-experimental.json` | 在“效果组”页面导入，追加 DLSSFG、XeSSFG、DLSSNR 三组预设，不替换已有组。 |
| `DLSSNR-DLL-Options-310.8.0.0.zip` | 提供 NVIDIA 官方版和 RTX 40/50 社区兼容版 DLL 选项；仅需切换版本时下载。完全退出 Magpie 并备份现有 `nvngx_dlssnr.dll`，再按包内说明选择一个版本放到 `Magpie.exe` 旁。 |
| `NGX_OTA_Switch.bat` | 用于查看、开关 NVIDIA NGX OTA 更新及清理更新进程，常规安装无需运行。相关操作需管理员权限且影响系统级 NGX 设置；恢复时使用菜单的 **Restore default**，删除 BAT 不会撤销设置。 |

---

# Magpie Experimental v0.6.5

## Magpie Application Updates

- **Effect groups**: Scaling modes are renamed to Effect groups for combining multiple effects; defaults include Lanczos, FSR, RTX Video VSR Ultra, DLSSFG, XeSSFG and DLSSNR. Existing custom groups are preserved.
- **Parameter editing**: adjust parameters from the toolbar while viewing the result, with automatic saving, Chinese translations and grouped controls. Double-click a slider within 170 ms to restore the effect's default value.
- **Toolbar and comparison**: assign shortcuts to the profiler, parameters, screenshots, toolbar pinning and comparison. Switch between original and processed images while effects keep running, with no reload when switching back and an automatically fading status badge.
- **Front Edge Sync**: enabled by default at 60 FPS to stabilize frame pacing, controlling real frames before FG when frame generation is active. Apply a matching limiter to the source application; enabling synchronization may increase latency.
- **Performance monitoring**: view each effect's processing time to identify costly effects. With FG, readings such as `120/60` show output FPS / real FPS.
- **[Deferred] VRR and HDR**: this version does not provide the VRR switch or HDR features.

## Effect Updates

- **DLSSNR image controls**: new residual saturation, lightness, shadow/structure and reflection/glow controls accompany improved detail at reduced input resolution. Residual controls work live; changes to core parameters or preceding effects automatically restart the group.
- **Super resolution**: DLSS SR, FSR 2/3/4 and XeSS SR each have a single entry with optical-flow selection in its parameters. **Separate Zero MV, jitter and Optical Flow variants, plus the old built-in flow, are removed**, with automatic migration of old settings.
- **Optical-flow quality and sharing**: SR, NR and FG can select their supported AMD OF / NVOF options and share estimation to reduce duplicate work. NVOF adds Highest Quality (Very High Cost), and differing effect settings produce a notice showing the selected method.
- **FrameRate Filter**: follows Front Edge Sync by default, requiring no separate cap while synchronization is enabled. Disable synchronization to choose Custom and adjust the frame-rate slider.
- **[Removed] Depth estimation**: depth effects and their DirectML/TensorRT components are removed, with automatic cleanup of legacy depth settings.

## Error and Stability Fixes

- **Parameters and configuration**: fixed crashes after parameter edits, damaged configurations preventing startup, and controls disagreeing with applied effects. Some parameter ranges, defaults and slider steps are also corrected.
- **Fullscreen task switching**: Alt+Tab, Alt+Shift+Tab and Win+Tab stop fullscreen effects to avoid task-switching artifacts; enable the group manually after returning. This rule does not affect windowed groups.
- **Overlays and frame generation**: fixed profiler dragging disrupting DLSSFG, missing per-effect timings, and drop-downs staying open after an outside click. Parameter-window interaction is improved at low frame rates and with static images.
- **Helpful errors**: messages include causes and suggested actions; Home's recent-issue card offers details, diagnostic copying, log access and manual dismissal.

## Usage

### Install or Upgrade

**We recommend deleting all previous Magpie program folders, including older beta builds, before installing this version; do not install over an old folder.**

1. If you want to keep settings or screenshots, back them up outside the program folders first.
2. Fully exit Magpie from the system tray and delete all old program folders.
3. Extract `Magpie-Experimental-x64.zip` completely into a new folder and run its `Magpie.exe`.

Normal settings are stored at `%LOCALAPPDATA%\Magpie\config\v4\config.json`; portable settings are in the old program folder's `config\config.json`. Keep or restore the configuration if needed, but **do not copy old effects, DLLs or depth components into the new folder**.

### Parameters and Frame Rates

- **Applying parameters**: Live takes effect immediately; Restart requires Apply and restart; Auto restart re-enables the group after editing finishes.
- **DLSSNR residual controls**: enable Adjust Input Resolution first, even when using 100%. Lowering the percentage reduces processing pressure but loses some image information.
- **Front Edge Sync**: a manual target is the rate before FG; `0` selects a rate from the display refresh rate, divided by the FG multiplier when applicable. For example, 80 FPS with 2× FG nominally outputs 160 FPS; apply a matching source-application limiter.
- **Combining FG**: use only one FG per group; if latency or GPU pressure is high, lower optical-flow quality or compare with Front Edge Sync disabled.

### Default Toolbar Shortcuts

| Action | Shortcut |
| --- | --- |
| Profiler | Alt+Shift+P |
| Effect parameters | Alt+Shift+E |
| Screenshot | Alt+Shift+S |
| Pin toolbar | Alt+Shift+F |
| Comparison | Alt+Shift+C |

Customize these in Home's Toolbar shortcut settings. For persistent issues, check the issue details and logs from Home; see `FRAME_SYNC_GUIDE.md` in the package for more frame-rate guidance.

## Assets: Purpose and Instructions

| Asset | Purpose and Instructions |
| --- | --- |
| `Magpie-Experimental-x64.zip` | **Required main package**, containing the application and its runtime components; extract it completely as described above. All other assets are optional. |
| `ScalingModes-v0.6.5-experimental.json` | Import from the Effect groups page to append DLSSFG, XeSSFG and DLSSNR presets without replacing existing groups. |
| `DLSSNR-DLL-Options-310.8.0.0.zip` | Contains official NVIDIA and community RTX 40/50-compatible DLL choices; download only when switching versions. Fully exit Magpie, back up `nvngx_dlssnr.dll`, then follow the archive instructions to place one choice beside `Magpie.exe`. |
| `NGX_OTA_Switch.bat` | Inspects or toggles NVIDIA NGX OTA updates and cleans up update processes; normal installation does not require it. Relevant actions require administrator privileges and affect system-wide NGX settings; use **Restore default** to undo changes, since deleting the BAT does not restore settings. |
