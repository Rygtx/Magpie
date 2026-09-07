#pragma once
#include <windows.h>

namespace Magpie {

// These are uncropped, physical screen coordinates. Use the monitor bounds,
// not its work area or the future scaling window's position and size.
inline bool SourceWindowCoversMonitor(
	const RECT& frameRect, const RECT& clientRect, const RECT& monitorRect
) noexcept {
	if (monitorRect.left >= monitorRect.right || monitorRect.top >= monitorRect.bottom) {
		return false;
	}
	const auto covers = [&monitorRect](const RECT& rect) noexcept {
		return rect.left <= monitorRect.left && rect.top <= monitorRect.top &&
			rect.right >= monitorRect.right && rect.bottom >= monitorRect.bottom;
	};
	return covers(frameRect) || covers(clientRect);
}

}
