#pragma once

#include <allegro5/allegro5.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_physfs.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_ttf.h>

namespace LSW {
	namespace v5 {
		namespace Handling {

			/// <summary>
			/// <para>Initializes basics.</para>
			/// <para>[al_init]</para>
			/// </summary>
			/// <returns>{bool} Success</returns>
			bool init_basic();

			/// <summary>
			/// <para>Initializes the basic part of the Allegro library.</para>
			/// <para>[init_basic;al_init_acodec_addon;al_install_audio;al_reserve_samples]</para>
			/// </summary>
			/// <param name="{unsigned short}">Amount of samples</param>
			/// <returns>{bool} Success</returns>
			bool init_audio(const unsigned short = 4);

			/// <summary>
			/// <para>Initializes font (normal and ttf) parts.</para>
			/// <para>[init_basic;al_init_font_addon;al_init_ttf_addon]</para>
			/// </summary>
			/// <returns>{bool} Success</returns>
			bool init_font();

			/// <summary>
			/// <para>Initializes image and primitives parts.</para>
			/// <para>[init_basic;al_init_image_addon;al_init_primitives_addon]</para>
			/// </summary>
			/// <returns>{bool} Success</returns>
			bool init_graphics();

			/// <summary>
			/// <para>Installs keyboard part.</para>
			/// <para>[init_basic;al_install_keyboard]</para>
			/// </summary>
			/// <returns>{bool} Success</returns>
			bool init_keyboard();

			/// <summary>
			/// <para>Installs mouse part.</para>
			/// <para>[init_basic;al_install_mouse]</para>
			/// </summary>
			/// <returns>{bool} Success</returns>
			bool init_mouse();

			/// <summary>
			/// <para>Installs joypad/joystick/controller part.</para>
			/// <para>[init_basic;al_install_joystick]</para>
			/// </summary>
			/// <returns>{bool} Success</returns>
			bool init_joypad();

			/// <summary>
			/// <para>Installs touchscreen part.</para>
			/// <para>[init_basic;al_install_touch_input]</para>
			/// </summary>
			/// <returns>{bool} Success</returns>
			bool init_touch();

#ifndef LSW_NO_PRELOAD
			// Pre-load everything (from libraries)
			class __pre_load_auto {
			public:
				__pre_load_auto();
			};

			static const __pre_load_auto __pre_load;
#endif
		}

	}
}