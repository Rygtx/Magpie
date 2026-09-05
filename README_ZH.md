<p align="center"><img src="./src/Magpie/Icons/SVG/Magpie Icon Full Disabled.svg" width="150" height="150" alt="Magpie"></p>
<h1 align="center">Magpie Experimental</h1>

🌍 [English](./README.md) | **简体中文**

这是 [Blinue/Magpie](https://github.com/Blinue/Magpie) 的非官方实验分支，提供基于窗口捕获画面的 DLSS、DLSS Frame Generation、DLSSNR、XeSS、FSR 和 RTX Video 效果。不代表 Magpie 官方，也不由上游项目提供支持。

## 版本与安装

0.6.5 已进入本地发布包准备阶段，完整说明见 [0.6.5 Release Note](docs/RELEASE_NOTES_v0.6.5-experimental.md)。已公开版本以 [GitHub Releases](https://github.com/SAOG0721/Magpie/releases) 为准。

完全退出 Magpie，将 `Magpie-Experimental-x64.zip` 完整解压到新目录后运行 `Magpie.exe`。不要只替换 EXE，也不要复制旧效果目录或已经移除的深度组件。普通设置位于 `%LOCALAPPDATA%\Magpie\config\v4\config.json`；便携设置位于程序目录的 `config\config.json`。升级前备份配置和旧安装。

## 效果组与参数

“缩放模式”现称为“效果组”，一个组可以组合多个效果器。新配置默认包含 Lanczos、FSR、RTX Video VSR Ultra、DLSSFG、XeSSFG、DLSSNR，默认选择 Lanczos。原有自定义组保持保留。

可从效果组页面导入 [可选预设](presets/ScalingModes-v0.6.5-experimental.json)，追加 DLSSFG、XeSSFG、DLSSNR。重置功能会恢复程序默认组；导入只追加。

参数修改自动保存。工具栏参数页按当前会话标记“实时”“重启”“自动重启”；后两者的区别是手动应用与编辑结束后自动重建。DLSSNR 核心和上游图像的原实时参数会触发完整停用，等待 500 毫秒再重新启用；残差合成参数保留原有实时行为。连续编辑合并处理，手动停用会取消等待中的重启。

在 170 毫秒内双击滑条可恢复效果器自身默认值。参数支持分组、下拉选项以及简体／繁体中文显示。保存失败或目标值尚未应用时，参数页会区分实际值与修改值。

## 工具栏与帧节奏

| 功能 | 默认快捷键 |
| --- | --- |
| 性能监测 | Alt+Shift+P |
| 效果参数 | Alt+Shift+E |
| 截屏 | Alt+Shift+S |
| 固定工具栏 | Alt+Shift+F |
| 对比 | Alt+Shift+C |

快捷键可在主页收起项中修改。“对比”显示原图时效果继续处理；角标停留 2 秒后以 500 毫秒淡出。启用 FG 后帧率支持“输出／真实帧”显示。

Front Edge Sync 默认开启，目标为 60 FPS，控制 FG 之前的基础帧节奏；目标程序也需要配合限帧。可能增加延迟。FrameRate Filter 默认跟随该设置，关闭同步后可自定义。完整解释见 [帧同步说明](docs/FRAME_SYNC_GUIDE.md)。VRR 当前隐藏并停用，HDR 尚未实现。

## 光流与兼容性

DLSS SR、FSR 2/3/4 和 XeSS SR 各提供一个入口，支持不使用／AMD OF／NVOF。独立 Zero MV、Optical Flow 和 metadata-only jitter 入口已合并，旧配置自动迁移。DLSSNR、DLSSFG 使用 NVOF；XeSSFG x2 可使用 AMD OF／NVOF，XeSS Multi-FG 当前仅支持 AMD OF 或不使用光流。

多个消费者共享实际申请中的同一提供者：NVOF 优先，再 AMD OF，使用选定来源实际申请中的较高档位。NVOF 最高质量为 2×2 Slow，开销可能很高；均衡档继续标注推荐。

Magpie 没有游戏引擎原生深度、运动矢量、曝光或 UI 分离信息。需要深度的接口使用全零纹理，Motion 从捕获颜色估算，不能等同于游戏原生 DLSS／FSR／XeSS 集成。同一组仅使用一种 FG，避免与 Smooth Motion 等其他补帧叠加。

## 排错与开发

优先查看主页“最近一次问题”的详情与日志。Release 配套提供 NGX OTA 开关和 DLSSNR DLL 选项；用途与限制见 Release Note。源码默认关闭可选专有后端；本机 SDK／运行时通过不入库的 `src/BuildOptions.props.user` 配置。

- [依赖、第三方声明与构建边界](docs/THIRD_PARTY_AND_REDISTRIBUTION.md)
- [实验开发文档](docs/experimental/README.md)
- [构建和发布脚本](scripts/Build-Release.ps1)
- [MagpieFX 效果格式](docs/MagpieFX.md)

源码沿用 [GPLv3](LICENSE)。第三方运行时、SDK、模型及本地配置不进入源码仓库。
