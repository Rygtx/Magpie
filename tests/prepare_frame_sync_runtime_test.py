"""Test the production renderer limiter configuration without any graphics API."""
from pathlib import Path
import sys

repo = Path(__file__).resolve().parents[1]
output = Path(sys.argv[1])
source = (repo/'src/Magpie.Core/Renderer.cpp').read_text(encoding='utf-8-sig')

def function(signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

prefix = r'''
#include "FramePacingOptions.h"
#include "FramePresentationTiming.h"
#include <atomic>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <vector>
namespace fmt { template<class... T> std::string format(T&&...) { return {}; } }
namespace Magpie {
struct EffectOption { std::string name; std::map<std::string,float> parameters; };
bool IsFrameGenerationEffect(std::string_view name) { return name == "DLSSFG" || name == "XeSSFG"; }
struct ScalingOptions {
    bool isFrontEdgeSyncEnabled=true;
    float frontEdgeSyncFrameRate=80, minFrameRate=0;
    std::optional<float> maxFrameRate;
    bool IsBenchmarkMode() const { return false; }
};
struct Dispatcher { template<class F> void TryEnqueue(F) {} };
struct ScalingWindow {
    ScalingOptions options;
    static ScalingWindow& Get() { static ScalingWindow window; return window; }
    auto& Options() { return options; }
    static auto Dispatcher() { return Magpie::Dispatcher{}; }
    static int RunId() { return 1; }
    const wchar_t* GetLocalizedString(const wchar_t* value) { return value; }
    void ShowToast(const wchar_t*) {}
    explicit operator bool() const { return true; }
};
struct Session { bool IsCurrent(int) { return true; } };
struct Logger { static Logger& Get() { static Logger logger; return logger; } void Info(std::string) {} };
struct Timer {
    std::optional<float> limit;
    bool strict=false;
    void Initialize(float, std::optional<float> value, bool strictValue=false) { limit=value; strict=strictValue; }
};
struct Reflex {
    bool available=true, failed=false;
    uint32_t interval=0;
    void SetFrameRateLimit(uint32_t value) { interval=value; if (failed) available=false; }
    bool Available() const { return available; }
    bool CanResume() const { return !failed; }
};
struct FG { unsigned Multiplier() const { return 2; } };
struct Renderer {
    std::optional<float> _captureMaxFrameRate;
    std::vector<EffectOption> _runtimeEffectOptions;
    float _frameRateFilterTarget=0;
    std::atomic<double> _presentationRefreshRate=240, _existingBaseFrameRateLimit=0;
    unsigned _configuredFrameGenerationMultiplier=1;
    bool _frameSyncEnabled=true, _frameSyncUsesSharedSlot=true;
    std::unique_ptr<FG> _dlssFrameGenerator;
    std::vector<int> _effectDrawers{1};
    FrameSyncBackend _frameSyncBackend=FrameSyncBackend::FrontEdge;
    FrameSyncBackend _appliedFrameSyncBackend=FrameSyncBackend::None;
    Reflex _reflex;
    bool _reflexFallbackNotified=false;
    Session session;
    Session* _sessionLifetime=&session;
    Timer _stepTimer;
    double _baseFrameRateLimit=0;
    CaptureFrameCadence _captureCadence;
    std::chrono::nanoseconds _synchronousPresentInterval{};
    FrameSyncBackend ActiveFrameSyncBackend() const {
        return _frameSyncBackend==FrameSyncBackend::Reflex && !_reflex.Available() ? FrameSyncBackend::Async : _frameSyncBackend;
    }
    double _FrameSyncFrameRate() const noexcept;
    void _UpdateFrameRateLimits() noexcept;
};
'''
suffix = r'''
}
int main() {
    using namespace Magpie;
    auto& options=ScalingWindow::Get().Options();
    Renderer renderer;
    renderer._UpdateFrameRateLimits();
    assert(!renderer._stepTimer.limit && renderer._reflex.interval==0);
    renderer._frameSyncBackend=FrameSyncBackend::Async;
    renderer._UpdateFrameRateLimits();
    assert(renderer._stepTimer.limit==80 && renderer._stepTimer.strict && renderer._reflex.interval==0);
    renderer._frameSyncBackend=FrameSyncBackend::Reflex;
    renderer._UpdateFrameRateLimits();
    assert(!renderer._stepTimer.limit && renderer._reflex.interval==12500);
    options.maxFrameRate=60.0f;
    renderer._UpdateFrameRateLimits();
    assert(!renderer._stepTimer.limit && renderer._reflex.interval==16667 && renderer._baseFrameRateLimit==60);
    renderer._reflex.failed=true;
    renderer._UpdateFrameRateLimits();
    assert(renderer._appliedFrameSyncBackend==FrameSyncBackend::Async && renderer._stepTimer.limit==60);
    assert(renderer._reflexFallbackNotified);
    // DLSS compatibility path retains Front Edge. Its failed FG fallback must still cap.
    Renderer dlss;
    dlss._frameSyncUsesSharedSlot=false;
    dlss._runtimeEffectOptions={{"DLSSFG",{}}};
    dlss._configuredFrameGenerationMultiplier=2;
    dlss._dlssFrameGenerator=std::make_unique<FG>();
    dlss._UpdateFrameRateLimits();
    assert(!dlss._stepTimer.limit && dlss._reflex.interval==0);
    for (unsigned multiplier=2;multiplier<=4;++multiplier) {
        dlss._configuredFrameGenerationMultiplier=multiplier;
        dlss._frameSyncBackend=ResolveFrameSyncBackend({true,80,FrameSyncMode::Reflex},true,false,true,false);
        dlss._UpdateFrameRateLimits();
        assert(dlss._stepTimer.limit==60 && dlss._stepTimer.strict && dlss._reflex.interval==0);
    }
    dlss._configuredFrameGenerationMultiplier=2;
    dlss._frameSyncBackend=FrameSyncBackend::FrontEdge;
    dlss._dlssFrameGenerator.reset();
    dlss._UpdateFrameRateLimits();
    assert(dlss._stepTimer.limit==60 && dlss._reflex.interval==0);
    options.maxFrameRate.reset(); options.frontEdgeSyncFrameRate=0;
    dlss._frameSyncBackend=FrameSyncBackend::Async;
    dlss._UpdateFrameRateLimits();
    assert(dlss._stepTimer.limit==120 && dlss._reflex.interval==0);
    dlss._frameSyncBackend=FrameSyncBackend::XeLL;
    dlss._UpdateFrameRateLimits();
    assert(!dlss._stepTimer.limit && dlss._reflex.interval==0);
    std::cout << "PASS: production renderer single limiter ownership, lower profile cap, Reflex failure fallback, DLSS recovery and XeLL handoff\n";
}
'''
(output/'frame_sync_runtime.cpp').write_text(prefix + function('double Renderer::_FrameSyncFrameRate()') + '\n' +
    function('void Renderer::_UpdateFrameRateLimits()') + suffix, encoding='utf-8')
