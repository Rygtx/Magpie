# 开发与分发准备 / Development and Distribution Preparation

## 0.6.9 Beta1 本地整合

七个 069 功能分支已合入本地 `experimental`：快捷键清除、xBR、DLSSNR 开销优化、工具栏拖拽、重复帧过滤优化、DLSSNR 细节控制、光标刷新与主页常驻选项。

[完整中英更新说明](RELEASE_NOTES_v0.6.9-beta1.md) 与 [主线整合核对](experimental/reviews/20261001-v0.6.9-beta1-integration.md) 记录范围和验证边界。运行版本为 `0.6.9-beta1`；本次本地编译部署不修改公开更新入口。构建／部署结果及准确提交以本地版本目录的清单为准，真实 UI／GPU／画质／性能验收仍需实测。

## 0.6.8 Draft 准备

`068` 已合并到开发主线 `experimental`。本轮准备 `v0.6.8-experimental` 的源码、完整包与 GitHub Draft，尚未公开发布；[完整中英更新说明](RELEASE_NOTES_v0.6.8-experimental.md) 等待维护者人工审核。开发和测试记录见 [分发准备记录](experimental/reviews/20260914-v0.6.8-release-preparation.md)。

当前公开版本仍为 [Magpie Experimental v0.6.7](https://github.com/SAOG0721/Magpie/releases/tag/untagged-1a7d59dfa5a6fd93c888)。Draft 阶段保留 `version.json` 的 0.6.7 版本，不提前宣告 0.6.8 可公开下载；0.6.8 的构建版本、目标提交和附件以分发清单记录。收到明确发布指令后再更新公开版本入口。

---

## 0.6.9 Beta1 local integration

All seven 0.6.9 feature branches are merged into local `experimental`: shortcut clearing, xBR, DLSSNR overhead optimization, toolbar dragging, duplicate-frame optimization, DLSSNR detail controls, and cursor refresh/permanent Home options.

The [full Chinese/English notes](RELEASE_NOTES_v0.6.9-beta1.md) and [integration audit](experimental/reviews/20261001-v0.6.9-beta1-integration.md) describe scope and validation boundaries. The runtime version is `0.6.9-beta1`; this local build/deployment does not change the public update entry. The local version-directory manifests record build/deployment results and the exact commit. Native UI, GPU, image quality and performance still require testing.

## 0.6.8 draft preparation

The `068` branch has been merged into the `experimental` development mainline. Source, distribution packages and a GitHub draft are being prepared for `v0.6.8-experimental`; it is not publicly released. The [full Chinese/English notes](RELEASE_NOTES_v0.6.8-experimental.md) await manual review. Development and validation records remain in the [preparation record](experimental/reviews/20260914-v0.6.8-release-preparation.md).

The current public release remains [Magpie Experimental v0.6.7](https://github.com/SAOG0721/Magpie/releases/tag/untagged-1a7d59dfa5a6fd93c888). During draft preparation, `version.json` stays at 0.6.7 rather than advertising an unavailable 0.6.8 download. The 0.6.8 build version, target commit and assets are recorded in its distribution manifest. Update the public-version entry only after explicit publication authorization.
