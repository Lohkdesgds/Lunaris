#pragma once

#include "../../Tools/SuperFunctionalMap/superfunctionalmap.h"
#include "../../Interface/Events/events.h"
#include "../../Interface/EventTimer/eventtimer.h"
#include "../Button/button.h"

#undef max

namespace LSW {
	namespace v5 {
		namespace Work {

			namespace textinput {
				enum class e_boolean_readonly {
					SELECTED, 
					DISABLE_BLINK_TEMP
				};

				enum class e_cstring_readonly {
					BUFFER 
				};

				enum class e_boolean { 
					ENTER_BREAK_LINE,
					NON_AXIS_CANCEL_READING,
					NO_COLOR_TRANSLATION
				};

				enum class e_tie_functional {
					SAVED_STRING = 0 // expect Cstring
				};

				const Tools::SuperMap<Tools::FastFunction<bool>>		e_boolean_defaults = {
					{false,							(e_boolean_readonly::SELECTED),						("textinput:selected")},
					{false,							(e_boolean_readonly::DISABLE_BLINK_TEMP),			("textinput:disable_blink_temp")},
					{false,							(e_boolean::ENTER_BREAK_LINE),						("textinput:enter_break_line")},
					{false,							(e_boolean::NON_AXIS_CANCEL_READING),				("textinput:non_axis_cancel_reading")},
					{false,							(e_boolean::NO_COLOR_TRANSLATION),					("textinput:no_color_translation")}
				};
				const Tools::SuperMap<Tools::FastFunction<Tools::Cstring>>	e_cstring_defaults = {
					{Tools::Cstring(),				(e_cstring_readonly::BUFFER),						("textinput:buffer")}
				};
				const Tools::SuperMap<Tools::FastFunction<sprite::functional>> e_functional_defaults = {
					{sprite::functional(),			(e_tie_functional::SAVED_STRING)}
				};

			}

			/// <summary>
			/// <para>TextInput is a Button with a keyboard and mouse events already linked so you just click and type, or click somewhere else.</para>
			/// <para>The Block will handle the events, while Text shows what you type.</para>
			/// <para>Because of the multithread feature, you cannot reference or move this.</para>
			/// <para>The textinput values are saved into the TEXT parent part because of the CSTRING data. Access it with no problem using main()!</para>
			/// <para>———————————————————————————————————————————————————————</para>
			/// <para># Things you SHOULD NOT change:</para>
			/// <para>- text::e_sprite_ref::FOLLOWING (Text part)</para>
			/// <para>- text::e_cstring::STRING (Text part, it will be overwritten)</para>
			/// </summary>
			class TextInput : protected Button {
				Interface::Event kbev, msev;
				Interface::EventHandler event_handler;

				void _apply_latest();
				void _rec_string();

				void handle_event(const Interface::RawEvent&);

				void mouse_event(const sprite::e_tie_functional, const Tools::Any&) override;
			public:
				using Button::clone;
				using Block::collide;
				using Button::draw;
				using Button::get_block;
				using Button::get_text;
				using Block::update_and_clear;

				/// <summary>
				/// <para>Gets the variable with textinput data.</para>
				/// </summary>
				/// <returns>{Text*} Current holder of its data.</returns>
				Text& main();

				/// <summary>
				/// <para>Gets the variable with textinput data.</para>
				/// </summary>
				/// <returns>{Text*} Current holder of its data.</returns>
				const Text& main() const;

				/// <summary>
				/// <para>As reading has several steps, this function was created.</para>
				/// </summary>
				/// <param name="{bool}">True to force "reading mode" (Like if you clicked it).</param>
				void set_reading(const bool);

				TextInput();
				TextInput(TextInput&&) = delete;
				TextInput(const TextInput&&) = delete;
				void operator=(TextInput&&) = delete;
				void operator=(const TextInput&&) = delete;
			};

		}
	}
}