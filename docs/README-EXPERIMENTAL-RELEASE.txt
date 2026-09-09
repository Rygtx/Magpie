Magpie Experimental v0.6.7-beta2 x64

安装或升级
从托盘完全退出 Magpie，将 Magpie-Experimental-x64 v0.6.7-beta2.zip 完整解压到新目录，再运行其中的 Magpie.exe。
普通配置优先读取 %LOCALAPPDATA%\Magpie\config\v4e\config.json；没有 v4e 时自动导入 v4，后续只保存到 v4e。
便携用户可将原配置复制到新程序目录内的 config\v4e\config.json，保留原文件用于回退。
旧效果组的光流设置在首次迁移时统一关闭一次，此后可在参数面板重新选择。
效果组出现失效项时，恢复对应效果文件，或移除此项并添加替代效果。

本次更新见 RELEASE-NOTES.md；帧同步用法见 FRAME_SYNC_GUIDE.md。
请保留许可证、THIRD-PARTY-NOTICES.md 和 build-manifest.json。

English

Install or upgrade
Fully exit Magpie from the system tray, extract Magpie-Experimental-x64 v0.6.7-beta2.zip completely into a new folder, then run its Magpie.exe.
Normal settings prefer %LOCALAPPDATA%\Magpie\config\v4e\config.json; if v4e is absent, v4 is imported automatically and subsequent saves use only v4e.
Portable users can copy their existing settings to config\v4e\config.json in the new program folder and retain the original for rollback.
Existing groups have optical flow disabled once during migration; select a provider again in the parameter panel if needed.
For an invalid effect, restore its file or remove the item and add a replacement.

See RELEASE-NOTES.md for this update and FRAME_SYNC_GUIDE.md for frame synchronization.
Retain the licenses, THIRD-PARTY-NOTICES.md and build-manifest.json.
