#include "initialize.h"

namespace LSW {
	namespace v5 {
		namespace Handling {

			bool init_basic() {
				if (!al_is_system_installed()) return al_init();
				return true;
			}
			bool init_audio(const unsigned short samples) {
				if (!al_is_acodec_addon_initialized()) {
					return init_basic() && al_init_acodec_addon() && al_install_audio() && al_reserve_samples(samples);
				}
				return true;
			}
			bool init_font() {
				if (!al_is_font_addon_initialized()) {
					return init_basic() && al_init_font_addon() && al_init_ttf_addon();
				}
				return true;
			}
			bool init_graphics() {
				if (!al_is_image_addon_initialized()) {
					return init_basic() && al_init_image_addon() && al_init_primitives_addon();
				}
				return true;
			}
			bool init_keyboard() {
				if (!al_is_keyboard_installed()) {
					return init_basic() && al_install_keyboard();
				}
				return true;
			}
			bool init_mouse() {
				if (!al_is_mouse_installed()) {
					return init_basic() && al_install_mouse();
				}
				return true;
			}
			bool init_joypad() {

				if (!al_is_joystick_installed()) {
					return init_basic() && al_install_joystick();
				}
				return true;
			}
			bool init_touch() {

				if (!al_is_joystick_installed()) {
					return init_basic() && al_install_touch_input();
				}
				return true;
			}
		}
	}
}