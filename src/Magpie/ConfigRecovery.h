#pragma once
#include "ConfigPersistence.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <cmath>
#include <limits>
#include <functional>
#include <optional>

namespace Magpie::ConfigRecovery {

// Increment when the recovery policy changes, independently of release versions.
inline constexpr uint32_t POLICY_VERSION = 1;
enum class Kind { None, Backup, Partial, Defaults, Repaired };
struct Plan {
	rapidjson::Document document;
	Kind kind = Kind::None;
	std::vector<std::string> fields;
	bool defaultModes = false;
};
struct ParameterRule {
	float min, max, fallback;
	bool integer;
	std::vector<int> choices;
};
using ParameterRules = std::function<std::optional<ParameterRule>(std::string_view, std::string_view)>;

inline std::string Serialize(const rapidjson::Document& doc) {
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	return { buffer.GetString(), buffer.GetLength() };
}

inline bool FloatInRange(const rapidjson::Value& value, double min, double max) {
	return value.IsNumber() && std::isfinite(value.GetDouble()) &&
		value.GetDouble() >= min && value.GetDouble() <= max;
}

inline Plan Prepare(std::string_view source, std::string_view backup,
	std::string_view recoveredGroupName, const ParameterRules& rules = {}) {
	Plan plan;
	std::string input(source);
	if (!ConfigPersistence::IsValid(input)) {
		if (ConfigPersistence::IsValid(backup)) {
			input = backup;
			plan.kind = Kind::Backup;
		} else {
			input = ConfigPersistence::RecoverPrefix(source);
			plan.kind = input.empty() ? Kind::Defaults : Kind::Partial;
			if (input.empty()) input = "{}";
		}
	}
	auto& doc = plan.document;
	doc.Parse(input.data(), input.size());
	auto& allocator = doc.GetAllocator();
	auto note = [&](const std::string& path) {
		plan.fields.push_back(path);
		if (plan.kind == Kind::None) plan.kind = Kind::Repaired;
	};
	auto check = [&](rapidjson::Value& object, const char* key,
		const std::string& path, auto valid) {
		auto it = object.FindMember(key);
		if (it != object.MemberEnd() && !valid(it->value)) {
			note(path + "/" + key);
			object.EraseMember(it); // Missing fields use the existing application defaults.
		}
	};
	auto object = [](const auto& value) { return value.IsObject(); };
	auto array = [](const auto& value) { return value.IsArray(); };
	auto number = [&](rapidjson::Value& value, const char* key,
		const std::string& path, double min, double max) {
		check(value, key, path, [&](const auto& v) { return FloatInRange(v, min, max); });
	};
	auto enumeration = [&](rapidjson::Value& value, const char* key,
		const std::string& path, unsigned count) {
		check(value, key, path, [&](const auto& v) { return v.IsUint() && v.GetUint() < count; });
	};
	for (const char* key : { "windowPos", "shortcuts", "hotkeys", "overlay" })
		check(doc, key, "", object);
	for (const char* key : { "scalingModes", "profiles", "scalingProfiles" })
		check(doc, key, "", array);
	for (const auto& [key, version] : { std::pair{ "experimentalDlssnrSettingsVersion", 2u },
		std::pair{ "experimentalDlssSrSettingsVersion", 1u }, std::pair{ "experimentalDepthRemovalVersion", 1u } }) {
		if (!doc.HasMember(key) || !doc[key].IsUint() || doc[key].GetUint() < version) note(std::string("/") + key);
	}
	number(doc, "minFrameRate", "", 0, 1000);
	number(doc, "frontEdgeSyncFrameRate", "", 15, 360);
	enumeration(doc, "theme", "", 3);
	enumeration(doc, "duplicateFrameDetectionMode", "", 3);
	if (doc.HasMember("windowPos")) {
		auto& pos = doc["windowPos"];
		for (const char* key : { "centerX", "centerY", "x", "y" })
			number(pos, key, "/windowPos", -1e7, 1e7);
		for (const char* key : { "width", "height" }) number(pos, key, "/windowPos", -1, 1e7);
	}
	plan.defaultModes = !doc.HasMember("scalingModes");
	if (plan.defaultModes) note("/scalingModes");
	size_t modeCount = plan.defaultModes ? 6 : doc["scalingModes"].Size();
	if (!plan.defaultModes) {
		for (rapidjson::SizeType i = 0; i < doc["scalingModes"].Size(); ++i) {
			auto& mode = doc["scalingModes"][i];
			const std::string path = "/scalingModes/" + std::to_string(i);
			const bool invalidMode = !mode.IsObject();
			if (invalidMode) { mode.SetObject(); note(path); }
			if (!mode.HasMember("name") || !mode["name"].IsString()) {
				mode.RemoveMember("name");
				mode.AddMember("name", rapidjson::Value(recoveredGroupName.data(),
					static_cast<rapidjson::SizeType>(recoveredGroupName.size()), allocator), allocator);
				note(path + "/name");
			}
			// Empty groups are legitimate drafts. A malformed chain falls back only
			// within this group; keep its index so other profile selections stay valid.
			bool broken = invalidMode || (mode.HasMember("effects") && !mode["effects"].IsArray());
			if (!broken && mode.HasMember("effects")) {
				for (auto& effect : mode["effects"].GetArray()) {
					if (!effect.IsObject() || !effect.HasMember("name") ||
						!effect["name"].IsString() || !effect["name"].GetStringLength()) { broken = true; break; }
				}
			}
			if (broken) {
				mode.RemoveMember("effects");
				rapidjson::Value effects(rapidjson::kArrayType), effect(rapidjson::kObjectType);
				effect.AddMember("name", "Lanczos", allocator);
				effects.PushBack(effect, allocator);
				mode.AddMember("effects", effects, allocator);
				note(path + "/effects");
			}
			if (!mode.HasMember("effects")) continue;
			for (rapidjson::SizeType j = 0; j < mode["effects"].Size(); ++j) {
				auto& effect = mode["effects"][j];
				const std::string effectPath = path + "/effects/" + std::to_string(j);
				enumeration(effect, "scalingType", effectPath, 4);
				check(effect, "scale", effectPath, object);
				if (effect.HasMember("scale")) {
					auto& scale = effect["scale"];
					if (!scale.HasMember("x") || !scale.HasMember("y") ||
						!FloatInRange(scale["x"], 1e-6, 1e7) || !FloatInRange(scale["y"], 1e-6, 1e7)) {
						effect.RemoveMember("scale"); note(effectPath + "/scale");
					}
				}
				check(effect, "parameters", effectPath, object);
				if (effect.HasMember("parameters")) {
					auto& params = effect["parameters"];
					for (auto it = params.MemberBegin(); it != params.MemberEnd();) {
						if (!FloatInRange(it->value, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())) {
							note(effectPath + "/parameters/" + it->name.GetString());
							it = params.EraseMember(it);
						} else {
							// Only installed, recognized parameters have a contract. Keep
							// unavailable effects and unknown extension parameters intact.
							const auto rule = rules ? rules(effect["name"].GetString(), it->name.GetString()) : std::nullopt;
							if (rule) {
								const float value = it->value.GetFloat();
								const bool choiceValid = rule->choices.empty() || std::any_of(rule->choices.begin(),
									rule->choices.end(), [&](int choice) { return value == static_cast<float>(choice); });
								if (value < rule->min || value > rule->max ||
									(rule->integer && std::round(value) != value) || !choiceValid) {
									it->value.SetFloat(rule->fallback);
									note(effectPath + "/parameters/" + it->name.GetString());
								}
							}
							++it;
						}
					}
				}
			}
		}
	}
	for (const char* key : { "profiles", "scalingProfiles" }) {
		if (!doc.HasMember(key)) continue;
		auto& profiles = doc[key];
		for (rapidjson::SizeType i = 0; i < profiles.Size(); ++i) {
			auto& profile = profiles[i];
			const std::string path = std::string("/") + key + "/" + std::to_string(i);
			if (!profile.IsObject()) {
				note(path);
				if (i == 0) {
					profile.SetObject();
					if (modeCount) profile.AddMember("scalingMode", 0, allocator);
				}
				else continue; // Existing loader discards malformed custom rules.
			}
			const size_t previousRepairs = plan.fields.size();
			check(profile, "scalingMode", path, [&](const auto& v) {
				return v.IsInt() && v.GetInt() >= -1 &&
					(v.GetInt() == -1 || static_cast<size_t>(v.GetInt()) < modeCount);
			});
			if (plan.defaultModes || (i == 0 && previousRepairs != plan.fields.size() && modeCount)) {
				profile.RemoveMember("scalingMode");
				profile.AddMember("scalingMode", i == 0 ? 0 : -1, allocator);
			}
			number(profile, "maxFrameRate", path, 10, 1000);
			number(profile, "customInitialWindowedScaleFactor", path, 1, 1e4);
			number(profile, "customCursorScaling", path, 0, 1e4);
			number(profile, "autoHideCursorDelay", path, 0.1, 5);
			enumeration(profile, "captureMethod", path, 4);
			enumeration(profile, "captureMode", path, 4);
			enumeration(profile, "initialWindowedScaleFactor", path, 8);
			enumeration(profile, "cursorScaling", path, 8);
			enumeration(profile, "cursorInterpolationMode", path, 2);
			enumeration(profile, "destAlignment", path, 9);
			check(profile, "cropping", path, object);
			if (profile.HasMember("cropping")) {
				for (const char* edge : { "left", "top", "right", "bottom" })
					number(profile["cropping"], edge, path + "/cropping", 0, 1e7);
			}
		}
	}
	return plan;
}

struct Files { std::filesystem::path original, result; };
inline Files FilesFor(const std::filesystem::path& config, std::string_view source) {
	// Not a security hash: always compare the exact saved bytes before reuse.
	uint64_t hash = 14695981039346656037ULL;
	for (unsigned char c : source) { hash ^= c; hash *= 1099511628211ULL; }
	const auto stem = config.native() + L".recovery-v" + std::to_wstring(POLICY_VERSION) +
		L"-" + std::to_wstring(hash) + L"-" + std::to_wstring(source.size());
	return { stem + L".original", stem + L".json" };
}

inline bool Preserve(const std::filesystem::path& config, std::string_view source, const Files& files) {
	if (!CopyFileW(config.c_str(), files.original.c_str(), TRUE) && GetLastError() != ERROR_FILE_EXISTS)
		return false;
	if (ConfigPersistence::Read(files.original) != source) { SetLastError(ERROR_INVALID_DATA); return false; }
	return true;
}

} // namespace Magpie::ConfigRecovery
