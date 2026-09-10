# Magpie Experimental v0.6.7-beta6-fix1

## 功能更新

### Magpie 本体

- DLSS FG 选择 NVIDIA Reflex 后，由驱动负责统一基础 FPS，捕获前每个基础候选调用一次 Sleep。原 Beta 6 的“选择 Reflex，实际使用 Async”映射已移除。
- Reflex 接管时不叠加同目标 Async／StepTimer 固定限帧或 Front Edge 输入期限等待；更低的配置与捕获上限合并进驱动目标。
- 分别显示请求模式、实际基础限帧器、低延迟请求与查询结果、FG 输出方式。低延迟查询 Off 不再使已接受的驱动限帧自动退回 Async。
- 调用失败时先清除应用设置的驱动限帧再回退 Async，保持同一个目标。清除失败会提示并停止本轮缩放；请重启 Magpie 后再试。
- 默认仍为 Front Edge Sync、60 FPS；保留已有配置。目标 0 为自动，手动范围 1–1000 FPS；自动基础目标为刷新率除以 FG 倍率。关闭帧同步仍保留其他已有上限与 FG 低延迟机制。

### 效果器

- 保留 DLSSFG 生成帧／真实帧 FIFO 顺序、独立输出间隔、槽位所有权、资源背压与 fence。2×／3×／4× 使用同一个基础 FPS 定义。
- 显式 Async、Front Edge 与 XeLL 路径保留。继承 Beta 6 的全屏参数交接、FIFO 重试计时及资源复用修复。

## 应该下载哪个文件？

上一实验版用户和首次使用者均使用完整包 `Magpie-Experimental-x64 v0.6.7-beta6-fix1.zip`。从托盘完全退出 Magpie，完整解压到新目录，再运行其中的 `Magpie.exe`。不要仅替换 EXE；程序和 `resources.pri` 来自同一次构建。

普通配置优先使用 `%LOCALAPPDATA%\Magpie\config\v4e\config.json`；便携用户可将原配置复制到新目录的 `config\v4e\config.json` 并保留旧文件用于回退。没有新增可选组件要求。

## 验证状态与已知边界

代码与无界面回归已完成，实际 NVIDIA 驱动帧率、HDR／捕获组合和性能收益尚未验证。Reflex 要求支持的 NVIDIA DXGI 路径，效果与呈现位于同一显卡，当前异步标记接口要求 R565+。目标表示基础输入上限；源程序或 GPU 跟不上时实际帧率会更低。详细模式与排错见包内 `FRAME_SYNC_GUIDE.md`。

---

# Magpie Experimental v0.6.7-beta6-fix1 User Guide

## Updates

### Magpie application

- Selecting NVIDIA Reflex with DLSS FG now assigns the unified base FPS to driver pacing, with one Sleep before capture per base candidate. The original Beta 6 mapping from Reflex to Async has been removed.
- Active Reflex does not add an Async／StepTimer limit or a Front Edge input deadline for the same target. Lower profile and capture caps merge into the driver target.
- The panel separates requested mode, actual base limiter, low-latency request and query result, and FG output scheduling. A successful low-latency Off query no longer forces an accepted driver frame limit back to Async.
- API failures clear the application driver frame limit before Async takes over at the same target. Failed cleanup reports the problem and stops the current scaling session; restart Magpie before retrying.
- Defaults remain Front Edge Sync at 60 FPS, preserving existing settings. Target 0 is automatic; manual targets span 1–1000 FPS. Automatic base FPS is refresh rate divided by the FG multiplier. Turning frame sync off retains other existing caps and FG low-latency handling.

### Effects

- DLSSFG retains generated／real FIFO order, separate output intervals, slot ownership, resource backpressure and fences. 2×／3×／4× share the same base-FPS definition.
- Explicit Async, Front Edge and XeLL paths remain available. This build includes Beta 6 fullscreen parameter handoff, FIFO retry timing and resource reuse fixes.

## Which file should I download?

Both previous experimental users and new users should use the full `Magpie-Experimental-x64 v0.6.7-beta6-fix1.zip`. Fully exit Magpie from the tray, extract the complete archive into a new folder, then run its `Magpie.exe`. Keep the EXE and `resources.pri` from the same build together.

Normal settings prefer `%LOCALAPPDATA%\Magpie\config\v4e\config.json`. Portable users can copy existing settings to `config\v4e\config.json` in the new folder and retain the originals for rollback. No new optional component is required.

## Validation and limitations

Implementation and headless regressions are complete; actual NVIDIA driver pacing, HDR／capture combinations and performance gains remain unverified. Reflex requires a supported NVIDIA DXGI path, with effects and presentation on the same GPU; the current async marker interface requires R565+. Targets cap base input; slower sources or GPUs can produce lower actual rates. See the included `FRAME_SYNC_GUIDE.md` for mode details and troubleshooting.
