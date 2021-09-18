#pragma once

#include "../../Tools/Common/common.h"
#include "../../Interface/Target/target.h"
#include "../../Interface/Vertex/vertex.h"
#include "../Sprite/sprite.h"

namespace LSW {
	namespace v5 {
		namespace Work {
			
			namespace shinefx {

				enum class e_chronomillis_readonly {
					LAST_FRAME, // Last time it has drawn
					LAST_TIE_FRAME_VERIFICATION // Last time it tested for screen's resolution so it can resize self to it (if tied to display)
				};

				enum class e_double { 
					SPEED_ROTATION_TIME, // multiplier of time for rotation
					EACH_SIZE_RAD, // how big each triangle should be? [0, 2PI] (recommended: proportion of PI)
					FRAMES_PER_SECOND, // how many frames per second update time
					TIED_SIZE_TO_DISPLAY_PROPORTION,  // proportion of pixels compared to display, 1.0 means 1:1
					CENTER_X, // center of animation
					CENTER_Y // center of animation
				};

				enum class e_color { 
					BACKGROUND, // background color
					FOREGROUND  // foreground color
				};

				const Tools::SuperMap<Tools::FastFunction<double>>								e_double_defaults = {
					{1.0,								(e_double::SPEED_ROTATION_TIME),							"shinefx:speed_rotation_time"},
					{0.1 * ALLEGRO_PI,					(e_double::EACH_SIZE_RAD),									"shinefx:each_size_rad"},
					{60.0,							    (e_double::FRAMES_PER_SECOND),								"shinefx:frames_per_second"},
					{1.0,								(e_double::TIED_SIZE_TO_DISPLAY_PROPORTION),				"shinefx:tied_size_to_display_proportion"},
					{0.0,								(e_double::CENTER_X),										"shinefx:center_x"},
					{0.0,								(e_double::CENTER_Y),										"shinefx:center_y"}
				};
				const Tools::SuperMap<Tools::FastFunction<Interface::Color>>					e_color_defaults = {
					{Interface::Color(0, 0, 0),			(e_color::BACKGROUND),										"shinefx:background"},
					{Interface::Color(128, 128, 128),	(e_color::FOREGROUND),										"shinefx:foreground"}
				};
				const Tools::SuperMap<Tools::FastFunction<std::chrono::milliseconds>>			e_chronomillis_defaults = {
					{std::chrono::milliseconds(0),		(e_chronomillis_readonly::LAST_FRAME),						"shinefx:last_frame"},
					{std::chrono::milliseconds(0),		(e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION),		"shinefx:last_tie_verification"}
				};

				constexpr auto default_delta_t_frame_delay = std::chrono::seconds(5); // don't need to be fast (see also block::...)
			}

			class ShineFX : public Sprite_Base, public Tools::SuperFunctionalMap<std::chrono::milliseconds> {
				Interface::Bitmap buffer;

				void check_internal();

			protected:
				void draw_task(Interface::Camera&) override;
			public:
				using Sprite_Base::set;
				using Sprite_Base::get;
				using Sprite_Base::get_direct;
				using Tools::SuperFunctionalMap<std::chrono::milliseconds>::set;
				using Tools::SuperFunctionalMap<std::chrono::milliseconds>::get;
				using Tools::SuperFunctionalMap<std::chrono::milliseconds>::get_direct;

				/// <summary>
				/// <para>Empty default start.</para>
				/// </summary>
				ShineFX();

				/// <summary>
				/// <para>Constructor referencing a ShineFX (not a copy).</para>
				/// </summary>
				/// <param name="{ShineFX}">The one to reference attributes from (no copy).</param>
				ShineFX(const ShineFX&);

				/// <summary>
				/// <para>Constructor to move a ShineFX to this (move).</para>
				/// </summary>
				/// <param name="{ShineFX}">The one to move attributes from.</param>
				ShineFX(ShineFX&&) noexcept;

				/// <summary>
				/// <para>Reference a ShineFX (not a copy).</para>
				/// </summary>
				/// <param name="{ShineFX}">The one to reference attributes from (no copy).</param>
				void operator=(const ShineFX&);

				/// <summary>
				/// <para>Move a ShineFX to this (move).</para>
				/// </summary>
				/// <param name="{ShineFX}">The one to move attributes from.</param>
				void operator=(ShineFX&&) noexcept;

				/// <summary>
				/// <para>Clone other ShineFX attributes.</para>
				/// <para>You can also clone a specific type by doing set(*get()).</para>
				/// </summary>
				/// <param name="{ShineFX}">Other to copy from.</param>
				void clone(const ShineFX&);
			};
		}
	}
}