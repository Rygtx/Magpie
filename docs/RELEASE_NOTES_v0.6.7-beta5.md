# Magpie Experimental v0.6.7 Beta 5

保留 Beta 4 的全部功能，本次更新：

- 预览面板的激活点击直接交给参数控件：单击开关、打开下拉菜单或按住滑条拖动，无需再点一次。
- 进入编辑前恢复上一帧面板的输入标记和有效命中范围，保留点击时的屏幕坐标，避免状态切换清空命中信息或窗口位置变化丢失首击。
- 保留 `Esc` 的“编辑 → 预览 → 关闭”逐级退出，以及参数按钮／快捷键的面板开关行为。

自动输入回归覆盖首击拖动滑条、切换开关、打开下拉菜单及子区域点击；实际应用中的预览激活和游戏兼容性仍需实测确认。

---

# Magpie Experimental v0.6.7 Beta 5

Includes all Beta 4 features, with the following updates:

- The click that activates the preview panel also operates the parameter control: toggle a checkbox, open a dropdown or start dragging a slider without clicking again.
- Restores the panel's previous-frame input flags and valid hit bounds before editing, and preserves the original screen coordinates so activation does not lose the first press when input state is cleared or the host moves.
- Keeps Escape's Edit → Preview → Closed sequence and the parameter toolbar button/shortcut's open-close behavior.

Automated input regressions cover first-click slider dragging, checkbox toggling, dropdown opening and child-region clicks. Preview activation in the full application and game compatibility still require hands-on validation.
