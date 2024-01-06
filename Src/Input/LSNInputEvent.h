/**
 * Copyright L. Spiro 2024
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: An input event for triggering a button press in a game.  An input event can come from one of different device types (keyboard or USB controller),
 *	come from a specific device instance of that type (a specific USB controller model), be one of multiple types (axis, POV, or button), and come from one of
 *	multiple instances (axis index, POV index, or button index).
 */


#pragma once

#include "../LSNLSpiroNes.h"
#include "LSNUsbControllerBase.h"
#include <Helpers/LSWHelpers.h>

namespace lsn {

	/**
	 * Class LSN_INPUT_EVENT
	 * \brief An input event for triggering a button press in a game.
	 *
	 * Description: An input event for triggering a button press in a game.  An input event can come from one of different device types (keyboard or USB controller),
	 *	come from a specific device instance of that type (a specific USB controller model), be one of multiple types (axis, POV, or button), and come from one of
	*	multiple instances (axis index, POV index, or button index).
	 */
	struct LSN_INPUT_EVENT {
		// == Enumerations.
		/** Device types. */
		enum LSN_DEVICE_TYPE : uint8_t {
			LSN_DT_KEYBOARD,																/**< The event comes from the system keyboard. */
			LSN_DT_USB_CONTROLLER,															/**< The event comes from a USB controller. */
		};


		// == Members.
		LSN_DEVICE_TYPE										dtType;							/**< The device type (keyboard or USB controller. */

		union {
			struct {
				lsw::LSW_KEY								kKey;							/**< If LSN_DEVICE_TYPE is LSN_DT_KEYBOARD. */
			}												kb;
			struct {
				GUID										guId;							/**< The USB controller's product ID. */
				CUsbControllerBase::LSN_INPUT_EVENT			ieEvent;						/**< The controller event */
			}												cont;
		}													u;
	};

}	// namespace lsn
