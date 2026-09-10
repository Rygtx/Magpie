#include "pch.h"
#include "OverlayDrawer.h"
#include "ScalingWindow.h"
#include "Renderer.h"
#include "CursorManager.h"
#include "Logger.h"
#include <imgui_internal.h>

namespace Magpie {

OverlayDrawer::~OverlayDrawer() noexcept {
	_parameterInputTransition = true;
	if (_hwndParameterInput) DestroyWindow(_hwndParameterInput);
}

bool OverlayDrawer::_HasParameterForeground() const noexcept {
	auto& scaling = ScalingWindow::Get();
	const HWND foreground = GetForegroundWindow();
	return foreground && (foreground == scaling.SrcTracker().Handle() || foreground == scaling.Handle() ||
		foreground == _hwndParameterInput);
}

bool OverlayDrawer::_EnsureParameterInputHost() noexcept {
	if (!_hwndParameterInput) {
		static const ATOM windowClass = [] {
			WNDCLASSEXW wc{ sizeof(wc) };
			wc.lpfnWndProc = _ParameterInputWndProc;
			wc.hInstance = GetModuleHandle(nullptr);
			wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wc.lpszClassName = L"Magpie_ParameterInputHost";
			return RegisterClassExW(&wc);
		}();
		if (windowClass) {
			// No redirection surface: fully transparent visuals, rectangular USER32
			// hit testing. Alpha-zero layered windows would pass the first click through.
			_hwndParameterInput = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP,
				MAKEINTATOM(windowClass), L"Magpie parameters", WS_POPUP,
				0, 0, 1, 1, ScalingWindow::Get().Handle(), nullptr, GetModuleHandle(nullptr), this);
		}
	}
	if (!_hwndParameterInput) {
		Logger::Get().Win32Error("Create parameter input host failed");
		return false;
	}
	return true;
}

bool OverlayDrawer::_BeginParameterInput() noexcept {
	// An asynchronous restart must never activate over an unrelated application.
	if (!_HasParameterForeground()) return false;
	_parameterInputTransition = true;
	if (!_EnsureParameterInputHost()) {
		_parameterInputTransition = false;
		return false;
	}
	auto& scaling = ScalingWindow::Get();
	_parameterPanelState = ParameterPanelState::Edit;
	_pendingParameterPanelState = ParameterPanelState::Edit;
	_parameterHeldButtons = 0;
	_returnClickPending = _escapePending = _parameterResumeClickPending = false;
	_parameterHeldKeys.fill(false);
	ClearStates();
	const RECT& rect = scaling.RendererRect();
	SetWindowPos(_hwndParameterInput, HWND_TOPMOST, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_SHOWWINDOW);
	const HWND currentForeground = GetForegroundWindow();
	if (currentForeground == scaling.SrcTracker().Handle() || currentForeground == scaling.Handle() ||
		currentForeground == _hwndParameterInput) SetForegroundWindow(_hwndParameterInput);
	const bool activated = GetForegroundWindow() == _hwndParameterInput;
	if (activated) {
		SetFocus(_hwndParameterInput);
		ClipCursor(nullptr);
		_imguiImpl.ParameterEditing(true);
		for (int key : { VK_CONTROL, VK_SHIFT, VK_MENU, VK_LWIN, VK_RWIN }) {
			if (GetAsyncKeyState(key) & 0x8000) {
				_parameterHeldKeys[key] = true;
				_imguiImpl.MessageHandler(WM_KEYDOWN, key, 0);
			}
		}
		scaling.CursorManager().Update();
		SetTimer(_hwndParameterInput, 1, 16, nullptr);
	} else {
		ShowWindow(_hwndParameterInput, SW_HIDE);
		_parameterPanelState = ParameterPanelState::Preview;
	}
	_parameterInputTransition = false;
	return activated;
}

void OverlayDrawer::_EndParameterInput(bool returnFocus) noexcept {
	_parameterInputTransition = true;
	const bool ownedFocus = _hwndParameterInput && GetForegroundWindow() == _hwndParameterInput;
	_imguiImpl.ParameterEditing(false);
	ClearStates();
	if (_hwndParameterInput) {
		KillTimer(_hwndParameterInput, 1);
		if (GetCapture() == _hwndParameterInput) ReleaseCapture();
		if (ownedFocus && GetForegroundWindow() == _hwndParameterInput) ClipCursor(nullptr);
		if (ownedFocus && returnFocus && GetForegroundWindow() == _hwndParameterInput &&
			IsWindow(ScalingWindow::Get().SrcTracker().Handle())) {
			auto& scaling = ScalingWindow::Get();
			_parameterFocusFailed = !scaling.SrcTracker().SetFocus();
			if (_parameterFocusFailed) scaling.ShowToast(scaling.GetLocalizedString(L"Overlay_Parameters_FocusFailed"));
		}
		ShowWindow(_hwndParameterInput, SW_HIDE);
	}
	_parameterHeldButtons = 0;
	_parameterHeldKeys.fill(false);
	_returnClickPending = _escapePending = _parameterResumeClickPending = false;
	_parameterInputTransition = false;
}

void OverlayDrawer::_UpdateParameterPreviewHost() noexcept {
	if (_parameterInputTransition || IsEditingParameters() || _parameterResumeClickPending) return;
	const auto rect = _imguiImpl.PresentedParameterRect();
	if (_parameterPanelState != ParameterPanelState::Preview || !_isEffectParametersVisible ||
		!_HasParameterForeground() || !rect) {
		if (_hwndParameterInput) ShowWindow(_hwndParameterInput, SW_HIDE);
		if (!_HasParameterForeground()) _previewEscapeCanClose = _previewClosePending = false;
		return;
	}
	auto& scaling = ScalingWindow::Get();
	const RECT& dest = scaling.Renderer().DestRect();
	const RECT& render = scaling.RendererRect();
	const RECT target{
		std::max(render.left, dest.left + LONG(std::floor(rect->x))),
		std::max(render.top, dest.top + LONG(std::floor(rect->y))),
		std::min(render.right, dest.left + LONG(std::ceil(rect->z))),
		std::min(render.bottom, dest.top + LONG(std::ceil(rect->w)))
	};
	if (target.left >= target.right || target.top >= target.bottom) {
		if (_hwndParameterInput) ShowWindow(_hwndParameterInput, SW_HIDE);
		return;
	}
	if (!_EnsureParameterInputHost()) return;
	RECT current{};
	GetWindowRect(_hwndParameterInput, &current);
	if (!EqualRect(&current, &target) || !IsWindowVisible(_hwndParameterInput)) {
		// Do not activate during preview. Only a user click may activate this
		// bounded window; game-area input stays outside its native hit region.
		SetWindowPos(_hwndParameterInput, HWND_TOPMOST, target.left, target.top,
			target.right - target.left, target.bottom - target.top, SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}
}

bool OverlayDrawer::HandleParameterPreviewEscape(WPARAM message, const KBDLLHOOKSTRUCT& key) noexcept {
	if (key.vkCode != VK_ESCAPE) return false;
	const bool down = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
	const bool up = message == WM_KEYUP || message == WM_SYSKEYUP;
	if (!down && !up) return false;
	if (_previewEscapeOwned) {
		if (up) {
			_previewEscapeOwned = false;
			_previewClosePending = std::exchange(_previewEscapeCanClose, false) &&
				_parameterPanelState == ParameterPanelState::Preview && _HasParameterForeground();
			if (_previewClosePending) {
				_overlayDirty = true;
				PostMessage(ScalingWindow::Get().Handle(), WM_NULL, 0, 0);
			}
		}
		return true; // Own repeats and release only after owning the initial press.
	}
	if (!down || _parameterPanelState != ParameterPanelState::Preview ||
		!_isEffectParametersVisible || !_HasParameterForeground() || (key.flags & LLKHF_ALTDOWN)) return false;
	for (int vk : { VK_ESCAPE, VK_CONTROL, VK_SHIFT, VK_MENU, VK_LWIN, VK_RWIN }) {
		if (GetAsyncKeyState(vk) & 0x8000) return false;
	}
	_previewEscapeOwned = _previewEscapeCanClose = true;
	return true;
}

void OverlayDrawer::_ToggleParameterPanel() noexcept {
	_SetParameterPanelState(_isEffectParametersVisible ? ParameterPanelState::Closed : ParameterPanelState::Edit);
}

void OverlayDrawer::_SetParameterPanelState(ParameterPanelState state, bool returnFocus) noexcept {
	_previewEscapeCanClose = _previewClosePending = false;
	if (state == ParameterPanelState::Edit) {
		_isEffectParametersVisible = true;
		if (!IsEditingParameters()) {
			_parameterFocusFailed = !_BeginParameterInput();
			if (_parameterFocusFailed) _parameterPanelState = ParameterPanelState::Preview;
		}
	} else if (IsEditingParameters() && returnFocus) {
		// Keep the host until every physical edge belongs to a complete pair.
		_pendingParameterPanelState = state;
		_FinishParameterInput();
	} else {
		_EndParameterInput(returnFocus);
		_parameterPanelState = state;
		_isEffectParametersVisible = state != ParameterPanelState::Closed;
	}
	_overlayDirty = true;
	_UpdateParameterPreviewHost();
}

void OverlayDrawer::_FinishParameterInput() noexcept {
	if (_pendingParameterPanelState == ParameterPanelState::Edit || HasHeldParameterInput()) return;

	// Let ImGui consume a queued release before clearing its active item.
	// Hover moves may keep arriving while the user returns to the game. Only
	// complete control edges need presentation; movement must not postpone focus.
	if (_imguiImpl.HasCriticalInput()) return;
	_EndParameterInput(true);
	_parameterPanelState = _pendingParameterPanelState;
	_isEffectParametersVisible = _parameterPanelState != ParameterPanelState::Closed;
	_UpdateParameterPreviewHost();
}

bool OverlayDrawer::HasHeldParameterInput() const noexcept {
	if (_previewEscapeOwned || _parameterResumeClickPending) return true;
	if (!IsEditingParameters()) return false;
	if (_parameterHeldButtons || std::ranges::any_of(_parameterHeldKeys, [](bool held) { return held; })) return true;
	// Shortcut callbacks can arrive before the invoking modifier messages.
	for (int key : {VK_CONTROL, VK_SHIFT, VK_MENU, VK_LWIN, VK_RWIN}) {
		if (GetAsyncKeyState(key) & 0x8000) return true;
	}
	return false;
}

void OverlayDrawer::ReleaseParameterInput() noexcept {
	_previewEscapeCanClose = _previewClosePending = false;
	const bool editing = IsEditingParameters();
	_EndParameterInput(true);
	if (editing) _parameterPanelState = ParameterPanelState::Preview;
}

void OverlayDrawer::SuspendParameterInput() noexcept {
	if (!IsEditingParameters() || _parameterInputTransition) return;
	_SetParameterPanelState(ParameterPanelState::Preview, false);
}

void OverlayDrawer::UpdateParameterInputHost() noexcept {
	if (std::exchange(_previewClosePending, false) &&
		_parameterPanelState == ParameterPanelState::Preview && _HasParameterForeground())
		_SetParameterPanelState(ParameterPanelState::Closed, false);
	_UpdateParameterPreviewHost();
	if (!IsEditingParameters() || _parameterInputTransition) return;
	if (GetForegroundWindow() != _hwndParameterInput) {
		SuspendParameterInput();
		return;
	}
	RECT current{};
	GetWindowRect(_hwndParameterInput, &current);
	const RECT& rect = ScalingWindow::Get().RendererRect();
	if (!EqualRect(&rect, &current)) SetWindowPos(_hwndParameterInput, nullptr,
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		SWP_NOACTIVATE | SWP_NOZORDER);
	_FinishParameterInput();
}

LRESULT CALLBACK OverlayDrawer::_ParameterInputWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	if (msg == WM_NCCREATE) {
		auto self = static_cast<OverlayDrawer*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
	}
	auto self = reinterpret_cast<OverlayDrawer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (!self) return DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCDESTROY) {
		self->_hwndParameterInput = nullptr;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	if (msg == WM_NCHITTEST) return HTCLIENT;
	if (msg == WM_MOUSEACTIVATE) return MA_ACTIVATE;
	if (msg == WM_ERASEBKGND) return 1;
	if (msg == WM_PAINT) { PAINTSTRUCT ps{}; BeginPaint(hwnd, &ps); EndPaint(hwnd, &ps); return 0; }
	if (msg == WM_SETCURSOR) { SetCursor(LoadCursor(nullptr, IDC_ARROW)); return TRUE; }
	if (self->_parameterInputTransition) return DefWindowProc(hwnd, msg, wParam, lParam);
	int button = -1;
	bool down = false;
	switch (msg) {
	case WM_LBUTTONDOWN: down = true; [[fallthrough]];
	case WM_LBUTTONUP: button = 0; break;
	case WM_RBUTTONDOWN: down = true; [[fallthrough]];
	case WM_RBUTTONUP: button = 1; break;
	case WM_MBUTTONDOWN: down = true; [[fallthrough]];
	case WM_MBUTTONUP: button = 2; break;
	case WM_XBUTTONDOWN: down = true; [[fallthrough]];
	case WM_XBUTTONUP: button = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 3 : 4; break;
	}
	if (button >= 0 && down && self->_parameterPanelState == ParameterPanelState::Preview &&
		!self->_parameterResumeClickPending) {
		// Preserve the event's position before activation resizes the host and
		// releases source cursor mapping. The same DOWN must reach the control.
		POINT point{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		ClientToScreen(hwnd, &point);
		self->_SetParameterPanelState(ParameterPanelState::Edit);
		self->_parameterResumeClickPending = !self->IsEditingParameters();
		self->_parameterHeldButtons = 1u << button;
		SetCapture(hwnd);
		if (self->IsEditingParameters()) {
			self->_imguiImpl.MessageHandler(msg, wParam, lParam, point);
			self->_overlayDirty = true;
		}
		return msg == WM_XBUTTONDOWN ? TRUE : 0;
	}
	if (self->_parameterResumeClickPending && button >= 0) {
		if (down) self->_parameterHeldButtons |= 1u << button;
		else self->_parameterHeldButtons &= ~(1u << button);
		if (!self->_parameterHeldButtons) {
			self->_parameterResumeClickPending = false;
			if (GetCapture() == hwnd) ReleaseCapture();
			self->_FinishParameterInput();
			self->_UpdateParameterPreviewHost();
		}
		return msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP ? TRUE : 0;
	}
	if (msg == WM_CAPTURECHANGED && reinterpret_cast<HWND>(lParam) != hwnd) {
		self->_parameterHeldButtons = 0;
		self->_parameterResumeClickPending = false;
	}
	if (!self->IsEditingParameters()) return DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_KILLFOCUS || (msg == WM_ACTIVATEAPP && !wParam)) {
		self->SuspendParameterInput();
		return 0;
	}
	if (msg == WM_TIMER) {
		self->UpdateParameterInputHost();
		// Rendering remains in the outer scaling loop, never in WndProc.
		return 0;
	}
	if (msg == WM_CLOSE) { self->_SetParameterPanelState(ParameterPanelState::Closed); return 0; }
	if (self->_parameterResumeClickPending &&
		(msg == WM_MOUSEMOVE || msg == WM_MOUSEWHEEL || msg == WM_MOUSEHWHEEL)) return 0;
	if (button >= 0) {
		if (down) {
			if (!self->_parameterHeldButtons && !self->_imguiImpl.OwnsPointerAtCursor()) {
				self->_returnClickPending = true;
				self->_pendingParameterPanelState = ParameterPanelState::Preview;
			}
			self->_parameterHeldButtons |= 1u << button;
			SetCapture(hwnd);
		} else self->_parameterHeldButtons &= ~(1u << button);
		if (!self->_returnClickPending) self->MessageHandler(msg, wParam, lParam);
		if (!self->_parameterHeldButtons && GetCapture() == hwnd) ReleaseCapture();
		self->_FinishParameterInput();
		return msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP ? TRUE : 0;
	}
	if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN || msg == WM_KEYUP || msg == WM_SYSKEYUP) {
		const bool keyDown = msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN;
		if (wParam < 256) self->_parameterHeldKeys[wParam] = keyDown;
		if (wParam == VK_ESCAPE) {
			if (keyDown && !(lParam & (1LL << 30))) self->_escapePending = true;
			if (!keyDown && std::exchange(self->_escapePending, false)) {
				if (!self->_imguiImpl.DismissParameterPopup()) self->_SetParameterPanelState(ParameterPanelState::Preview);
				self->_overlayDirty = true;
			}
			return 0;
		}
		self->MessageHandler(msg, wParam, lParam);
		// Preserve system switching keys (Alt+Tab, Alt+F4), never forward to the game.
		if (msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP) return DefWindowProc(hwnd, msg, wParam, lParam);
		return 0;
	}
	if (msg == WM_CHAR || msg == WM_MOUSEMOVE || msg == WM_MOUSEWHEEL || msg == WM_MOUSEHWHEEL || msg == WM_CANCELMODE || msg == WM_CAPTURECHANGED) {
		self->MessageHandler(msg, wParam, lParam);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

}
