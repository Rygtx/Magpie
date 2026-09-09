#pragma once
#include <windows.h>
#include <filesystem>
#include <utility>

namespace Magpie::ConfigLocations {

struct Selection {
	std::filesystem::path destination;
	std::filesystem::path source;
	DWORD error = ERROR_SUCCESS;
	bool portable = false;
};

inline std::filesystem::path Directory(const std::filesystem::path& exe,
	const std::filesystem::path& localAppData, bool portable) {
	return portable ? exe / L"config/v4e" : localAppData / L"Magpie/config/v4e";
}

// An unreadable existing file is not absence. Never silently select an older
// version or overwrite an inaccessible current configuration.
inline Selection Select(const std::filesystem::path& exe, const std::filesystem::path& localAppData) {
	const auto portable = Directory(exe, localAppData, true) / L"config.json";
	const auto user = Directory(exe, localAppData, false) / L"config.json";
	Selection result{user};
	const std::pair<std::filesystem::path, bool> candidates[]{
		{portable, true}, {user, false}, {user.native() + L".bak", false},
		{exe / L"config/config.json", true},
		{localAppData / L"Magpie/config/v4/config.json", false},
		{localAppData / L"Magpie/config/v4/config.json.bak", false}
	};
	for (const auto& [path, isPortable] : candidates) {
		const DWORD attributes = GetFileAttributesW(path.c_str());
		const DWORD error = attributes == INVALID_FILE_ATTRIBUTES ? GetLastError() :
			(attributes & FILE_ATTRIBUTE_DIRECTORY) ? ERROR_DIRECTORY : ERROR_SUCCESS;
		if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) continue;
		result.source = path;
		result.portable = isPortable;
		result.destination = isPortable ? portable : user;
		result.error = error;
		break;
	}
	return result;
}

}
