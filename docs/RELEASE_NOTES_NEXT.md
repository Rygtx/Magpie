# 开发与分发准备 / Development and Distribution Preparation

## 0.6.9 开发：快捷键清除与“未设置”

工具栏快捷键展开项支持在编辑按钮右侧点击垃圾桶清除；全部八项快捷键的编辑弹窗提供“保存｜清除｜取消”。清除后显示“未设置”，绑定立即解除并跨重启保留；缺失的旧配置字段仍沿用默认值，重新设置仍执行合法性和冲突检查。

功能与自动回归已完成，真实 UI／系统热键验收仍待执行。此条目不是发布公告；069 beta1 的统一编译部署由单独会话负责。范围及验证边界见 [快捷键清除 TODO](experimental/todos/20261001-v0.6.9-shortcut-clear-TODO.md)。

## 0.6.8 Draft 准备

`068` 已合并到开发主线 `experimental`。本轮准备 `v0.6.8-experimental` 的源码、完整包与 GitHub Draft，尚未公开发布；[完整中英更新说明](RELEASE_NOTES_v0.6.8-experimental.md) 等待维护者人工审核。开发和测试记录见 [分发准备记录](experimental/reviews/20260914-v0.6.8-release-preparation.md)。

当前公开版本仍为 [Magpie Experimental v0.6.7](https://github.com/SAOG0721/Magpie/releases/tag/untagged-1a7d59dfa5a6fd93c888)。Draft 阶段保留 `version.json` 的 0.6.7 版本，不提前宣告 0.6.8 可公开下载；0.6.8 的构建版本、目标提交和附件以分发清单记录。收到明确发布指令后再更新公开版本入口。

---

## 0.6.9 development: clear shortcuts and the “Not set” state

The six expanded toolbar shortcut rows now offer a trash button beside the editor. All eight shortcut dialogs provide Save, Clear and Cancel. Clearing immediately removes the binding, displays “Not set” and survives restarts; missing legacy fields still receive their defaults, and rebinding retains validation and conflict checks.

Implementation and automated regression checks are complete; native UI and actual system-hotkey acceptance remain pending. This is not a release announcement. The unified 0.6.9 beta1 build and deployment belong to a separate session. See the [shortcut clearing TODO](experimental/todos/20261001-v0.6.9-shortcut-clear-TODO.md) for scope and validation boundaries.

## 0.6.8 draft preparation

The `068` branch has been merged into the `experimental` development mainline. Source, distribution packages and a GitHub draft are being prepared for `v0.6.8-experimental`; it is not publicly released. The [full Chinese/English notes](RELEASE_NOTES_v0.6.8-experimental.md) await manual review. Development and validation records remain in the [preparation record](experimental/reviews/20260914-v0.6.8-release-preparation.md).

The current public release remains [Magpie Experimental v0.6.7](https://github.com/SAOG0721/Magpie/releases/tag/untagged-1a7d59dfa5a6fd93c888). During draft preparation, `version.json` stays at 0.6.7 rather than advertising an unavailable 0.6.8 download. The 0.6.8 build version, target commit and assets are recorded in its distribution manifest. Update the public-version entry only after explicit publication authorization.
