/**
 * Copyright L. Spiro 2023
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: Emulator options.
 */


#pragma once

#include "../LSNLSpiroNes.h"
#include "LSNInputOptions.h"



namespace lsn {

	/**
	 * Class LSN_OPTIONS
	 * \brief Emulator options.
	 *
	 * Description: Emulator options.
	 */
	struct LSN_OPTIONS {
		LSN_INPUT_OPTIONS									ioGlobalInputOptions;										/**< Global input options. */
		LSN_INPUT_OPTIONS									ioThisGameInputOptions;										/**< This game's input options. */

		std::wstring										wInRomInitPath;												/**< The initial directory for the input ROM. */
		std::wstring										wOutRomInitPath;											/**< The initial directory for the output ROM. */
		std::wstring										wPatchInitPath;												/**< The initial directory for the patch file. */
	};

}	// namespace lsn
