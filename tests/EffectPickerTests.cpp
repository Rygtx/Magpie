#include "../src/Magpie/EffectPickerModel.h"
#include "../src/Magpie/EffectHelper.h"
#include <cassert>
#include <iostream>

int main() {
	using namespace Magpie;
	const auto search = NormalizeEffectSearch(L"DLSS\\DLSS_SR 抗锯齿 时序边缘平滑");
	assert(MatchesEffectSearch(search, L"dlss sr"));
	assert(MatchesEffectSearch(search, L"抗锯齿 DLSS"));
	assert(MatchesEffectSearch(search, L" DLSS_SR\t"));
	assert(!MatchesEffectSearch(search, L"DLSS 降噪"));
	assert(MatchesEffectSearch(search, L"   "));
	EffectPickerEntry aa;
	aa.category = L"upscale";
	aa.subcategory = L"动画线条";
	aa.purposes = { L"upscale", L"antialiasing" };
	assert(MatchesEffectCategory(aa, L"", L""));
	assert(MatchesEffectCategory(aa, L"antialiasing", L""));
	assert(!MatchesEffectCategory(aa, L"antialiasing", L"时序"));
	assert(MatchesEffectCategory(aa, L"upscale", L"动画线条"));
	assert(!MatchesEffectCategory(aa, L"first_try", L""));
	aa.firstTry = true;
	assert(MatchesEffectCategory(aa, L"first_try", L""));
	for (int family = 0; family < 2; ++family) {
		for (int tier = 0; tier < 4; ++tier) {
			const auto oldId = RTX_VIDEO_IDS[family][tier];
			assert(RTXVideoFamily(oldId) == family);
			assert(RTXVideoStrength(oldId) == tier);
			assert(RTXVideoId(oldId, tier) == oldId);
			for (int newTier = 0; newTier < 4; ++newTier) {
				const auto nextId = RTXVideoId(oldId, newTier);
				assert(RTXVideoId(nextId, tier) == oldId);
				assert(EffectHelper::GetDisplayName(nextId) == EffectHelper::GetDisplayName(oldId));
			}
			assert(RTXVideoId(oldId, -1).empty());
			assert(RTXVideoId(oldId, 4).empty());
		}
	}
	assert(RTXVideoFamily(L"custom\\RTXVideo_VSR_High") == -1);
	assert(RTXVideoFamily(L"RTXVideo\\RTXVideo_VSR_High_Custom") == -1);
	assert(RTXVideoId(L"RTXVideo\\RTXVideo_VSR", 0).empty());
	assert(EffectHelper::GetDisplayName(L"XeSSFG\\XeSS_FrameGeneration_x2_ZeroMV") == L"XeSS_FrameGeneration_x2");
	assert(EffectHelper::GetDisplayName(L"XeSSFG\\XeSS_MultiFrameGeneration_ZeroMV") == L"XeSS_MultiFrameGeneration");
	std::cout << "Effect picker search, purpose filters, exact legacy aliases and all 32 tier round trips passed.\n";
}
