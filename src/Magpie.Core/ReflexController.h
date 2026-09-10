#pragma once
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>

struct ID3D11Device;
struct ID3D12CommandQueue;

namespace Magpie {

enum class ReflexMarker { SimulationStart, SimulationEnd, RenderStart, RenderEnd };

enum class ReflexState { Unavailable, Active, DriverOff, Paused, Faulted, Stopped };

struct ReflexConfigurationResult {
	int setStatus = 0;
	int queryStatus = 0;
	bool queried = false;
	bool lowLatency = false;
};

struct ReflexSettings {
	bool lowLatency = true;
	bool boost = false;
	uint32_t minimumIntervalUs = 0;
	bool operator==(const ReflexSettings&) const = default;
};

// Injectable driver boundary: tests exercise the production frame lifecycle
// without loading a display driver or running NGX.
class ReflexDriver {
public:
	virtual ~ReflexDriver() = default;
	virtual ReflexConfigurationResult Configure(ReflexSettings settings) noexcept = 0;
	virtual int Sleep() noexcept = 0;
	virtual int Marker(ReflexMarker marker, uint64_t frameId) noexcept = 0;
	virtual int RegisterGenerationQueue(ID3D12CommandQueue* queue) noexcept = 0;
	virtual int Generation(ID3D12CommandQueue* queue, uint64_t frameId,
		uint64_t presentId, bool start) noexcept = 0;
	virtual int FrontendRender(uint64_t frameId, uint64_t presentId, bool start) noexcept = 0;
	virtual int Present(uint64_t frameId, uint64_t presentId, bool generated, bool start) noexcept = 0;
	virtual void ReportFailure(const char* operation, int status) noexcept = 0;
};

std::unique_ptr<ReflexDriver> CreateNvReflexDriver(ID3D11Device* presentDevice) noexcept;

// Owned by Renderer, initialized before either rendering thread can use it and
// destroyed after the backend joins. The driver object stays immutable/alive
// even after a failure. Sleep never holds the configuration mutex: Present on
// the other thread must be able to make progress while the driver sleeps.
class ReflexController {
public:
	~ReflexController() { Stop(); }
	void Initialize(std::unique_ptr<ReflexDriver> driver, ReflexSettings settings = {}) noexcept {
		_driver = std::move(driver);
		_settings = settings;
		if (_driver) SetPresentationAvailable(true);
	}
	void SetPresentationAvailable(bool available) noexcept {
		if (!_driver || _stopped.load() || _presentationAvailable.load() == available) return;
		std::scoped_lock lock(_configurationMutex);
		if (!_driver || _stopped.load() || _presentationAvailable.load() == available) return;
		_presentationAvailable.store(available);
		_ConfigureLocked(available ? _settings : ReflexSettings{ .lowLatency = false });
	}
	void SetFrameRateLimit(uint32_t intervalUs) noexcept {
		std::scoped_lock lock(_configurationMutex);
		if (!_driver || _stopped.load() || _settings.minimumIntervalUs == intervalUs) return;
		_settings.minimumIntervalUs = intervalUs;
		if (_presentationAvailable.load()) _ConfigureLocked(_settings);
	}
	void Stop() noexcept {
		std::scoped_lock lock(_configurationMutex);
		_StopLocked(nullptr, 0);
	}
	ReflexState State() const noexcept { return _state.load(); }
	bool Available() const noexcept { return State() == ReflexState::Active; }
	bool CanResume() const noexcept { return _driver && !_stopped.load(); }

	// Backend only. A Waiting/duplicate capture keeps this candidate open;
	// polling, staged FG input and generated frames must not call Sleep again.
	uint64_t BeginCapture(uint64_t minimumFrameId = 1) noexcept {
		if (_captureFrameId) return _captureFrameId;
		if (!Available() || !_Check("Sleep", _driver->Sleep()) || !Available()) return 0;
		_nextFrameId = std::max(_nextFrameId + 1, minimumFrameId);
		_captureFrameId = _nextFrameId;
		_renderEnded = false;
		_Marker(ReflexMarker::SimulationStart);
		// Magpie has no access to the source application's simulation or input.
		// This is its own capture/processing cycle; do not invent INPUT_SAMPLE.
		_Marker(ReflexMarker::SimulationEnd);
		_Marker(ReflexMarker::RenderStart);
		return _captureFrameId;
	}
	uint64_t CaptureFrameId() const noexcept { return _captureFrameId; }
	uint64_t NextPresentId() noexcept { return _captureFrameId ? ++_nextPresentId : 0; }
	void EndCaptureRender() noexcept {
		if (_captureFrameId && !_renderEnded) {
			_Marker(ReflexMarker::RenderEnd);
			_renderEnded = true;
		}
	}
	void CompleteCapture() noexcept {
		EndCaptureRender();
		_captureFrameId = 0;
	}
	void RegisterGenerationQueue(ID3D12CommandQueue* queue) noexcept {
		if (_Usable()) _Check("NotifyOutOfBandCommandQueue", _driver->RegisterGenerationQueue(queue));
	}
	void Generation(ID3D12CommandQueue* queue, uint64_t frameId,
		uint64_t presentId, bool start) noexcept {
		if (frameId && presentId && _Usable())
			_Check("D3D12 async generation marker", _driver->Generation(queue, frameId, presentId, start));
	}
	// Frontend only. IDs come from the immutable published ring slot, never
	// from the backend's currently running (possibly newer) capture.
	void FrontendRender(uint64_t frameId, uint64_t presentId, bool start) noexcept {
		if (frameId && presentId && _Usable())
			_Check("D3D11 async render marker", _driver->FrontendRender(frameId, presentId, start));
	}
	void Present(uint64_t frameId, uint64_t presentId, bool generated, bool start) noexcept {
		if (frameId && presentId && _Usable())
			_Check("D3D11 async present marker", _driver->Present(frameId, presentId, generated, start));
	}

private:
	void _ConfigureLocked(ReflexSettings settings) noexcept {
		const auto result = _driver->Configure(settings);
		if (result.setStatus || (result.queried && result.queryStatus)) {
			_StopLocked(result.setStatus ? "SetSleepMode" : "GetSleepStatus",
				result.setStatus ? result.setStatus : result.queryStatus);
			return;
		}
		if (_presentationAvailable.load() && !result.lowLatency && settings.minimumIntervalUs) {
			// The driver cap is independent of low-latency activation. Clear it
			// before the renderer falls back to Async, so two limiters cannot run.
			const auto reset = _driver->Configure(ReflexSettings{ .lowLatency = false });
			if (reset.setStatus || (reset.queried && reset.queryStatus)) {
				_StopLocked("clear inactive Reflex frame limit",
					reset.setStatus ? reset.setStatus : reset.queryStatus);
				return;
			}
		}
		// A successful request does not guarantee driver activation (for example,
		// a control-panel override). Keep the interface alive without re-requesting
		// On every frame or pretending that the driver returned NOT_SUPPORTED.
		_state.store(!_presentationAvailable.load() ? ReflexState::Paused :
			(result.queried && result.lowLatency ? ReflexState::Active : ReflexState::DriverOff));
	}
	bool _Usable() const noexcept { return _driver && !_stopped.load(); }
	void _Marker(ReflexMarker marker) noexcept {
		if (_Usable()) _Check("SetLatencyMarker", _driver->Marker(marker, _captureFrameId));
	}
	bool _Check(const char* operation, int status) noexcept {
		if (!status) return true;
		std::scoped_lock lock(_configurationMutex);
		_StopLocked(operation, status);
		return false;
	}
	void _StopLocked(const char* operation, int status) noexcept {
		if (!_driver || _stopped.exchange(true)) return;
		_state.store(operation ? ReflexState::Faulted : ReflexState::Stopped);
		if (operation) _driver->ReportFailure(operation, status);
		const auto reset = _driver->Configure(ReflexSettings{ .lowLatency = false });
		if (reset.setStatus) _driver->ReportFailure("restore SleepMode Off", reset.setStatus);
		else if (reset.queried && reset.queryStatus)
			_driver->ReportFailure("query restored SleepMode", reset.queryStatus);
	}
	std::unique_ptr<ReflexDriver> _driver;
	ReflexSettings _settings;
	std::mutex _configurationMutex;
	std::atomic<ReflexState> _state = ReflexState::Unavailable;
	std::atomic<bool> _presentationAvailable = false;
	std::atomic<bool> _stopped = false;
	uint64_t _nextFrameId = 0;
	uint64_t _nextPresentId = 0;
	uint64_t _captureFrameId = 0;
	bool _renderEnded = false;
};

}
