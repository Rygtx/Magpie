#pragma once
#include <atomic>
#include <cstdint>

namespace Magpie {

// A queued callback owns this token, never the Renderer. The run ID is fixed
// at session creation; reading the global ID while reporting a late failure
// could accidentally assign that failure to the next session.
class ScalingSessionLifetime {
public:
	explicit ScalingSessionLifetime(uint32_t runId) noexcept : _runId(runId) {}

	void RequestStop() noexcept { _stopping.store(true, std::memory_order_release); }
	bool IsStopping() const noexcept { return _stopping.load(std::memory_order_acquire); }
	bool IsCurrent(uint32_t runId) const noexcept { return _runId == runId && !IsStopping(); }

private:
	const uint32_t _runId;
	std::atomic<bool> _stopping = false;
};

}
