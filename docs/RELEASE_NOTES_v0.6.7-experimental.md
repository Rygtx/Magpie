# Magpie Experimental v0.6.7

## 更新内容

1. **新增效果器选择器**：支持用途分类、搜索、同族折叠、入门／进阶筛选和字母定位；分类支持整行点击，说明区保持固定高度，可使用 `Ctrl+滚轮` 滚动较长说明。
2. **完善实时参数编辑与预览**：参数面板可常驻预览，点击控件即可进入编辑并执行本次操作；支持拖动滑条、下拉选择和 `Ctrl+单击` 输入实际数值。工具栏按钮与默认快捷键 `Alt+Shift+E` 统一控制开关，`Esc` 按“编辑 → 预览 → 关闭”逐级退出。
3. **改进全屏参数交接与停止重建**：修复已确认的重复停止请求、待重建请求丢失和旧帧消费确认问题；输入未释放时保留待处理请求，短暂源窗口变化恢复后继续呈现。不同游戏的失焦、锁鼠与后台输入行为仍需分别验证。
4. **增强配置恢复和效果组管理**：增强版配置独立保存到 `v4e`，可导入旧配置；损坏配置尽量按备份或有效条目恢复。修复配置目录缺失、便携保存及删除效果组时的问题；新建、复制、导入和重命名时避免效果组重名。
5. **新增 Async 与 NVIDIA Reflex 帧同步模式**：普通效果和 DLSS FG 均可选择 Reflex 驱动基础限帧，统一基础目标不叠加多个限帧器；XeSS FG／MFG 仍由 XeLL 接管。Reflex 自动回退仅记简短日志，实时参数面板不显示同步、Reflex 或 FG 输出状态。
6. **RTX Video 降噪与 VSR 改为参数化强度**：低／中／高／极高四档可实时调整，默认中档；旧分档效果自动迁移并保留对应强度。
7. **扩展 DLSSNR 参数与输入适配**：NR 强度、局部色调强度和局部结构强度范围为 `0–2`，默认 `1`；皮肤结构强度范围为 `0–2`，默认 `0`，步进均为 `0.05`。NR 根据效果链输入自动适配 SDR／HDR。
8. **扩展光流选择与调整默认行为**：DLSS NR、DLSS FG 和光流诊断效果支持 AMD／NVIDIA 方法及质量选择。有光流选择的效果默认关闭光流，从旧配置首次迁移时也会关闭一次，之后可自行开启并保存。
9. **新增 HDR 组件**：提供 HDR → SDR、SDR → HDR 和 RTX Video HDR 效果，用于搭建显式转换链；完善 HDR 工作格式、亮度和效果边界处理。HDR 默认关闭，旧全局兼容开关不再作为入口。
10. **优化运行开销与使用引导**：减少 DLSS NR 的重复复制和 DLSS FG 的重复资源准备，改进补帧输出排序、队列等待与停止处理；完善错误详情、恢复步骤、效果说明、工具栏快捷键提示和帮助链接。XeSS 补帧显示名称统一去掉 ZeroMV 后缀，保留旧效果 ID 兼容。

## 使用说明

### 安装或从旧版升级

**请使用全新程序目录安装，不直接覆盖 0.6.6 或旧 Beta 目录。**

1. 如需保留设置或截图，先备份到程序目录之外。
2. 从托盘完全退出 Magpie。
3. 将 `Magpie-Experimental-x64.zip` 完整解压到新目录，运行其中的 `Magpie.exe`。程序与 `resources.pri` 必须来自同一次构建，不要仅替换 EXE。

普通配置优先读取 `%LOCALAPPDATA%\Magpie\config\v4e\config.json`；没有增强版配置时可导入旧 `v4` 配置，后续保存到 `v4e`。便携配置使用新程序目录内的 `config\v4e\config.json`。如需手动迁移，只复制所需配置，保留原文件用于回退；**不要复制旧效果目录、DLL 或深度组件到新目录**。

升级后发现同名效果组时，原内容会保留，请逐个改名；出现失效效果时，可恢复对应效果文件，或移除失效项后添加替代效果。

### 参数与帧率设置

- **参数生效方式**：Live／实时立即生效；Restart／重启需点击“应用并重新启用”。面板位置、大小及编辑／预览状态会在重新启用时恢复。
- **面板操作**：默认 `Alt+Shift+E` 打开或关闭参数面板，可在设置中改绑。点击预览中的控件可直接操作；点击游戏区域返回预览。`Esc` 先关闭临时输入或下拉菜单，再逐级退出面板。
- **统一基础 FPS**：默认开启 Front Edge Sync，目标 `60 FPS`。主页支持 `0` 自动或 `1–1000 FPS`，参数面板滑条为 `15–360 FPS`、步进 `1`；已有合法值不会因打开面板而被改写。修改自动保存，重新启用缩放后生效。
- **Front Edge／Async／Reflex**：Front Edge 保留原提交节奏；Async 在捕获前限制基础输入间隔；Reflex 将同一基础目标交给 NVIDIA 驱动。Reflex 要求支持的 NVIDIA DXGI 呈现路径，效果与呈现使用同一显卡；当前异步标记接口要求 R565+。不可用时自动回退 Async，仅记日志。关闭帧同步仍保留其他既有上限和 FG 的低延迟机制。
- **自动目标与补帧**：`0` 按显示器刷新率折算基础 FPS，有 FG 时除以倍率。例如 240 Hz 下，2×／3×／4× 分别为基础 120／80／60 FPS；手动 80 FPS 配合 2× 的名义输出为 160 FPS。源程序限帧仍需单独设置，相同数值不代表逐帧同步；源或 GPU 跟不上时实际帧率会更低。
- **DLSSNR 残差控制**：先开启“调整输入分辨率”，即使比例为 100% 也能使用残差调整。降低比例可减轻性能压力，但会损失部分画面信息。
- **补帧与光流搭配**：一个效果组只使用一种 FG。需要光流时手动选择方法和质量；性能压力较大时可降低质量或关闭光流对比。
- **HDR 使用**：根据源内容和输出需要排列“HDR 组件”；HDR 输出需要相应显示环境。不要用旧全局兼容开关代替转换链配置。

代码回归与实机验收分开记录。游戏焦点和锁鼠、HDR 捕获／显示组合、实际驱动限帧及显示端延迟仍需实机确认；本说明不预先承诺性能收益。

## 附件的作用与使用

| 附件 | 用途与使用方法 |
| --- | --- |
| `Magpie-Experimental-x64.zip` | **必选主包**，包含程序、匹配的界面资源、效果与所需运行组件；按上述步骤完整解压。其余附件均为可选。 |
| `DLSSNR-DLL-Options-310.8.0.0.zip` | 沿用 0.6.6 的 NVIDIA 官方版与 RTX 40/50 社区兼容版 DLL 选项；仅需切换 NR DLL 时下载。完全退出 Magpie，备份现有 `nvngx_dlssnr.dll`，按包内说明选择一个版本放到 `Magpie.exe` 旁。 |
| `NGX_OTA_Switch.bat` | 沿用 0.6.6 的可选工具，用于查看、开关 NVIDIA NGX OTA 更新及清理更新进程，常规安装无需运行。相关操作需管理员权限且影响系统级 NGX 设置；恢复时使用 **Restore default**，删除 BAT 不会撤销设置。 |

Contributor: [TurnX-alt](https://github.com/TurnX-alt) 提供界面、预设与构建一致性修复；[konodiodaaaaa1](https://github.com/konodiodaaaaa1) 提供 HDR 支持。

---

# Magpie Experimental v0.6.7

## Updates

1. **Added an effect picker** with purpose categories, search, collapsible families, Beginner/Advanced filtering and alphabetical navigation. Categories support full-row activation; a fixed description area supports `Ctrl+wheel` scrolling for longer text.
2. **Improved live editing and persistent preview**: clicking a preview control enters editing and applies that gesture. Sliders, dropdowns and actual-value entry with `Ctrl+click` are supported. The toolbar button and default `Alt+Shift+E` shortcut share one open/close action; Escape steps through Edit → Preview → Closed.
3. **Improved fullscreen parameter handoff and shutdown/restart**: fixed identified duplicate stop requests, lost pending rebuilds and stale-frame consumption acknowledgements. Pending requests survive held input, and rendering can continue after temporary source-window changes recover. Game-specific focus, cursor confinement and background input still require testing.
4. **Strengthened configuration recovery and group management**: enhanced settings use a separate `v4e` directory and can import older settings. Damaged configurations recover from backups or valid entries where possible. Fixes cover missing directories, portable saving and group deletion; creation, copying, importing and renaming avoid duplicate group names.
5. **Added Async and NVIDIA Reflex frame-sync modes**: ordinary effects and DLSS FG can use Reflex driver base pacing without stacking multiple limiters for the same target. XeSS FG/MFG remains controlled by XeLL. Automatic Reflex fallback only writes a brief log; live parameters do not display synchronization, Reflex or FG output status.
6. **Parameterized RTX Video Denoise and VSR strength**: Low/Medium/High/Ultra levels apply live, with Medium as the default. Legacy tier-specific effects migrate while retaining the corresponding strength.
7. **Expanded DLSSNR controls and input adaptation**: NR intensity, local tone strength and local structure strength range from `0–2`, defaulting to `1`; skin structure strength ranges from `0–2`, defaulting to `0`. All four use steps of `0.05`. NR adapts to SDR/HDR based on its effect-chain input.
8. **Extended optical-flow selection and changed defaults**: DLSS NR, DLSS FG and optical-flow diagnostics support AMD/NVIDIA methods and quality levels. Effects with an optical-flow selector default to Off; the first migration from older settings also disables it once, after which users can enable and save their choice.
9. **Added HDR Components**: HDR to SDR, SDR to HDR and RTX Video HDR effects form explicit conversion chains. HDR working formats, luminance and effect boundaries have been improved. HDR defaults to Off; the old global compatibility switch is no longer the entry point.
10. **Reduced recurring work and improved guidance**: reduced redundant DLSS NR copies and DLSS FG resource preparation, and improved generated-frame ordering, queue waits and shutdown handling. Error details, recovery actions, effect descriptions, shortcut tooltips and help links are improved. XeSS frame-generation display names omit ZeroMV while retaining compatible effect IDs.

## Usage

### Install or Upgrade

**Install into a new program folder; do not overwrite a 0.6.6 or older Beta folder.**

1. Back up any settings or screenshots you want to keep outside the program folders.
2. Fully exit Magpie from the system tray.
3. Extract `Magpie-Experimental-x64.zip` completely into a new folder and run its `Magpie.exe`. Keep the EXE and `resources.pri` from the same build together; do not replace only the EXE.

Normal settings prefer `%LOCALAPPDATA%\Magpie\config\v4e\config.json`. If enhanced settings are absent, older `v4` settings can be imported; subsequent saves use `v4e`. Portable settings use `config\v4e\config.json` inside the new program folder. For manual migration, copy only the required configuration and retain the originals for rollback; **do not copy old effects, DLLs or depth components into the new folder**.

Existing groups with duplicate names retain their contents and should be renamed individually. For an invalid effect, restore its file or remove the entry and add a replacement.

### Parameters and Frame Rates

- **Applying parameters**: Live takes effect immediately; Restart requires Apply and restart. Re-enabling restores panel geometry and its editing/preview state.
- **Panel controls**: `Alt+Shift+E` opens or closes the panel by default and can be rebound in settings. Clicking a preview control operates it directly; clicking the game returns to preview. Escape dismisses temporary input or dropdowns before stepping out of the panel.
- **Unified base FPS**: defaults remain Front Edge Sync enabled at `60 FPS`. Home supports `0` for automatic or `1–1000 FPS`; the panel slider spans `15–360 FPS` in steps of `1`. Opening the panel preserves existing valid values. Changes save automatically and apply when scaling is re-enabled.
- **Front Edge/Async/Reflex**: Front Edge retains the existing submission pacing; Async limits base-input intervals before capture; Reflex gives the same base target to the NVIDIA driver. Reflex requires a supported NVIDIA DXGI path with effects and presentation on the same GPU; the current async marker interface requires R565+. Unavailable Reflex falls back to Async with logging only. Disabling frame sync retains other existing caps and FG low-latency handling.
- **Automatic targets and FG**: `0` derives base FPS from display refresh rate, divided by the FG multiplier. At 240 Hz, 2×/3×/4× target base rates of 120/80/60 FPS. A manual 80 FPS target with 2× FG nominally outputs 160 FPS. Source-application limiting remains separate; matching numbers do not imply frame-by-frame synchronization, and slower sources or GPUs can produce lower actual rates.
- **DLSSNR residual controls**: enable Adjust Input Resolution first, even at 100%. Lowering the percentage can reduce processing pressure but loses some image information.
- **Combining FG and optical flow**: use one FG effect per group. Select an optical-flow method and quality manually when needed; lower its quality or disable it for comparison when performance is constrained.
- **HDR usage**: arrange HDR Components for the source and intended output; HDR output requires a suitable display setup. Do not substitute the old global compatibility switch for a conversion chain.

Code regressions and hardware validation are recorded separately. Game focus and cursor confinement, HDR capture/display combinations, actual driver pacing and display latency still require hands-on validation; this note does not promise performance gains.

## Assets: Purpose and Instructions

| Asset | Purpose and Instructions |
| --- | --- |
| `Magpie-Experimental-x64.zip` | **Required main package**, containing the application, matching UI resources, effects and runtime components. Extract it completely as described above. All other assets are optional. |
| `DLSSNR-DLL-Options-310.8.0.0.zip` | Reuses the official NVIDIA and community RTX 40/50-compatible DLL choices from 0.6.6. Download only when switching NR DLLs. Fully exit Magpie, back up `nvngx_dlssnr.dll`, and follow the archive instructions to place one choice beside `Magpie.exe`. |
| `NGX_OTA_Switch.bat` | Reuses the optional 0.6.6 tool to inspect or toggle NVIDIA NGX OTA updates and clean up update processes; normal installation does not require it. Relevant actions require administrator privileges and affect system-wide NGX settings. Use **Restore default** to undo changes; deleting the BAT does not restore settings. |

Contributor: [TurnX-alt](https://github.com/TurnX-alt) contributed UI, preset and build-consistency fixes; [konodiodaaaaa1](https://github.com/konodiodaaaaa1) contributed HDR support.
