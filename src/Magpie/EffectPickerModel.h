#pragma once
#include <algorithm>
#include <cwctype>
#include <string>
#include <string_view>
#include <vector>
#include "RTXVideoParameters.h"

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

}
