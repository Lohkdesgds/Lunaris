#pragma once

#include "../../Tools/Resource/resource.h"
#include "../../Interface/Vertex/vertex.h"
#include "../Sprite/sprite.h"

#undef max

namespace LSW {
	namespace v5 {
		namespace Work {

			namespace progressbar {

				enum class e_color {
					BORDER,
					FOREGROUND_TOP_LEFT,
					FOREGROUND_TOP_RIGHT,
					FOREGROUND_BOTTOM_LEFT,
					FOREGROUND_BOTTOM_RIGHT,
					BACKGROUND_TOP_LEFT,
					BACKGROUND_TOP_RIGHT,
					BACKGROUND_BOTTOM_LEFT,
					BACKGROUND_BOTTOM_RIGHT
				};

				enum class e_boolean {
					FIXED_BACKGROUND
				};

				enum class e_double_readonly {
					PROGRESS_SMOOTH // the progress shown, smooth value
				};

				enum class e_double {
					PROGRESS, // [0.0, 1.0]
					SMOOTHNESS, // [0.0, inf), if 5, (1 * curr + 5 * old) / (5 + 1)
					BORDER_THICKNESS, // has to greater than 0 to enable border
					BORDER_ROUNDNESS, // has to be balanced with THICKNESS
					BORDER_PROPORTION_X, // 1.0 = 1:1, more is offset to out
					BORDER_PROPORTION_Y // 1.0 = 1:1, more is offset to out
				};

				const Tools::SuperMap<Tools::FastFunction<Interface::Color>> e_vertex_defaults = {
					{Interface::Color{0.1f,0.08f,0.2f},									(e_color::BORDER),						("progressbar:border")},
					{Interface::Color{0.9f,0.9f,0.9f},									(e_color::FOREGROUND_TOP_LEFT),			("progressbar:foreground_top_left")},
					{Interface::Color{1.0f,1.0f,1.0f},									(e_color::FOREGROUND_TOP_RIGHT),		("progressbar:foreground_top_right")},
					{Interface::Color{0.9f,0.9f,0.9f},									(e_color::FOREGROUND_BOTTOM_LEFT),		("progressbar:foreground_bottom_left")},
					{Interface::Color{1.0f,1.0f,1.0f},									(e_color::FOREGROUND_BOTTOM_RIGHT),		("progressbar:foreground_bottom_right")},
					{Interface::Color{0.1f,0.1f,0.1f},									(e_color::BACKGROUND_TOP_LEFT),			("progressbar:background_top_left")},
					{Interface::Color{0.07f,0.07f,0.07f},								(e_color::BACKGROUND_TOP_RIGHT),		("progressbar:background_top_right")},
					{Interface::Color{0.1f,0.1f,0.1f},									(e_color::BACKGROUND_BOTTOM_LEFT),		("progressbar:background_bottom_left")},
					{Interface::Color{0.07f,0.07f,0.07f},								(e_color::BACKGROUND_BOTTOM_RIGHT),		("progressbar:background_bottom_right")}
				};

				const Tools::SuperMap<Tools::FastFunction<bool>> e_boolean_defaults = {
					{false,																(e_boolean::FIXED_BACKGROUND),			("progressbar:fixed_background")}
				};

				const Tools::SuperMap<Tools::FastFunction<double>> e_double_defaults = {
					{0.0,																(e_double_readonly::PROGRESS_SMOOTH),	("progressbar:progress_smooth")},
					{0.0,																(e_double::PROGRESS),					("progressbar:progress")},
					{3.0,																(e_double::SMOOTHNESS),					("progressbar:smoothness")},
					{0.04,																(e_double::BORDER_THICKNESS),			("progressbar:border_thickness")},
					{0.01,																(e_double::BORDER_ROUNDNESS),			("progressbar:border_roundness")},
					{1.012,																(e_double::BORDER_PROPORTION_X),		("progressbar:border_proportion_x")},
					{1.03,																(e_double::BORDER_PROPORTION_Y),		("progressbar:border_proportion_y")}
				};


			}

			/// <summary>
			/// <para>Easy to use direct Progress Bar so you can easily print a bar.</para>
			/// </summary>
			class ProgressBar : public Sprite_Base {

				// val, min, max
				double max(const double, const double = -1.0, const double = 1.0);
				// return the value beyond a val both - +
				double beyond(const double, const double);

				void draw_task(Interface::Camera&);
			public:
				using Sprite_Base::set;
				using Sprite_Base::get;
				using Sprite_Base::get_direct;

				/// <summary>
				/// <para>Empty default start.</para>
				/// </summary>
				ProgressBar();

				/// <summary>
				/// <para>Constructor referencing a ProgressBar (not a copy).</para>
				/// </summary>
				/// <param name="{ProgressBar}">The one to reference attributes from (no copy).</param>
				ProgressBar(const ProgressBar&);

				/// <summary>
				/// <para>Constructor to move a ProgressBar to this (move).</para>
				/// </summary>
				/// <param name="{ProgressBar}">The one to move attributes from.</param>
				ProgressBar(ProgressBar&&);

				/// <summary>
				/// <para>Reference a ProgressBar (not a copy).</para>
				/// </summary>
				/// <param name="{ProgressBar}">The one to reference attributes from (no copy).</param>
				void operator=(const ProgressBar&);

				/// <summary>
				/// <para>Move a ProgressBar to this (move).</para>
				/// </summary>
				/// <param name="{ProgressBar}">The one to move attributes from.</param>
				void operator=(ProgressBar&&);

				/// <summary>
				/// <para>Clone other ProgressBar attributes.</para>
				/// <para>You can also clone a specific type by doing set(*get()).</para>
				/// </summary>
				/// <param name="{ProgressBar}">Other to copy from.</param>
				void clone(const ProgressBar&);
			};

		}
	}
}