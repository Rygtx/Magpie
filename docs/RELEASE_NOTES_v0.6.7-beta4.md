# Magpie Experimental v0.6.7 Beta 4

保留 Beta 3 的全部功能，本次更新：

- 实时参数面板支持常驻预览与编辑切换：编辑时释放鼠标；单击游戏区域后返回游戏，本次点击不会执行游戏操作。
- 默认 `Alt+Shift+E` 改为“编辑参数／返回游戏”，可在首页配置。预览保持显示并鼠标穿透，可通过工具栏重新编辑或关闭。
- `Esc` 优先关闭下拉菜单／数值输入，再返回游戏；重新启用效果组后恢复面板位置和交互状态，切到其他应用时尊重新焦点。
- 主动停止与重建会等待参数层的本次输入结束，避免松开事件落入游戏。不同游戏的失焦、锁鼠与后台原始输入表现仍需分别实测。

- 修复删除效果组时可能闪退的问题。
- 效果组重命名增加重复检查，名称不区分大小写和首尾空格。
- 升级时发现旧配置中的同名效果组，会保留全部内容并提示逐个改名。
- 新建、复制和导入效果组时自动避免重名，导入的重复名称会添加序号。

---

# Magpie Experimental v0.6.7 Beta 4

Includes all Beta 3 features, with the following updates:

- Live parameters can remain visible as a click-through preview. Editing releases the cursor; clicking the game returns control after consuming that complete click.
- `Alt+Shift+E` now toggles editing and returning to the game, with a configurable shortcut. The toolbar can resume editing or close the preview.
- Escape closes temporary controls first, then returns to the game. Effect-group restarts restore panel geometry and interaction state while respecting a switch to another app.
- Requested stops and restarts wait for outstanding parameter input to finish. Game-specific focus, confinement and background raw-input compatibility requires separate testing.

- Fixed a possible crash when deleting an effect group.
- Renaming now checks for duplicate names, ignoring capitalization and surrounding spaces.
- Existing groups with duplicate names are preserved during upgrades and marked for renaming.
- New, copied and imported groups receive unique names, with numbered suffixes added to duplicate import names.
