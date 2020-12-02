#pragma once

#include "../../Tools/Common/common.h"
#include "../../Tools/Resource/resource.h"
#include "../../Interface/Bitmap/bitmap.h"
#include "../../Interface/Target/target.h"
#include "../Sprite/sprite.h"

#undef max

namespace LSW {
	namespace v5 {
		namespace Work {

			namespace block {

				enum class e_chronomillis_readonly { 
					LAST_FRAME//, // last time a frame was drawn
					//LAST_TIE_FRAME_VERIFICATION // last time it verified reference bitmap size (target) 
				};
				enum class e_uintptr_t { FRAME };
				enum class e_boolean { SET_FRAME_VALUE_READONLY };
				enum class e_double { FRAMES_PER_SECOND/*, TIE_SIZE_TO_DISPLAY_PROPORTION*/ };

				const Tools::SuperMap<Tools::FastFunction<std::chrono::milliseconds>>		e_chronomillis_defaults = {
					{std::chrono::milliseconds(0),																		(e_chronomillis_readonly::LAST_FRAME),					("last_frame")},
					//{std::chrono::milliseconds(0),																		(e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION),	("last_tie_verification")}
				};
				const Tools::SuperMap<Tools::FastFunction<uintptr_t>>		e_uintptr_t_defaults = {
					{(uintptr_t)0,																						(e_uintptr_t::FRAME),									("frame")}
				};
				const Tools::SuperMap<Tools::FastFunction<bool>>		e_boolean_defaults = {
					{false,																								(e_boolean::SET_FRAME_VALUE_READONLY),					("set_frame_readonly")}
				};
				const Tools::SuperMap<Tools::FastFunction<double>>		e_double_defaults = {
					{10.0,																								(e_double::FRAMES_PER_SECOND),							("frames_per_second")}//, // FPS, not delta sec
					//{0.0,																								(e_double::TIE_SIZE_TO_DISPLAY_PROPORTION),				("tie_size_to_display_proportion")}
				};

				constexpr size_t maximum_bitmap_amount = static_cast<size_t>(std::numeric_limits<int>::max());
				constexpr size_t static_frame = static_cast<size_t>(-1); // static frame X?
				constexpr auto default_delta_t_frame_delay = std::chrono::seconds(5); // don't need to be fast (see also bubblefx::...)
				constexpr size_t max_frames_behind = 30;
			}


			/// <summary>
			/// <para>A Sprite_Base with Bitmaps.</para>
			/// <para>If you use custom index, please change the sprite::e_uintptr_t::INDEX_TARGET_IN_USE.</para>
			/// </summary>
			class Block : public Sprite_Base, public Tools::SuperFunctionalMap<std::chrono::milliseconds> {

				// bitmap internally uses shared_ptr so no worries!
				std::vector<Interface::Bitmap> bitmaps;

				//Interface::Bitmap reference;

				void draw_task(Interface::Camera&);
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
				Block();

				/// <summary>
				/// <para>Constructor referencing a Block (not a copy).</para>
				/// </summary>
				/// <param name="{Block}">The one to reference attributes from (no copy).</param>
				Block(const Block&);

				/// <summary>
				/// <para>Constructor to move a Block to this (move).</para>
				/// </summary>
				/// <param name="{Block}">The one to move attributes from.</param>
				Block(Block&&) noexcept;

				/// <summary>
				/// <para>Reference a Block (not a copy).</para>
				/// </summary>
				/// <param name="{Block}">The one to reference attributes from (no copy).</param>
				void operator=(const Block&);

				/// <summary>
				/// <para>Move a Block to this (move).</para>
				/// </summary>
				/// <param name="{Block}">The one to move attributes from.</param>
				void operator=(Block&&) noexcept;

				/// <summary>
				/// <para>Clone other Block attributes.</para>
				/// <para>You can also clone a specific type by doing set(*get()).</para>
				/// </summary>
				/// <param name="{Block}">Other to copy from.</param>
				void clone(const Block&);

				/// <summary>
				/// <para>Clone other Block Bitmaps.</para>
				/// </summary>
				/// <param name="{Block}">Other to copy from.</param>
				void clone_bitmaps(const Block&);

				/// <summary>
				/// <para>Add a Bitmap to be drawn here (more than 1 = "slideshow").</para>
				/// </summary>
				/// <param name="{Bitmap}">A Bitmap.</param>
				void insert(const Interface::Bitmap&);

				/// <summary>
				/// <para>Remove Bitmap(s) inserted before.</para>
				/// </summary>
				/// <param name="{std::function}">A match function. If true, it removes the match, else keep.</param>
				/// <returns>{size_t} How many were matched.</returns>
				size_t remove(const std::function<bool(const Interface::Bitmap&)>);
			};

		}
	}
}