#include "DlssOpticalFlowParameters.h"
#include "EffectParameterRules.h"
#include "MotionVectorRequest.h"
#include <cassert>
#include <iostream>
#include <limits>
#include <map>

namespace Magpie {
struct EffectOption { std::map<std::string, float, std::less<>> parameters; };
struct MotionSettings { MotionVectorRequest motionRequest; };
enum class EffectParameterRestartReason { FrameGuidance, ResourceRecreation };
struct DLSSNRFilter {
	struct Impl { bool disabled = false; };
	Impl* _impl = nullptr;
	MotionSettings _settings;
	FrameGuidanceRequirements GetFrameGuidanceRequirements() const noexcept;
	EffectParameterRestartReason GetParameterRestartReason(std::string_view) const noexcept;
};
struct DLSSFrameGenerator {
	MotionSettings _requestedSettings;
	FrameGuidanceRequirements GetFrameGuidanceRequirements() const noexcept;
};
}
#include "dlss_optical_flow_production.h"

struct StoredEffect {
	std::wstring name;
	std::map<std::wstring, float, std::less<>> parameters;
	int scalingType = 3;
	float scale = 1.5f;
};

int main() {
	using namespace Magpie;
	const auto nvBalanced = MotionVectorRequest::Nvidia(NvidiaOpticalFlowQuality::Balanced);
	for (auto id : {L"DLSSNR\\DLSSNR_AI_Filter", L"DLSSFG\\DLSS_FrameGeneration"}) {
		for (int quality = 0; quality <= 5; ++quality) {
			StoredEffect effect{id, {{L"motionVectorQuality", float(quality)}, {L"other", 0.25f}}};
			assert(MigrateDlssOpticalFlowParameters(effect));
			assert(effect.parameters.at(L"opticalFlowMethod") == (quality ? 2 : 0));
			assert(effect.parameters.at(L"nvidiaOpticalFlowQuality") == (quality ? quality : 2));
			assert(!effect.parameters.contains(L"motionVectorQuality"));
			assert(effect.parameters.at(L"other") == 0.25f && effect.scalingType == 3 && effect.scale == 1.5f);
			assert(!MigrateDlssOpticalFlowParameters(effect));
			EffectOption runtime;
			for (const auto& [name, value] : effect.parameters) {
				std::string key;
				for (wchar_t c : name) key.push_back(static_cast<char>(c));
				runtime.parameters[key] = value;
			}
			assert(ParseDlssOpticalFlowRequest(runtime) == MotionVectorRequest::Nvidia(
				static_cast<NvidiaOpticalFlowQuality>(quality)));
		}
		for (float enabled : {0.0f, 1.0f}) {
			StoredEffect legacy{id, {{L"useMotionVectors", enabled}}};
			assert(MigrateDlssOpticalFlowParameters(legacy));
			assert(legacy.parameters.at(L"opticalFlowMethod") == (enabled ? 2 : 0));
			assert(!legacy.parameters.contains(L"useMotionVectors"));
		}
		StoredEffect custom{std::wstring(id) + L"_Custom", {{L"motionVectorQuality", 4.0f}}};
		assert(!MigrateDlssOpticalFlowParameters(custom) && custom.parameters.size() == 1);
		StoredEffect fresh{id};
		assert(MigrateDlssOpticalFlowParameters(fresh));
		assert(fresh.parameters.at(L"opticalFlowMethod") == 2);
		StoredEffect selected{id, {{L"opticalFlowMethod", 1.0f}, {L"amdOpticalFlowMode", 0.0f},
			{L"motionVectorQuality", 0.0f}, {L"nvidiaOpticalFlowQuality", 5.0f}}};
		assert(MigrateDlssOpticalFlowParameters(selected));
		assert(selected.parameters.at(L"opticalFlowMethod") == 1 && selected.parameters.at(L"amdOpticalFlowMode") == 0);
		assert(selected.parameters.at(L"nvidiaOpticalFlowQuality") == 5);
		assert(!MigrateDlssOpticalFlowParameters(selected));
	}
	assert(ParseDlssOpticalFlowRequest({}) == nvBalanced);
	for (float invalid : {-1.0f, 9.0f, 1.5f, std::numeric_limits<float>::infinity(),
		std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::max()}) {
		assert(ParseDlssOpticalFlowRequest({{{"motionVectorQuality", invalid}}}) == nvBalanced);
		assert(ParseDlssOpticalFlowRequest({{{"motionVectorQuality", invalid}, {"useMotionVectors", 0.0f}}}) == nvBalanced);
		assert(ParseDlssOpticalFlowRequest({{{"opticalFlowMethod", invalid}}}) == nvBalanced);
		assert(ParseDlssOpticalFlowRequest({{{"opticalFlowMethod", 2.0f}, {"nvidiaOpticalFlowQuality", invalid}}}) == nvBalanced);
		assert(ParseDlssOpticalFlowRequest({{{"motionVectorQuality", 5.0f}, {"nvidiaOpticalFlowQuality", invalid}}}) == nvBalanced);
		assert(ParseDlssOpticalFlowRequest({{{"opticalFlowMethod", 1.0f}, {"amdOpticalFlowMode", invalid}}}) ==
			MotionVectorRequest::Amd(AmdOpticalFlowMode::Quality));
	}
	DLSSNRFilter::Impl impl;
	DLSSNRFilter nr{&impl};
	DLSSFrameGenerator fg;
	for (int method = 0; method <= 2; ++method) {
		for (int quality = (method == 2 ? 1 : 0); quality <= (method == 2 ? 5 : 1); ++quality) {
			const auto request = ParseDlssOpticalFlowRequest({{{"opticalFlowMethod", float(method)},
				{"amdOpticalFlowMode", float(quality)}, {"nvidiaOpticalFlowQuality", float(quality)}}});
			assert(request.method == static_cast<OpticalFlowMethod>(method));
			nr._settings.motionRequest = fg._requestedSettings.motionRequest = request;
			const auto nrReq = nr.GetFrameGuidanceRequirements();
			const auto fgReq = fg.GetFrameGuidanceRequirements();
			assert(nrReq == fgReq && nrReq.zero && nrReq.HasMotion() == (method != 0));
			if (method) assert(nrReq.Contains(request) && request.quality == quality);
			FrameGuidanceRequirements combined = nrReq;
			combined.Merge(fgReq);
			int providers = 0;
			combined.Resolved().ForEachMotion([&](auto) { ++providers; });
			assert(providers == (method != 0));
		}
	}
	impl.disabled = true;
	assert(!nr.GetFrameGuidanceRequirements().Any());
	FrameGuidanceRequirements mixed{.zero = true};
	mixed.Add(MotionVectorRequest::Amd(AmdOpticalFlowMode::Quality));
	mixed.Add(nvBalanced);
	assert(mixed.PreferredMotion() == nvBalanced);
	assert(FrameGuidanceRequirements::ResolveConsumer({}, nvBalanced) == MotionVectorRequest{});
	for (auto name : {"opticalFlowMethod", "amdOpticalFlowMode", "nvidiaOpticalFlowQuality"})
		assert(nr.GetParameterRestartReason(name) == EffectParameterRestartReason::FrameGuidance);
	for (auto id : {"DLSSNR\\DLSSNR_AI_Filter", "DLSSFG\\DLSS_FrameGeneration"}) {
		for (int method = 0; method <= 2; ++method) {
			auto get = [&](auto name, float fallback) { return std::string_view(name) == "opticalFlowMethod" ? float(method) : fallback; };
			assert(IsEffectParameterVisible(id, "amdOpticalFlowMode", get) == (method == 1));
			assert(IsEffectParameterVisible(id, "nvidiaOpticalFlowQuality", get) == (method == 2));
			assert(IsEffectParameterVisible(id, "opticalFlowMethod", get));
		}
	}
	assert(IsEffectParameterVisible("Custom", "amdOpticalFlowMode", [](auto, float fallback) { return fallback; }));
	std::cout << "DLSS optical flow: legacy migration, defaults, invalid values, AMD/NVIDIA/None routing, shared requests and parameter visibility passed.\n";
}
