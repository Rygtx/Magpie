# Magpie Experimental v0.6.7 Beta 1

相较 0.6.6 的功能变化：

- 新增按用途分类的效果器选择器，为全部内置效果提供用途、组合位置、适用场景及必要的 HDR 注意事项。
- 选择器支持入门／进阶入口、主分类与子分类、同族折叠、搜索和单列 `# / A–Z` 快速定位。
- 选择器采用与主体一致的圆角和主题配色，可超出主窗口显示，并以加减号展开或收起分类。
- 单击效果器追加到组末尾，底部固定 200 高度的说明区支持 Ctrl＋滚轮查看超出内容。
- 修复添加效果器闪退、列表异常空行、参数选项空白及设置失效的问题。
- RTX Video Denoise／VSR 各合并为一个效果器，强度默认中档并可实时调节，旧档位配置自动迁移。
- RTX Video 参数仅在参数按钮及工具栏的参数面板中显示。
- DLSS SR 与 FSR／XeSS 超分辨率归为同类，DLSS NR 按风格化用途说明，并移除均衡档的“推荐”备注。
- XeSS FG／MFG 显示名称移除 ZeroMV，继续兼容旧效果组。
- DLSS NR／FG 和运动向量／置信度调试效果增加 AMDOF／NVOF 提供者及对应质量选择。
- 所有光流消费者默认不使用光流，旧效果组升级时统一关闭一次，此后保留用户的手动选择。
- DLSS NR 根据输入自动选择颜色处理路径，移除手动 HDR 路径和实验倍率参数。
- 减少 DLSS NR 冗余复制和 DLSS FG 重复资源操作，并默认关闭详细逐帧性能采样以降低开销。
- DLSS FG 接入 Reflex，默认开启低延迟、关闭 Boost，且不额外设置驱动限帧目标。
- 帧同步新增 Async 与普通效果链的 NVIDIA Reflex 模式，工具栏显示实际模式及回退状态。
- 新增“HDR 组件”分类，提供 HDR → SDR、SDR → HDR 和 RTX Video HDR，可自行组合 HDR 兼容效果组。
- 隐藏原全局 HDR 入口并关闭新旧配置中的该开关，HDR 捕获与输出由显式转换组件启用。
- 改进 HDR 管线的亮度保持、效果间颜色转换及初始化错误提示。
- 增强版配置独立保存到 `v4e`，优先读取已有 `v4e`，不存在时静默导入 `v4` 并保留原版存档。
- 损坏配置自动备份并尽量保留可用内容，失效效果留在原组提示替换，含失效项的组在启动前拦截。
- 修复便携模式切换和异步保存顺序问题，避免较新的配置被旧保存任务覆盖。
- 错误提示补充具体问题、相关设置入口和下一步操作，主页可查看最近一次问题及日志。
- 启动前统一检查效果、捕获和窗口条件，窗口模式遇到全屏或最大化源应用时提示先切换为普通窗口。
- 修复停止缩放时迟到回调可能导致的闪退。
- 修复窄窗口下中文说明被裁剪，并在缩放成功启动时显示确认提示。
- 修正旧预设的 DLSS FG 倍率参数及反馈、触控说明入口。

Contributor: [TurnX-alt](https://github.com/TurnX-alt) 提供 PR #23 的界面、预设与入口修复；[konodiodaaaaa1](https://github.com/konodiodaaaaa1) 提供 PR #24 的 HDR 兼容管线基础与集成。

---

# Magpie Experimental v0.6.7 Beta 1

Functional changes since 0.6.6:

- Added a purpose-based picker with usage descriptions, placement advice, suitable scenarios and relevant HDR cautions for all built-in effects.
- The picker supports Beginner/Advanced shortcuts, categories and subcategories, collapsible families, search and a single-column `# / A–Z` index.
- The picker follows the application's rounded theme, can extend beyond the main window and uses plus/minus controls for category expansion.
- Clicking an effect appends it to the group, while the fixed-height 200 description area supports Ctrl + wheel scrolling for overflow.
- Fixed crashes when adding effects, unexpected blank rows, empty parameter choices and settings that failed to apply.
- RTX Video Denoise and VSR each use one effect with live strength adjustment, default to Medium and migrate legacy tier configurations automatically.
- RTX Video parameters appear only through the parameter button and toolbar parameter panel.
- DLSS SR is grouped with FSR/XeSS super resolution, while DLSS NR is described as stylization and its Balanced option no longer carries a recommendation annotation.
- XeSS FG/MFG display names omit ZeroMV while retaining compatibility with existing groups.
- DLSS NR/FG and motion-vector/confidence diagnostics add AMDOF/NVOF provider and quality selection.
- All optical-flow consumers default to None, with a one-time reset for existing groups followed by preservation of subsequent manual choices.
- DLSS NR selects its color-processing path from the input automatically, replacing manual HDR-path and experimental-scale controls.
- Reduced redundant DLSS NR copies and recurring DLSS FG resource operations, with detailed per-frame timing disabled by default to reduce overhead.
- DLSS FG integrates Reflex with low latency On, Boost Off and no additional driver frame-rate target.
- Frame synchronization adds Async and NVIDIA Reflex for ordinary effect chains, with active-mode and fallback status in the toolbar.
- Added an HDR Components category with HDR to SDR, SDR to HDR and RTX Video HDR for custom HDR-compatible groups.
- The old global HDR entry is hidden and disabled in new and existing configurations, with explicit conversion components enabling HDR capture and output.
- Improved HDR luminance preservation, color conversion between effects and initialization error guidance.
- Enhanced settings use a separate `v4e` location, preferring existing v4e settings and otherwise silently importing v4 while preserving the upstream file.
- Damaged settings are backed up and repaired where possible, with invalid effects retained for replacement and affected groups blocked before startup.
- Fixed portable-mode switching and asynchronous save ordering to prevent older save tasks from overwriting newer settings.
- Error messages identify the issue, relevant settings and next steps, with recent-issue details and logs available from Home.
- Startup checks effect, capture and window requirements first, asking users to restore fullscreen or maximized source applications before windowed scaling.
- Fixed a possible crash caused by late callbacks when stopping scaling.
- Fixed clipped Chinese descriptions in narrow windows and added a confirmation when scaling starts successfully.
- Corrected the legacy preset's DLSS FG multiplier and the feedback and touch-documentation links.

Contributor: [TurnX-alt](https://github.com/TurnX-alt) contributed UI, preset and navigation fixes in PR #23; [konodiodaaaaa1](https://github.com/konodiodaaaaa1) contributed the HDR compatibility pipeline foundation and integration in PR #24.
