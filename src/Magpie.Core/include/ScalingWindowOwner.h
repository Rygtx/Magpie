#pragma once
#include <windows.h>

namespace Magpie {

// Call on the scaling window's thread. Keep windows unowned during potentially
// blocking initialization/teardown; input-queue detachment alone is insufficient.
inline bool SetScalingWindowOwner(HWND window, HWND owner) noexcept {
	SetLastError(ERROR_SUCCESS);
	const LONG_PTR previous = SetWindowLongPtrW(window, GWLP_HWNDPARENT,
		reinterpret_cast<LONG_PTR>(owner));
	return previous != 0 || GetLastError() == ERROR_SUCCESS;
}

}
