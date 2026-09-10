"""Generate a no-GPU harness from the production input host and ImGui backend.

Win32 focus/capture are deterministic doubles; Dear ImGui and all input queue,
hit-test, host-message and transition methods come from the checked-out source.
Run parameter_input_host_prototype.cpp separately for actual USER32/DWM behavior.
"""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[2]
core = root / 'src/Magpie.Core'
def body(name):
    return '\n'.join(line for line in (core / name).read_text(encoding='utf-8-sig').splitlines()
                     if not line.startswith(('#include', '#pragma once')))

host_header = (core / 'OverlayDrawer.h').read_text(encoding='utf-8-sig')
host_members = host_header.split('private:', 1)[1].split('\tbool _BuildFonts()', 1)[0]
enum = re.search(r'enum class ParameterPanelState[^;]+;', (core / 'include/ScalingOptions.h').read_text(encoding='utf-8-sig')).group()

prefix = r'''
#define UNICODE
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <deque>
#include <iostream>
#include <limits>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include "OverlayWindowGeometry.h"
namespace phmap { template<class K, class V> using flat_hash_map = std::unordered_map<K,V>; }
namespace fmt { template<class... T> std::string format(const char* s, T&&...) { return s; } }
static HWND game = (HWND)1, scaling = (HWND)2, foreground = game, capture = nullptr, inputHost = nullptr;
static POINT cursor{100,100};
static RECT hostRect{}, clipRect{};
static std::array<bool,256> keys{};
static WNDPROC hostProc;
static LONG_PTR hostUserData;
static bool denyFocus, visibleHost;
static int focusAttempts, gameEdges, updates;
static SHORT FakeAsync(int key) { return keys[key] ? SHORT(0x8000) : 0; }
static BOOL FakeCursor(POINT* p) { *p = cursor; return TRUE; }
static HWND FakeForeground() { return foreground; }
static BOOL FakeSetForeground(HWND hwnd) {
    ++focusAttempts;
    if (denyFocus) return FALSE;
    const HWND old = foreground; foreground = hwnd;
    if (old == inputHost && old != hwnd) hostProc(old, WM_KILLFOCUS, (WPARAM)hwnd, 0);
    return TRUE;
}
static HWND FakeCapture() { return capture; }
static HWND FakeSetCapture(HWND hwnd) {
    HWND old = capture; capture = hwnd;
    if (old == inputHost && old != hwnd) hostProc(old, WM_CAPTURECHANGED, 0, (LPARAM)hwnd);
    return old;
}
static BOOL FakeRelease() { FakeSetCapture(nullptr); return TRUE; }
static BOOL FakeClip(const RECT* r) { clipRect = r ? *r : RECT{}; return TRUE; }
static ATOM FakeRegister(const WNDCLASSEXW* wc) { hostProc = wc->lpfnWndProc; return 1; }
static HWND FakeCreate(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID user) {
    inputHost = (HWND)3; CREATESTRUCT cs{}; cs.lpCreateParams = user;
    hostProc(inputHost, WM_NCCREATE, 0, (LPARAM)&cs); return inputHost;
}
static LONG_PTR FakeSetLong(HWND, int, LONG_PTR data) { return std::exchange(hostUserData,data); }
static LONG_PTR FakeGetLong(HWND, int) { return hostUserData; }
static BOOL FakePosition(HWND, HWND, int x, int y, int w, int h, UINT flags) {
    hostRect = {x,y,x+w,y+h}; if (flags & SWP_SHOWWINDOW) visibleHost = true; return TRUE;
}
static BOOL FakeRect(HWND, RECT* r) { *r = hostRect; return TRUE; }
static BOOL FakeShow(HWND, int command) { visibleHost = command != SW_HIDE; return TRUE; }
static BOOL FakeDestroy(HWND hwnd) { hostProc(hwnd,WM_NCDESTROY,0,0); inputHost=nullptr; visibleHost=false; return TRUE; }
static HWND FakeFocus(HWND hwnd) { return hwnd; }
static UINT_PTR FakeTimer(HWND, UINT_PTR id, UINT, TIMERPROC) { return id; }
static BOOL FakeKillTimer(HWND, UINT_PTR) { return TRUE; }
static LRESULT FakeDef(HWND, UINT, WPARAM, LPARAM) { return 0; }
static LONG FakeMessageTime() { static LONG time; return ++time; }
#define GetAsyncKeyState FakeAsync
#define GetKeyState FakeAsync
#define GetCursorPos FakeCursor
#define GetForegroundWindow FakeForeground
#define SetForegroundWindow FakeSetForeground
#define GetCapture FakeCapture
#define SetCapture FakeSetCapture
#define ReleaseCapture FakeRelease
#define ClipCursor FakeClip
#define RegisterClassExW FakeRegister
#define CreateWindowExW FakeCreate
#define SetWindowLongPtrW FakeSetLong
#define GetWindowLongPtrW FakeGetLong
namespace ImGui { inline void FakePosition(ImGuiWindow* w, ImVec2 p) { SetWindowPos(w,p); } }
#define SetWindowPos FakePosition
#define GetWindowRect FakeRect
#define ShowWindow FakeShow
#define DestroyWindow FakeDestroy
#define SetFocus FakeFocus
#define SetTimer FakeTimer
#define KillTimer FakeKillTimer
#define DefWindowProcW FakeDef
#define GetMessageTime FakeMessageTime
#define IsWindow(hwnd) ((hwnd) != nullptr)
namespace Magpie {
struct DeviceResources {};
struct ImGuiBackend {
    bool Initialize(DeviceResources&) { return true; }
    bool BuildFonts() { return true; }
    void RenderDrawData(ImDrawData&, POINT) {}
};
struct Logger {
    static Logger& Get() { static Logger l; return l; }
    template<class... T> void Error(T&&...) {}
    template<class... T> void Warn(T&&...) {}
    template<class... T> void Win32Error(T&&...) {}
};
struct StrHelper { template<class... T> static std::string Concat(T&&... t) { std::string s; (s.append(t),...); return s; } };
struct Win32Helper { static SIZE GetSizeOfRect(const RECT& r) { return {r.right-r.left,r.bottom-r.top}; } };
}
'''

fixture = r'''
namespace Magpie {
ENUM
SESSION
enum class OverlayAction { Profiler };
class OverlayDrawer {
public:
    ~OverlayDrawer() noexcept;
    HOST_MEMBERS
    bool _isEffectParametersVisible = false, _overlayDirty = false;
    bool _isToolbarVisible=false, _isToolbarPinned=false, _isProfilerVisible=false;
    OverlaySessionState CaptureSessionState() const noexcept;
    void RestoreSessionState(const OverlaySessionState&) noexcept;
    void InvokeAction(OverlayAction) { _isProfilerVisible = !_isProfilerVisible; }
    void _ClearStatesIfNoVisibleWindow() { if (!_isEffectParametersVisible && !_isToolbarVisible && !_isProfilerVisible) ClearStates(); }
    ImGuiImpl _imguiImpl;
    bool IsEditingParameters() const noexcept { return _parameterPanelState == ParameterPanelState::Edit; }
    void ClearStates() noexcept { _imguiImpl.ClearStates(); _overlayDirty=true; }
    void SuspendParameterInput() noexcept;
    void ReleaseParameterInput() noexcept;
    bool HasHeldParameterInput() const noexcept;
    void UpdateParameterInputHost() noexcept;
    bool MessageHandler(UINT m, WPARAM w, LPARAM l) noexcept { return _imguiImpl.MessageHandler(m,w,l) == ImGuiInputResult::Urgent; }
};
static OverlayDrawer* overlay;
class CursorManager {
public:
    bool onOverlay=false, captured=false;
    void Update() { ++updates; }
    POINT CursorPos() const { return cursor; }
    bool IsCursorCapturedOnForeground() const { return false; }
    void IsCursorOnOverlay(bool value) { onOverlay=value; }
    void IsCursorCapturedOnOverlay(bool value) { captured=value; }
};
struct TestRenderer {
    RECT rect{0,0,800,600};
    const RECT& DestRect() const { return rect; }
    HWND ParameterInputHandle() const { return overlay->_hwndParameterInput; }
};
struct TestSource { HWND Handle() const { return game; } bool SetFocus() const { return FakeSetForeground(game); } };
class ScalingWindow {
public:
    static ScalingWindow& Get() { static ScalingWindow window; return window; }
    TestRenderer renderer;
    Magpie::CursorManager cursorManager;
    TestSource source;
    int toasts=0;
    bool alive=true, _stopRequested=false, _isDestroying=false;
    HWND Handle() const { return alive ? scaling : nullptr; }
    bool HasHeldParameterInput() const { return overlay->HasHeldParameterInput(); }
    void Stop() noexcept;
    STOP_PROCESS
    void Destroy() { overlay->ReleaseParameterInput(); alive=false; }
    void _CancelParameterRestart() {}
    void CleanAfterSrcRepositioned() {}
    TestRenderer& Renderer() { return renderer; }
    const RECT& RendererRect() const { return renderer.rect; }
    Magpie::CursorManager& CursorManager() { return cursorManager; }
    Magpie::CursorManager* TryGetCursorManager() { return &cursorManager; }
    TestSource& SrcTracker() { return source; }
    bool IsParameterInputWindow(HWND hwnd) { return hwnd && hwnd==overlay->_hwndParameterInput; }
    bool IsResizingOrMoving() const { return false; }
    std::wstring GetLocalizedString(std::wstring_view s) { return std::wstring(s); }
    void ShowToast(std::wstring_view) { ++toasts; }
};
}
'''.replace('ENUM', enum).replace('HOST_MEMBERS', host_members)

tests = r'''
using namespace Magpie;
enum class ShortcutAction { EffectParameters };
struct ShortcutProbe {
    std::array<bool,256> _parameterShortcutKeys{}; bool _keyboardHookShortcutActivated=false;
    void Claim(int code) { auto& that=*this; auto action=ShortcutAction::EffectParameters; SHORTCUT_CLAIM }
    int Edge(int key, WPARAM wParam) { KBDLLHOOKSTRUCT data{}; data.vkCode=key; auto* info=&data; auto& that=*this; SHORTCUT_EDGE return 0; }
};
static int slider = 25;
static bool checkbox = false;
static POINT choicePoint{}, childPoint{};
static phmap::flat_hash_map<std::string,OverlayWindowOption> windows;
static void Frame(bool present=true) {
    overlay->_imguiImpl.NewFrame(windows,0,1);
    ImGui::SetNextWindowPos({20,20}); ImGui::SetNextWindowSize({300,260});
    if (ImGui::Begin("Parameters - mode###effectParameters",nullptr,overlay->IsEditingParameters()?0:ImGuiWindowFlags_NoInputs)) {
        ImGui::SliderInt("Slider",&slider,0,100);
        ImGui::Checkbox("Checkbox",&checkbox);
        const bool comboOpen = ImGui::BeginCombo("Choice","Current");
        if (!comboOpen) { auto r = ImGui::GetItemRectMin(); choicePoint = {LONG(r.x+100),LONG(r.y+8)}; }
        if (comboOpen) {
            ImGui::Selectable("One"); ImGui::Selectable("Two"); ImGui::EndCombo();
        }
        ImGui::BeginChild("previewProbe",{200,70});
        auto cp=ImGui::GetWindowPos(); childPoint={LONG(cp.x+20),LONG(cp.y+20)};
        ImGui::TextUnformatted("Preview child region"); ImGui::EndChild();
    }
    ImGui::End();
    overlay->_imguiImpl.Draw({});
    if (present) overlay->_imguiImpl.OnPresentSucceeded();
}
static void Frames(int count=6) { for(int i=0;i<count;++i) Frame(); }
static void Mouse(UINT msg, int x, int y) {
    cursor={x,y};
    if(msg==WM_LBUTTONDOWN) keys[VK_LBUTTON]=true;
    if(msg==WM_LBUTTONUP) keys[VK_LBUTTON]=false;
    if (foreground==game) { if (msg==WM_LBUTTONDOWN || msg==WM_LBUTTONUP) ++gameEdges; return; }
    OverlayDrawer::_ParameterInputWndProc(inputHost,msg,0,0);
}
static void Key(UINT msg, int key) {
    keys[key]=msg==WM_KEYDOWN;
    OverlayDrawer::_ParameterInputWndProc(inputHost,msg,key,0);
}
int main() {
    ShortcutProbe shortcut;
    keys['E']=false; shortcut.Claim('E'); assert(shortcut._parameterShortcutKeys['E']);
    assert(shortcut.Edge('E',WM_KEYDOWN)==1 && shortcut.Edge('E',WM_KEYUP)==1);
    assert(!shortcut._parameterShortcutKeys['E']);
    keys['E']=true; shortcut.Claim('E'); assert(!shortcut._parameterShortcutKeys['E']);
    assert(shortcut.Edge('E',WM_KEYUP)==0); keys['E']=false;
    OverlayDrawer panel; overlay=&panel;
    DeviceResources resources; assert(panel._imguiImpl.Initialize(resources));
    auto& io=ImGui::GetIO(); unsigned char* pixels; int w,h;
    io.Fonts->GetTexDataAsRGBA32(&pixels,&w,&h);
    io.DisplaySize={800,600}; io.DeltaTime=1.0f/60;
    panel._SetParameterPanelState(ParameterPanelState::Edit); Frame();
    assert(panel._imguiImpl.OwnsPointerAtCursor()); Frames();
    assert(panel.IsEditingParameters() && foreground==inputHost && visibleHost);
    // Moving out never exits edit; the first outside click stays entirely in the host.
    Mouse(WM_MOUSEMOVE,600,450); Frames(); assert(panel.IsEditingParameters());
    Mouse(WM_LBUTTONDOWN,600,450); Frames(); panel.UpdateParameterInputHost();
    assert(panel.IsEditingParameters() && foreground==inputHost && gameEdges==0);
    Mouse(WM_MOUSEMOVE,610,455); // Queued noncritical movement must not postpone the release.
    Mouse(WM_LBUTTONUP,600,450);
    assert(!panel.IsEditingParameters()); Frames();
    Mouse(WM_MOUSEMOVE,610,455);
    panel.UpdateParameterInputHost();
    assert(panel._parameterPanelState==ParameterPanelState::Preview && !visibleHost && foreground==game && gameEdges==0);
    // Preview, including child windows, must not claim ImGui mouse input.
    cursor={100,80}; Frames(); assert(!io.WantCaptureMouse);
    cursor=childPoint; Frames(); assert(!io.WantCaptureMouse && !panel._imguiImpl.OwnsPointerAtCursor());
    Mouse(WM_LBUTTONDOWN,600,450); Mouse(WM_LBUTTONUP,600,450); assert(gameEdges==2);
    // A drag that starts on the panel stays captured after leaving the panel.
    panel._SetParameterPanelState(ParameterPanelState::Edit); Frames();
    Mouse(WM_LBUTTONDOWN,100,51); Frames();
    assert(ImGui::IsAnyItemActive() && capture==inputHost);
    Mouse(WM_MOUSEMOVE,600,450); Frames();
    assert(panel.IsEditingParameters() && capture==inputHost);
    Mouse(WM_LBUTTONUP,600,450); Frames(); panel.UpdateParameterInputHost();
    assert(panel.IsEditingParameters() && !capture && !ImGui::IsAnyItemActive());
    // A game-area click returns after its pair even when a dropdown is open.
    Mouse(WM_LBUTTONDOWN,choicePoint.x,choicePoint.y); Frames(); Mouse(WM_LBUTTONUP,choicePoint.x,choicePoint.y); Frames();
    assert(!ImGui::GetCurrentContext()->OpenPopupStack.empty());
    Mouse(WM_LBUTTONDOWN,600,450); Frames(); Mouse(WM_LBUTTONUP,600,450); Frames();
    panel.UpdateParameterInputHost();
    assert(!panel.IsEditingParameters() && ImGui::GetCurrentContext()->OpenPopupStack.empty());
    panel._SetParameterPanelState(ParameterPanelState::Edit); Frames();
    // Keyboard Escape first dismisses a popup, then exits on a complete key pair.
    Mouse(WM_LBUTTONDOWN,choicePoint.x,choicePoint.y); Frames(); Mouse(WM_LBUTTONUP,choicePoint.x,choicePoint.y); Frames();
    assert(!ImGui::GetCurrentContext()->OpenPopupStack.empty());
    Key(WM_KEYDOWN,VK_ESCAPE); Key(WM_KEYUP,VK_ESCAPE); Frames();
    assert(panel.IsEditingParameters() && ImGui::GetCurrentContext()->OpenPopupStack.empty());
    Key(WM_KEYDOWN,VK_ESCAPE); assert(panel.IsEditingParameters());
    Key(WM_KEYUP,VK_ESCAPE); Frames(); panel.UpdateParameterInputHost();
    assert(!panel.IsEditingParameters() && foreground==game);
    // Real ImGui numeric input receives modifier/key/character events in order.
    panel._SetParameterPanelState(ParameterPanelState::Edit); Frames();
    Key(WM_KEYDOWN,VK_CONTROL); Frames();
    Mouse(WM_LBUTTONDOWN,100,51); Frames(); Mouse(WM_LBUTTONUP,100,51); Frames();
    assert(ImGui::GetCurrentContext()->InputTextState.ID == ImGui::GetCurrentContext()->ActiveId && ImGui::IsAnyItemActive());
    Key(WM_KEYDOWN,'A'); Frames(); Key(WM_KEYUP,'A'); Key(WM_KEYUP,VK_CONTROL); Frames();
    OverlayDrawer::_ParameterInputWndProc(inputHost,WM_CHAR,'4',0);
    OverlayDrawer::_ParameterInputWndProc(inputHost,WM_CHAR,'2',0); Frames();
    Key(WM_KEYDOWN,VK_RETURN); Frames(); Key(WM_KEYUP,VK_RETURN); Frames();
    assert(slider==42 && panel.IsEditingParameters() && gameEdges==2);
    panel._SetParameterPanelState(ParameterPanelState::Preview); Frames(); panel.UpdateParameterInputHost();
    // Shortcut modifiers and queued releases delay a return to the game.
    keys[VK_CONTROL]=true; panel._SetParameterPanelState(ParameterPanelState::Edit); Frames();
    panel._SetParameterPanelState(ParameterPanelState::Preview); assert(panel.IsEditingParameters());
    Key(WM_KEYUP,VK_CONTROL); Frames(); panel.UpdateParameterInputHost(); assert(!panel.IsEditingParameters());
    panel._SetParameterPanelState(ParameterPanelState::Edit); Frames();
    Mouse(WM_LBUTTONDOWN,100,51); Frames();
    panel._SetParameterPanelState(ParameterPanelState::Preview); assert(panel.IsEditingParameters());
    Mouse(WM_LBUTTONUP,100,51); assert(panel.IsEditingParameters());
    Frames(); panel.UpdateParameterInputHost(); assert(!panel.IsEditingParameters());
    // Losing foreground cancels capture and ImGui activity without activating the game.
    panel._SetParameterPanelState(ParameterPanelState::Edit); Frames();
    Mouse(WM_LBUTTONDOWN,100,51); Frames();
    const int attempts=focusAttempts; FakeSetForeground((HWND)9);
    assert(!panel.IsEditingParameters() && foreground==(HWND)9 && !capture && !visibleHost);
    assert(focusAttempts==attempts+1 && !ImGui::IsAnyItemActive());
    Mouse(WM_LBUTTONUP,600,450); keys.fill(false);
    // A rebuild restore over another app stays in preview. No focus attempt is made.
    panel._SetParameterPanelState(ParameterPanelState::Edit,false);
    assert(!panel.IsEditingParameters() && foreground==(HWND)9 && focusAttempts==attempts+1);
    // Focus failure makes one attempt, then leaves a visible preview and no capture.
    foreground=game; denyFocus=true; panel._SetParameterPanelState(ParameterPanelState::Edit);
    assert(!panel.IsEditingParameters() && panel._parameterFocusFailed && !visibleHost && !capture);
    const int failedAttempts=focusAttempts; panel.UpdateParameterInputHost(); panel.UpdateParameterInputHost();
    assert(focusAttempts==failedAttempts); denyFocus=false;
    // Stopping while a panel press is down defers HWND destruction until its up.
    panel._SetParameterPanelState(ParameterPanelState::Edit); Frames();
    Mouse(WM_LBUTTONDOWN,100,51); Key(WM_KEYDOWN,'Z'); Frames();
    auto& scalingWindow = ScalingWindow::Get(); scalingWindow.Stop();
    assert(scalingWindow.Handle() && scalingWindow._stopRequested && panel.IsEditingParameters());
    assert(!scalingWindow.ProcessPendingStop());
    Mouse(WM_LBUTTONUP,600,450); Key(WM_KEYUP,'Z'); Frames();
    assert(scalingWindow.ProcessPendingStop());
    assert(!scalingWindow.Handle() && !visibleHost && !capture && gameEdges==2);
    scalingWindow.alive=true;
    panel._SetParameterPanelState(ParameterPanelState::Edit); Frames();
    const auto editingState=panel.CaptureSessionState();
    Key(WM_KEYDOWN,'Z'); panel._SetParameterPanelState(ParameterPanelState::Preview);
    assert(panel.CaptureSessionState().parameterPanelState==ParameterPanelState::Preview);
    panel._SetParameterPanelState(ParameterPanelState::Closed);
    assert(!panel.CaptureSessionState().effectParametersVisible);
    Key(WM_KEYUP,'Z'); Frames(); panel.UpdateParameterInputHost();
    panel.RestoreSessionState(editingState); Frames(); assert(panel.IsEditingParameters());
    panel.ReleaseParameterInput(); foreground=(HWND)9;
    const int beforeRestore=focusAttempts; panel.RestoreSessionState(editingState); Frames();
    assert(!panel.IsEditingParameters() && foreground==(HWND)9 && focusAttempts==beforeRestore);
    foreground=game; panel.RestoreSessionState(editingState); Frames(); assert(panel.IsEditingParameters());
    panel._SetParameterPanelState(ParameterPanelState::Closed); Frames(); panel.UpdateParameterInputHost();
    assert(!panel._isEffectParametersVisible && !visibleHost && foreground==game);
    std::cout << "PASS production input: three states, preview hit test, outside click pairing, drag outside, popup ownership, numeric keyboard input, Esc priority, modifier and release drain, external focus, restore guard, focus failure, deferred-stop input pairing, close\n";
}
'''

def method(text, signature):
    start = text.index(signature)
    left = text.index('{', start)
    depth = 1
    end = left + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]
scaling_header = (core / 'ScalingWindow.h').read_text(encoding='utf-8-sig')
scaling_cpp = (core / 'ScalingWindow.cpp').read_text(encoding='utf-8-sig')
fixture = fixture.replace('STOP_PROCESS', method(scaling_header, 'bool ProcessPendingStop() noexcept'))
session_header = (core / 'include/ScalingOptions.h').read_text(encoding='utf-8-sig')
fixture = fixture.replace('SESSION', method(session_header, 'struct OverlaySessionState') + ';')
drawer_cpp = (core / 'OverlayDrawer.cpp').read_text(encoding='utf-8-sig')
session_code = 'namespace Magpie {\n' + method(drawer_cpp, 'OverlaySessionState OverlayDrawer::CaptureSessionState()') + '\n' + method(drawer_cpp, 'void OverlayDrawer::RestoreSessionState(') + '\n}\n'
shortcut_cpp = (root / 'src/Magpie/ShortcutService.cpp').read_text(encoding='utf-8-sig')
tests = tests.replace('SHORTCUT_EDGE', method(shortcut_cpp, 'if (info->vkCode < that._parameterShortcutKeys.size()'))
claim = re.search(r'if \(action == ShortcutAction::EffectParameters &&[^\n]+\n\s+that\._parameterShortcutKeys\[code\] = true;', shortcut_cpp).group()
tests = tests.replace('SHORTCUT_CLAIM', claim)
stop_code = 'namespace Magpie {\n' + method(scaling_cpp, 'void ScalingWindow::Stop() noexcept') + '\n}\n'

output = Path(sys.argv[1]).resolve()
output.mkdir(parents=True, exist_ok=True)
(output / 'parameter_input.cpp').write_text(prefix + body('ImGuiImpl.h') + fixture + body('ImGuiImpl.cpp') + body('ParameterInputHost.cpp') + stop_code + session_code + tests, encoding='utf-8')
print(output / 'parameter_input.cpp')
