# Magpie Experimental v0.6.5

本版汇总 0.6.5 r1–r10 的更新，以下以最终保留的功能为准。开发期间撤回、替换或暂缓的功能已单独标注。

## Magpie 本体更新

- **“缩放模式”改名为“效果组”**：一个效果组可以包含多个效果器。主页、倒计时和快捷键说明同步调整，更容易区分效果组合、全屏／窗口输出以及尺寸缩放；已有配置继续兼容。
- **可在使用效果组时调节参数**：工具栏新增“效果参数”窗口，可拖动位置、调整大小；主窗口参数页支持分组展开、下拉选项、较长标签和简体／繁体中文翻译。
- **参数自动保存**：修改后自动保存，不必再点保存。界面会区分修改值与实际生效值；标为“重启”的参数仍需“应用并重新启用”。**原先的独立保存按钮已移除。**
- **双击滑条恢复默认值**：在 170 毫秒内双击，恢复该效果器自身默认值，而不是效果组为它设置的覆盖值。“恢复本次启用初始值”则用于撤销本次试调。
- **工具栏快捷键更齐全**：性能监测、效果参数、截屏、固定工具栏和对比均可设置快捷键，统一放在主页可收起的“工具栏快捷键设置”中。
- **新增“对比”**：一键切换原图和处理后画面。对比原图期间效果继续处理，切回时无需重新加载。右下角“原图／处理后”角标立即显示，2 秒后用 500 毫秒淡出；不再一直占据画面。
- **新增 Front Edge Sync**：默认开启、目标 60 FPS，用于稳定画面提交节奏；有 FG 时控制补帧前的真实帧输入。可能增加延迟，需要配合目标游戏／程序的限帧设置。
- **补帧帧率更直观**：启用 FG 后，帧率支持类似 `120/60` 的“输出帧率／真实帧率”显示。
- **默认效果组调整**：新配置包含 Lanczos、FSR、RTX Video VSR Ultra、DLSSFG、XeSSFG、DLSSNR，默认选择 Lanczos。默认 DLSSNR 组不再单独附加帧率过滤器；不会自动替换已有自定义组。
- **[已撤回／暂缓] VRR 与 HDR**：VRR 开关暂时隐藏并停用，旧配置中开启的值不生效；全屏强制焦点尝试已撤回。HDR 仍留待后续版本，本版没有新增 HDR 效果器。

## 效果器更新

- **DLSSNR 细节控制更完整**：除总残差倍率外，新增残差饱和度、亮度、阴影／结构和反射／辉光控制，可分别调整细节回填的强度与观感。默认值保持中性；提高倍率也可能放大伪影。
- **DLSSNR 降分辨率处理改进**：优化降低输入分辨率后的缩小和细节回填。开启“调整输入分辨率”后，即使比例为 100%，也可以使用残差控制；关闭该开关时不使用这些残差调整。
- **DLSSNR 核心参数调整更稳妥**：修改强度、风格等核心参数，或修改它前面的图像效果参数后，会合并连续编辑，完整停用效果组，等待 500 毫秒再以新值重新启用。原来可实时调整的残差参数继续实时生效。**上述核心／上游参数已由直接实时修改改为自动重新启用。**
- **SR 效果入口简化**：DLSS SR、FSR 2、FSR 3、FSR 4、XeSS SR 各保留一个入口，可在参数中选择不使用光流、AMD OF 或 NVOF。**独立 Zero MV、Optical Flow、jitter 实验入口及旧内置光流已移除，旧配置自动迁移。**
- **光流方法与质量可选**：AMD OF 提供性能／质量；NVOF 提供性能、均衡（推荐）、质量、高质量（高开销）、最高质量（极高开销）。最高质量使用 2×2 Slow；质量档取消“推荐”备注，均衡档保留。
- **多个效果器共用光流**：SR、NR、FG 同时使用时，共用一次光流估算。选择不一致时，优先使用实际申请中的 NVOF，再使用 AMD OF，并采用该来源实际申请中的较高档位；不会自动开到最高质量。界面会说明哪些效果器设置不一致，以及实际采用的方法。**此前不同来源／档位各自计算的方案已被统一选择替代。**
- **FG 光流选择补全**：DLSSNR、DLSSFG 可选不使用或 NVOF；XeSSFG x2 可选不使用、AMD OF、NVOF；XeSS Multi-FG 当前可选不使用或 AMD OF。可用性仍取决于显卡、驱动和运行时。
- **FrameRate Filter 跟随同步设置**：默认“基于 Front Edge Sync”。Front Edge Sync 开启时锁定跟随；关闭后可选择“自定义”并调节帧率滑条，原自定义值会保留。
- **参数范围与默认值修正**：DLSSNR 强度、局部色调和局部结构统一为 0–1，旧超范围值自动调整；Jinc 的步进可正确表达 0.825，运动可视化的默认增益统一为 0.08。
- **[已移除] 深度估算**：移除学习型深度估算、相关诊断效果，以及 DirectML／TensorRT 深度组件。旧深度选项自动清理；DLSS 等效果不再提供估算深度选项。

## 错误与稳定性修复

- **参数保存与配置恢复**：修复保存成功后闪退、连续修改时保存顺序不正确等问题。保存保留上一份有效备份；配置损坏时先保留原文件，再尝试恢复备份或完整可读内容，减少损坏配置导致无法启动的情况。
- **参数与画面保持一致**：修复窗口变化后参数恢复旧值、应用失败时控件与实际画面不一致，以及特定效果器的参数显隐误影响其他效果器的问题。
- **浮窗交互修复**：改善启用 FG、静止画面和较低帧率下的按钮、滑条、滚轮、拖动与尺寸调整；下拉框打开后点击其他位置或失去焦点可自动收起。
- **切屏与捕获恢复**：修复切屏、窗口尺寸变化或捕获中断时继续处理不完整／重复输入的路径。恢复期间保留最后有效画面，恢复后重新建立处理节奏。
- **性能监测与补帧稳定性**：修复性能分析器采样可能造成等待、拖动浮窗时干扰 DLSSFG，以及恢复后帧间隔异常的路径。减少工具栏和光标不必要的重复呈现，优先显示新画面；GPU 调度优先级保持 REALTIME。
- **光流降级与启动开销**：光流停止工作后，后续效果不再持续反复重置；避免没有变更时重复编译 AMD OF 着色器。
- **更有用的报错**：捕获、窗口状态、设备、效果启动、保存、导入／导出、截图与文件选择器的问题会给出对应建议。主页“最近一次问题”可查看详情、复制诊断信息、打开日志目录，也可手动关闭；正常取消操作不会被当作失败。

以上修复针对已定位的问题，不代表所有设备上的切屏异常或 1% Low 卡顿都已解决。

## 使用说明

### 安装或从旧版升级

所有用户都使用 `Magpie-Experimental-x64.zip`。本版涉及效果删除与配置迁移，不提供从 0.6.1 覆盖安装的最小更新包。

1. 先备份配置和旧安装，完全退出 Magpie。
2. 将主包完整解压到新目录，运行其中的 `Magpie.exe`；不要在 ZIP 内运行或只替换 EXE。
3. 不要复制旧效果目录或旧 DirectML／TensorRT 深度组件到新包。

普通设置仍位于 `%LOCALAPPDATA%\Magpie\config\v4\config.json`。便携用户可将原有 `config` 目录（含 `config.json`）复制到新安装。保留旧安装及配置备份便于回退。

旧 Zero MV／jitter 配置迁移为不使用光流；旧 FSR／XeSS Optical Flow 迁移为 AMD OF 质量档，旧 DLSS Optical Flow 保留 NVOF 均衡。有效的关闭状态、档位、效果组顺序与尺寸设置继续保留。

### 参数调整与默认值

工具栏“效果参数”中的 **Live／实时**立即生效，**Restart／重启**保存后等待手动应用，**Auto restart／自动重启**在编辑结束后自动重新启用效果组。自动重启前会合并连续修改；停用完成后等待 500 毫秒，效果初始化还需要额外时间。手动停用会取消等待中的重启。

DLSSNR 主要参数如下：

| 参数 | 范围 | 默认值 | 步进 |
| --- | --- | --- | --- |
| 输入分辨率比例 | 25–100% | 100%，总开关默认关闭 | 1% |
| 残差倍率 | 1–2 | 1 | 0.05 |
| 残差饱和度、亮度、阴影／结构、反射／辉光 | 各 0–2 | 各 1 | 0.05 |
| NR 强度、局部色调、局部结构 | 各 0–1 | 各 1 | 0.05 |
| 皮肤结构 | -1–2 | -1 | 0.05 |

自动遮罩和 UI 修正默认关闭。残差五项只在“调整输入分辨率”启用时使用，100% 也有效。降低输入比例可降低处理压力，但会损失部分画面信息；先保持默认值，再小幅调整。

### Front Edge Sync 与补帧

- 默认启用，目标 60 FPS；设置在重新启用效果组后生效。目标程序需要配合限制帧率。
- 目标可设 `0`（自动）或 `1–1000 FPS`。自动按显示器刷新率设置，FG 按倍率折算基础帧率；手动值表示补帧前帧率，例如 80 FPS 配合 2× FG，名义输出 160 FPS。
- FrameRate Filter 跟随模式不能单独改上限；关闭 Front Edge Sync 后才可选自定义，范围 1–240 FPS，默认 60，步进 1。已有更低的捕获／配置限制仍会生效。
- 同一效果组只保留一种 FG，避免与 NVIDIA Smooth Motion 等其他补帧叠加。若延迟增加，可关闭 Front Edge Sync 比较；若 GPU 压力较大，可降低光流档位。
- 新建 DLSS SR 默认 NVOF 均衡；新建 FSR 2/3/4、XeSS SR 默认不使用光流。NVOF 最高质量不一定更适合当前画面，先从均衡或不使用光流开始。

Magpie 从已捕获画面估算运动，不能获得游戏原生的运动矢量、深度和 UI 分离信息，因此不等同于游戏原生 DLSS／FSR／XeSS 接入。更多同步说明见主包的 `FRAME_SYNC_GUIDE.md`；本版不承诺使 FreeSync／G-SYNC 生效。

### 工具栏快捷键与排错

| 功能 | 默认快捷键 |
| --- | --- |
| 性能监测 | Alt+Shift+P |
| 效果参数 | Alt+Shift+E |
| 截屏 | Alt+Shift+S |
| 固定工具栏 | Alt+Shift+F |
| 对比 | Alt+Shift+C |

可在主页“工具栏快捷键设置”中修改。出现持续异常时先停用并重新启用效果组；在主页查看“最近一次问题”的详情并打开日志目录，再比较关闭 FG、光流或 Front Edge Sync 后的表现。

## 附件的作用与使用

### 完整主包：`Magpie-Experimental-x64.zip`

所有用户的必选包。完整解压到新目录即可使用，包含本次构建所需运行时、Release Note 和帧同步说明。其他附件均为可选。

### 效果组预设：`ScalingModes-v0.6.5-experimental.json`

在“效果组”页面导入，追加 DLSSFG、XeSSFG、DLSSNR 三组，不会删除或替换已有组。帧率过滤器默认跟随 Front Edge Sync；无需为了升级而重置全部效果组。

### DLSSNR DLL 选项：`DLSSNR-DLL-Options-310.8.0.0.zip`

沿用 0.6.1 的 NVIDIA 官方版和 RTX 40/50 社区兼容版选项。只有需要切换 DLL 时才下载。

完全退出 Magpie，备份现有 `nvngx_dlssnr.dll`，再从选项包选择一个版本复制到 `Magpie.exe` 所在目录并覆盖。遇到问题可恢复备份；具体版本与路径见包内说明。不同显卡／驱动组合的兼容性仍需自行确认。

### NGX OTA 开关：`NGX_OTA_Switch.bat`

当怀疑 NVIDIA NGX OTA 更新进程积累与异常内存占用有关时，可使用此工具；常规安装无需运行。

运行 BAT 后，菜单可查看状态、关闭／启用 OTA、清理 `nvngx_update.exe`，或使用 **Restore default** 恢复 NVIDIA 默认值。相关操作需要管理员权限，会修改系统级 NGX 设置并影响其他 NGX 程序。删除 BAT 不会撤销设置，恢复时请使用菜单。它不是所有卡顿或画面异常的通用修复。

---

# Magpie Experimental v0.6.5

This release combines the r1–r10 updates. The list describes the final behavior and marks features withdrawn, replaced or deferred during development.

## Magpie Application Updates

- **Scaling modes are now Effect groups**: each group combines multiple effects. Home, countdown and shortcut wording distinguishes effect combinations, fullscreen/windowed output and size scaling. Existing settings remain compatible.
- **Adjust parameters while a group is running**: the toolbar now provides a movable, resizable Effect parameters window. The main editor supports grouped layouts, drop-downs, long labels and Simplified/Traditional Chinese translations.
- **Automatic parameter saving**: edits save without an extra click. The editor distinguishes desired and applied values; Restart parameters still need Apply and restart. **The separate Save button has been removed.**
- **Double-click to restore a default**: double-click a slider within 170 ms to restore the effect's own default, rather than a group's override. Restore session startup values reverts the current session's adjustments.
- **More toolbar shortcuts**: configure the profiler, parameters, screenshots, pinning and comparison in Home's collapsible Toolbar shortcut settings section.
- **New Comparison control**: switch between original and processed images while effects continue running, so switching back needs no reload. The Original/Processed corner badge appears immediately, stays two seconds and fades over 500 ms instead of remaining permanently visible.
- **New Front Edge Sync**: enabled by default at 60 FPS to stabilize submissions, controlling real input frames before FG when FG is active. It may increase latency and requires a matching limiter in the source game/application.
- **Clearer FG frame rates**: with FG, the display supports output/real-frame readings such as `120/60`.
- **Updated default groups**: fresh configurations contain Lanczos, FSR, RTX Video VSR Ultra, DLSSFG, XeSSFG and DLSSNR, selecting Lanczos. The default DLSSNR group no longer adds a separate frame-rate filter. Custom groups are not replaced automatically.
- **[Withdrawn/deferred] VRR and HDR**: VRR is hidden and disabled, including previously enabled settings. The forced-fullscreen-focus experiment was withdrawn. HDR remains deferred; this version adds no HDR effects.

## Effect Updates

- **More DLSSNR detail controls**: residual saturation, lightness, shadow/structure and reflection/glow join the overall residual multiplier, allowing finer control of reconstructed detail. Defaults are neutral; higher multipliers can also amplify artifacts.
- **Improved reduced-resolution DLSSNR**: image reduction and detail reconstruction are improved when lowering input resolution. Residual controls also work at 100% when Adjust Input Resolution is enabled; disabling it bypasses these adjustments.
- **Safer DLSSNR core adjustments**: changes to intensity, style or upstream image effects are coalesced, then the group fully stops and restarts with the new values after 500 ms. Previously live residual controls remain live. **These core/upstream parameters now use automatic restart instead of direct live changes.**
- **Simplified SR entries**: DLSS SR, FSR 2/3/4 and XeSS SR each have one entry with None / AMD OF / NVOF selection. **Separate Zero MV, Optical Flow and jitter experiments, plus the old built-in optical flow, are removed; old settings migrate automatically.**
- **Selectable optical-flow quality**: AMD OF offers Performance/Quality. NVOF offers Performance, Balanced (Recommended), Quality, High Quality (High Cost), and Highest Quality (Very High Cost). Highest Quality uses 2×2 Slow. Quality is no longer marked Recommended; Balanced still is.
- **Shared flow across effects**: SR, NR and FG share one estimate. If their requests differ, requested NVOF takes priority over AMD OF, using the highest quality actually requested for that source. It does not automatically select the highest available quality. An informational notice identifies the affected effects and the selected method. **The earlier independent computation for differing sources/qualities has been replaced by this shared selection.**
- **More FG flow choices**: DLSSNR/DLSSFG offer None or NVOF; XeSSFG x2 offers None, AMD OF or NVOF; XeSS Multi-FG currently offers None or AMD OF. Availability still depends on the GPU, driver and runtime.
- **FrameRate Filter follows synchronization**: it defaults to Based on Front Edge Sync and is locked to that mode while Front Edge Sync is enabled. Disable synchronization to choose Custom and use the frame-rate slider; the custom value is retained.
- **Corrected parameter ranges/defaults**: NR intensity, local tone and local structure now use 0–1, with old out-of-range values normalized. Jinc's step correctly represents 0.825, and motion visualization consistently defaults to a gain of 0.08.
- **[Removed] Depth estimation**: learned depth, related diagnostic effects and DirectML/TensorRT depth components are removed. Legacy depth settings are cleaned up; DLSS effects no longer offer estimated depth.

## Error and Stability Fixes

- **Saving and configuration recovery**: fixed exits after a successful save and incorrect ordering of consecutive saves. A previous valid configuration is retained; damaged originals are preserved before recovery from a backup or complete readable content, reducing cases where corruption prevents startup.
- **Parameters match the image**: fixed old values returning after window changes, controls disagreeing with applied values after failure, and effect-specific visibility rules affecting unrelated effects.
- **Overlay interaction**: improved buttons, sliders, scrolling, dragging and resizing with FG, static images and low frame rates. Drop-downs dismiss on an outside click or loss of focus.
- **Switching and capture recovery**: fixed paths that kept processing incomplete/repeated input during switching, resizing or capture interruption. The last valid image is retained during recovery and normal pacing is re-established afterward.
- **Profiler and FG stability**: fixed waits caused by profiler sampling, overlay dragging interfering with DLSSFG, and abnormal frame intervals after recovery. Reduced unnecessary toolbar/cursor presentations and prioritized new images. GPU scheduling priority remains REALTIME.
- **Optical-flow fallback and startup overhead**: stopping optical flow no longer repeatedly resets downstream effects, and unchanged AMD OF shaders no longer recompile unnecessarily.
- **Actionable errors**: capture, window state, device/effect startup, saving, import/export, screenshots and file-picker failures have relevant suggestions. Home's recent-issue card offers details, diagnostic copying, log access and manual dismissal. Normal cancellation is not reported as failure.

These fixes address identified problems; they do not mean every switching artifact or 1% Low stall is resolved on every device.

## Usage

### Install or Upgrade

All users should use `Magpie-Experimental-x64.zip`. Effect removals and settings migration mean there is no in-place minimal update from 0.6.1.

1. Back up the configuration and old installation, then fully exit Magpie.
2. Extract the complete main ZIP into a new directory and run its `Magpie.exe`. Do not run inside the ZIP or replace only the EXE.
3. Do not copy old effects or DirectML/TensorRT depth components into the new package.

Normal settings remain at `%LOCALAPPDATA%\Magpie\config\v4\config.json`. Portable users can copy their existing `config` directory, including `config.json`, to the new installation. Retain the old installation and configuration backup for rollback.

Legacy Zero MV/jitter settings migrate to None; legacy FSR/XeSS Optical Flow migrates to AMD OF Quality, and legacy DLSS Optical Flow retains NVOF Balanced. Valid disabled/quality settings, group order and size settings remain intact.

### Parameter Editing and Defaults

**Live** applies immediately; **Restart** saves the change but requires manual application; **Auto restart** re-enables the group after editing. Consecutive changes coalesce. Automatic restart waits 500 ms after stopping, and effect initialization takes additional time. Manually stopping cancels the pending restart.

Main DLSSNR controls:

| Parameter | Range | Default | Step |
| --- | --- | --- | --- |
| Input resolution | 25–100% | 100%; adjustment switch off | 1% |
| Residual multiplier | 1–2 | 1 | 0.05 |
| Residual saturation, lightness, shadow/structure, reflection/glow | Each 0–2 | Each 1 | 0.05 |
| NR intensity, local tone, local structure | Each 0–1 | Each 1 | 0.05 |
| Skin structure | -1–2 | -1 | 0.05 |

Automatic mask and UI correction default to off. All five residual controls require Adjust Input Resolution, including at 100%. Lowering the percentage reduces processing pressure but loses some image information. Start with defaults and make small adjustments.

### Front Edge Sync and FG

- Enabled by default at 60 FPS; changes apply when enabling the group again. Apply a matching source-application limiter.
- The target accepts `0` (automatic) or `1–1000 FPS`. Automatic mode uses display refresh rate divided by the FG multiplier; a manual target is the rate before FG. For example, 80 FPS with 2× FG nominally outputs 160 FPS.
- FrameRate Filter cannot set its own cap in follow mode. Disable Front Edge Sync to use Custom, with a range of 1–240 FPS, default 60 and step 1. Existing lower capture/profile limits still apply.
- Use only one FG per group and avoid stacking it with NVIDIA Smooth Motion or another generator. Compare with Front Edge Sync off if latency increases; lower flow quality if GPU pressure is high.
- New DLSS SR defaults to NVOF Balanced; new FSR 2/3/4 and XeSS SR default to None. Highest Quality is not necessarily better for your content; start with Balanced or None.

Magpie estimates motion from captured images and lacks engine-native motion, depth and UI separation, so this is not equivalent to native DLSS/FSR/XeSS integration. See `FRAME_SYNC_GUIDE.md` in the main package. This release does not promise working FreeSync/G-SYNC output.

### Shortcuts and Troubleshooting

| Action | Default Shortcut |
| --- | --- |
| Profiler | Alt+Shift+P |
| Effect parameters | Alt+Shift+E |
| Screenshot | Alt+Shift+S |
| Pin toolbar | Alt+Shift+F |
| Comparison | Alt+Shift+C |

Customize these in Home's Toolbar shortcut settings. For a persistent problem, stop and restart the group, inspect Home's recent-issue details and logs, then compare with FG, optical flow or Front Edge Sync disabled.

## Assets: Purpose and Instructions

### Main Package: `Magpie-Experimental-x64.zip`

Required for all users. Extract it completely into a new directory. It contains this build's runtimes, Release Note and frame-sync guide. All other assets are optional.

### Group Presets: `ScalingModes-v0.6.5-experimental.json`

Import from the Effect groups page to append DLSSFG, XeSSFG and DLSSNR without removing or replacing existing groups. Frame-rate filtering follows Front Edge Sync by default. Upgrading does not require resetting all groups.

### DLSSNR DLL Choices: `DLSSNR-DLL-Options-310.8.0.0.zip`

Reuses 0.6.1's official NVIDIA and community RTX 40/50-compatible options. Download only if you need to switch DLLs.

Fully exit Magpie, back up `nvngx_dlssnr.dll`, then copy one choice beside `Magpie.exe`, allowing overwrite. Restore the backup if needed. See the archive instructions for exact versions and paths; compatibility with a particular GPU/driver combination still requires checking.

### NGX OTA Switch: `NGX_OTA_Switch.bat`

Use when accumulated NVIDIA NGX OTA update processes are suspected in abnormal memory usage. It is not needed for normal installation.

Its menu inspects status, disables/enables OTA, cleans up `nvngx_update.exe`, or restores NVIDIA defaults with **Restore default**. Relevant actions require administrator privileges and affect system-wide NGX settings, including other NGX applications. Deleting the BAT does not undo the setting; use its restore menu. This is not a universal fix for stalls or image artifacts.
