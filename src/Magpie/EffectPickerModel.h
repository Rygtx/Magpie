#pragma once
#include <algorithm>
#include <cwctype>
#include <string>
#include <string_view>
#include <vector>

namespace Magpie {

struct EffectPickerEntry {
	std::wstring id, name, category, subcategory, summary, details, searchText;
	std::vector<std::wstring> purposes;
	std::wstring recommendation;
	bool firstTry = false;
};

struct EffectPickerCategory {
	std::wstring id, name, description;
	std::vector<std::pair<std::wstring, std::wstring>> subcategories;
};

inline std::wstring NormalizeEffectSearch(std::wstring_view text) {
	std::wstring result;
	for (wchar_t c : text) {
		if (c == L'_' || c == L'\\' || c == L'/' || c == L'-' || std::iswspace(c)) continue;
		result += static_cast<wchar_t>(std::towlower(c));
	}
	return result;
}

inline bool MatchesEffectSearch(std::wstring_view normalizedText, std::wstring_view query) {
	// Each whitespace-separated term may match anywhere, including a legacy alias.
	size_t start = 0;
	while (start < query.size()) {
		while (start < query.size() && std::iswspace(query[start])) ++start;
		size_t end = start;
		while (end < query.size() && !std::iswspace(query[end])) ++end;
		if (normalizedText.find(NormalizeEffectSearch(query.substr(start, end - start))) == std::wstring_view::npos) return false;
		start = end;
	}
	return true;
}

inline bool MatchesEffectCategory(const EffectPickerEntry& entry,
	std::wstring_view category, std::wstring_view subcategory) {
	if (category.empty()) return true;
	if (category == L"first_try") return entry.firstTry;
	if (category != entry.category && std::ranges::find(entry.purposes, category) == entry.purposes.end()) return false;
	return subcategory.empty() || (entry.category == category && entry.subcategory == subcategory);
}

// These are exact aliases, never prefix rewrites of custom effects.
inline constexpr std::wstring_view RTX_VIDEO_IDS[2][4] = {
	{ L"RTXVideo\\RTXVideo_Denoise_Low", L"RTXVideo\\RTXVideo_Denoise_Medium",
	  L"RTXVideo\\RTXVideo_Denoise_High", L"RTXVideo\\RTXVideo_Denoise_Ultra" },
	{ L"RTXVideo\\RTXVideo_VSR_Low", L"RTXVideo\\RTXVideo_VSR_Medium",
	  L"RTXVideo\\RTXVideo_VSR_High", L"RTXVideo\\RTXVideo_VSR_Ultra" }
};

inline int RTXVideoFamily(std::wstring_view id) {
	for (int family = 0; family < 2; ++family) {
		for (auto candidate : RTX_VIDEO_IDS[family]) if (id == candidate) return family;
	}
	return -1;
}

inline int RTXVideoStrength(std::wstring_view id) {
	const int family = RTXVideoFamily(id);
	if (family < 0) return -1;
	for (int i = 0; i < 4; ++i) if (RTX_VIDEO_IDS[family][i] == id) return i;
	return -1;
}

inline std::wstring_view RTXVideoId(std::wstring_view currentId, int strength) {
	const int family = RTXVideoFamily(currentId);
	return family >= 0 && strength >= 0 && strength < 4 ? RTX_VIDEO_IDS[family][strength] : std::wstring_view{};
}

}
