

/**
 * Copyright L. Spiro 2023
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: Provides a grouping of all of the window-/UI- specific settings, such as hotkeys, show/hide menu, etc.
 */

#pragma once

#include "../LSNLSpiroNes.h"

#ifdef LSN_USE_WINDOWS
#include <Helpers/LSWInputListenerBase.h>
typedef lsw::LSW_KEY															CUiKey;
#define LSN_DEFAULT_BEK_KEYMOD													0x00
#define LSN_DEFAULT_BEK_KEYCODE													VK_ESCAPE
#define LSN_DEFAULT_BEK_SCANCODE												0x00010001
#define LSN_KEY_INIT( SCANCODE, KEYCODE, KEYMOD )								{ .dwScanCode = DWORD( SCANCODE ), .bKeyCode = BYTE( KEYCODE ), .bKeyModifier = BYTE( KEYMOD ) }
#else
#endif	// #ifdef LSN_USE_WINDOWS



namespace lsn {

	/**
	 * Class LSN_WINDOW_OPTIONS
	 * \brief Provides a grouping of all of the window-/UI- specific settings, such as hotkeys, show/hide menu, etc.
	 *
	 * Description: Provides a grouping of all of the window-/UI- specific settings, such as hotkeys, show/hide menu, etc.
	 */
	struct LSN_WINDOW_OPTIONS {
		/** The key to get out of borderless mode. */
		CUiKey								ukBorderlessExitKey					= LSN_KEY_INIT( LSN_DEFAULT_BEK_SCANCODE, LSN_DEFAULT_BEK_KEYCODE, LSN_DEFAULT_BEK_KEYMOD );
		/** Enter borderless mode on maximize? */
		bool								bGoBorderless						= true;
		/** Hide the menu in borderless maximized mode? */
		bool								bBorderlessHidesMenu				= true;
	};

}	// namespace lsn
