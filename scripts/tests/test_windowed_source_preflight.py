"""Extract production preflight branches for a CPU-only boundary regression test."""
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[2]
tracker = (root / 'src/Magpie.Core/SrcTracker.cpp').read_text(encoding='utf-8-sig')
window = (root / 'src/Magpie.Core/ScalingWindow.cpp').read_text(encoding='utf-8-sig')


def block(text, marker):
    start = text.index(marker)
    brace = text.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


preflight = block(tracker, 'if (options.IsWindowedMode()) {')
maximized = block(window, 'if (_srcTracker.IsZoomed()) {')
assert tracker.index(preflight) < tracker.index('// 计算窗口样式') < tracker.index('return _CalcSrcRect(')
start_impl = window.split('ScalingError ScalingWindow::_StartImpl(', 1)[1].split('\nvoid ScalingWindow::Start(', 1)[0]
assert start_impl.index('_srcTracker.Set(') < start_impl.index('CreateWindowEx(') < start_impl.index('_renderer->Initialize(')
assert 'return error;' in start_impl.split('_srcTracker.Set(', 1)[1].split('CreateWindowEx(', 1)[0]
assert 'if (!_options.IsWindowedMode() && !_options.RealIsAllowScalingMaximized())' in start_impl

harness = r'''
#define NOMINMAX
#include "SourceWindowGeometry.h"
#include <cassert>
#include <iostream>
#include <string>
using namespace Magpie;
enum class ScalingError { NoError, DisplayLayoutFailed, BannedInWindowedMode, Maximized };
struct Logger {
    static Logger& Get() { static Logger l; return l; }
    void Info(const std::string&) {} void Win32Error(const char*) {}
};
namespace fmt { template<class... T> std::string format(const char* text, T...) { return text; } }
struct Options {
    bool windowed=true, allowFullscreen=true;
    bool IsWindowedMode() const { return windowed; }
    bool RealIsAllowScalingMaximized() const { return allowFullscreen && !windowed; }
};
struct Tracker { bool maximized=false; bool IsZoomed() const { return maximized; } };
static RECT monitor{0,0,2560,1440};
static bool monitorAvailable=true;
static int monitorQueries=0, windowCreations=0, gpuInitializations=0;
bool TestGetMonitorInfo(HMONITOR, MONITORINFO* info) {
    ++monitorQueries;
    if (!monitorAvailable) return false;
    info->rcMonitor=monitor;
    info->rcWork={monitor.left,monitor.top,monitor.right,monitor.bottom-48};
    return true;
}
#define GetMonitorInfoW TestGetMonitorInfo
ScalingError Check(RECT frame, RECT client, Options options={}, bool zoomed=false) {
    const auto& _options=options;
    const RECT& _windowFrameRect=frame;
    const RECT& clientRect=client;
    HMONITOR hMon=nullptr;
    Tracker _srcTracker{zoomed};
    PRODUCTION_PREFLIGHT
    PRODUCTION_MAXIMIZED
    ++windowCreations;
    ++gpuInitializations;
    return ScalingError::NoError;
}
void Rejected(RECT frame, RECT client, bool zoomed=false) {
    windowCreations=gpuInitializations=0;
    assert(Check(frame,client,{},zoomed)==ScalingError::BannedInWindowedMode);
    assert(windowCreations==0 && gpuInitializations==0);
}
void Accepted(RECT frame, RECT client, Options options={}) {
    windowCreations=gpuInitializations=0;
    assert(Check(frame,client,options)==ScalingError::NoError);
    assert(windowCreations==1 && gpuInitializations==1);
}
int main() {
    // The reported 2560x1440 source, including non-maximized borderless windows.
    Rejected(monitor,monitor);
    Rejected(monitor,{0,0,2560,1392});
    Rejected({1,1,2559,1439},monitor); // client is authoritative if DWM bounds are inset
    Rejected({-8,-8,2568,1448},{0,0,2560,1440});
    Rejected({0,0,2560,1392},{0,31,2560,1392},true); // maximized, taskbar visible
    Accepted({0,0,2560,1392},{0,31,2560,1392}); // ordinary work-area window
    Accepted({320,180,2240,1260},{328,211,2232,1252});
    Accepted({0,0,2559,1440},{0,0,2559,1440}); // one axis alone is not fullscreen
    Accepted({0,0,2560,1439},{0,0,2560,1439});
    // Crop/scale/output positions are deliberately absent from the source check.
    Rejected(monitor,monitor);
    // Source monitor can be left/above primary, portrait, or a different DPI.
    for (RECT bounds : {RECT{-2560,0,0,1440},RECT{2560,-180,6400,1980},
        RECT{0,-2560,1440,0},RECT{-3840,-2160,0,0},RECT{0,0,5120,2880}}) {
        monitor=bounds;
        Rejected(bounds,bounds);
        RECT normal{bounds.left+100,bounds.top+100,bounds.right-100,bounds.bottom-100};
        Accepted(normal,normal);
    }
    monitor={0,0,2560,1440};
    Rejected({-2560,0,2560,1440},{-2560,0,2560,1440}); // spanning source covers target
    assert(!SourceWindowCoversMonitor(monitor,monitor,RECT{}));
    monitorAvailable=false;
    windowCreations=gpuInitializations=0;
    assert(Check(monitor,monitor)==ScalingError::DisplayLayoutFailed);
    assert(windowCreations==0 && gpuInitializations==0);
    // The existing fullscreen-effects path does not acquire a new restriction.
    const int queries=monitorQueries;
    Accepted(monitor,monitor,Options{false,true});
    assert(monitorQueries==queries);
    monitorAvailable=true;
    std::cout << "Windowed preflight: production branches reject fullscreen/maximized sources "
        "before window/GPU work; ordinary windows, physical-coordinate/DPI layouts, "
        "negative monitor coordinates, query failure and fullscreen-effects bypass passed.\n";
}
'''.replace('PRODUCTION_PREFLIGHT', preflight).replace('PRODUCTION_MAXIMIZED', maximized)
out = Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
(out / 'windowed_source_preflight.cpp').write_text(harness, encoding='utf-8')
print(out / 'windowed_source_preflight.cpp')
