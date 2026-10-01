#pragma once
#include <array>
#include <cmath>
#include <string_view>

namespace Magpie {
inline constexpr std::array<std::string_view, 14> DLSSNR_RESIDUAL_PARAMETERS{
	"residualMultiplier", "residualSaturation", "residualLightness",
	"shadowStructureMultiplier", "reflectionGlowMultiplier", "residualColorMode",
	"residualHueProtection", "residualDarkProtection", "residualHighlightProtection",
	"residualLocalCompression", "residualLowFrequencyGain", "residualDetailGain",
	"residualChromaTemporalStrength", "residualDebugView"
};
inline bool IsDLSSNRResidualParameter(std::string_view name) noexcept {
	for (auto value : DLSSNR_RESIDUAL_PARAMETERS) if (value == name) return true;
	return false;
}
inline bool IsDLSSNROklabParameter(std::string_view name) noexcept {
	return name == "residualHueProtection" || name == "residualDarkProtection" ||
		name == "residualHighlightProtection" || name == "residualLocalCompression" ||
		name == "residualLowFrequencyGain" || name == "residualDetailGain" ||
		name == "residualChromaTemporalStrength" || name == "residualDebugView";
}
template<class GetValue>
int DLSSNRColorMode(GetValue&& get) noexcept {
	// Unversioned imported/old options retain HSL. New effects explicitly store 1.
	return get("residualColorMode", 0.f) == 1.f ? 1 : 0;
}
// Shared by import normalization and the new-effect creation path. Copy/export
// preserve the explicit key, including hidden controls.
template<class Map>
void InitializeDLSSNRColorMode(Map& values, bool newlyCreated) {
	values.try_emplace(L"residualColorMode", newlyCreated ? 1.f : 0.f);
}
}
