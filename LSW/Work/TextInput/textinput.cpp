#include "textinput.h"

namespace LSW {
	namespace v5 {
		namespace Work {

			void TextInput::_apply_latest()
			{
				Text* here_i_am = ((Text*)this);
				const auto limited = here_i_am->get_direct<bool>(text::e_boolean_readonly::REACHED_LIMIT);
				auto last_buf = here_i_am->get_direct<Tools::Cstring>(textinput::e_cstring_readonly::BUFFER);
				if (limited) {
					//last_buf.pop_utf8();
					if (last_buf.size() && last_buf.back().ch == '\n') last_buf.pop_utf8();
				}
				if (here_i_am->get_direct<bool>(textinput::e_boolean::NO_COLOR_TRANSLATION)) last_buf.refresh();
				here_i_am->set<Tools::Cstring>(text::e_cstring::STRING, last_buf);
				if (auto f = here_i_am->get_direct<sprite::functional>(textinput::e_tie_functional::SAVED_STRING); f) f(last_buf);
			}

			void TextInput::_rec_string()
			{
				Text* here_i_am = ((Text*)this);
				auto _res = here_i_am->get_direct<Tools::Cstring>(text::e_cstring::STRING);
				here_i_am->set<Tools::Cstring>(textinput::e_cstring_readonly::BUFFER, _res);
				here_i_am->set<Tools::Cstring>(text::e_cstring::STRING, [_res] { return _res + ((int)al_get_time() % 2 ? "_" : ""); });
			}

			void TextInput::handle_event(const Interface::RawEvent& ev)
			{
				Text* here_i_am = ((Text*)this);
				if (!here_i_am->get_direct<bool>(textinput::e_boolean_readonly::SELECTED)) return;

				auto buffer = here_i_am->get_direct<Tools::Cstring>(textinput::e_cstring_readonly::BUFFER);


				const auto max_length_total = here_i_am->get_direct<int>(text::e_integer::TOTAL_TEXT_MAX_LENGTH);
				const auto max_num_lines = here_i_am->get_direct<int>(text::e_integer::MAX_LINES_AMOUNT);
				const auto max_length_line = here_i_am->get_direct<int>(text::e_integer::LINE_MAX_LENGTH);// *here_i_am->get_direct<int>(text::e_integer::MAX_LINES_AMOUNT);

				//const size_t max_any = (max_length_line_total > max_length_total) ? max_length_line_total : max_length_total;

				const auto enter_newline = here_i_am->get_direct<bool>(textinput::e_boolean::ENTER_BREAK_LINE);

				switch (ev.type()) {
				case ALLEGRO_EVENT_KEY_CHAR:
				{
					const bool was_enter = ev.keyboard_event().keycode == ALLEGRO_KEY_ENTER || ev.keyboard_event().keycode == ALLEGRO_KEY_PAD_ENTER;

					if (ev.keyboard_event().unichar >= 32)
					{
						char multibyte[8] = { 0 };

						auto len = al_utf8_encode(multibyte, ev.keyboard_event().unichar <= 32 ? ' ' : ev.keyboard_event().unichar);
						std::string cpyh;
						for (size_t g = 0; g < len && g < 8; g++) cpyh += multibyte[g];

						Tools::Cstring now = buffer + cpyh;

						if (here_i_am->check_fit(now)) {
							here_i_am->set<Tools::Cstring>(textinput::e_cstring_readonly::BUFFER, now);
							here_i_am->set<Tools::Cstring>(text::e_cstring::STRING, [_res = now] {return _res + ((int)al_get_time() % 2 ? "_" : ""); });
						}
					}
					else if (ev.keyboard_event().keycode == ALLEGRO_KEY_BACKSPACE)
					{
						buffer.pop_utf8();
						here_i_am->set<Tools::Cstring>(textinput::e_cstring_readonly::BUFFER, buffer);
						here_i_am->set<Tools::Cstring>(text::e_cstring::STRING, [_res = buffer] { return _res + ((int)al_get_time() % 2 ? "_" : ""); });
					}
					else if ((was_enter && !enter_newline) || ev.keyboard_event().keycode == ALLEGRO_KEY_ESCAPE)
					{
						set_reading(false);
					}
					else if (was_enter && enter_newline)
					{
						Tools::Cstring now = buffer + '\n';

						if (here_i_am->check_fit(now)) {
							here_i_am->set<Tools::Cstring>(textinput::e_cstring_readonly::BUFFER, buffer + '\n');
							here_i_am->set<Tools::Cstring>(text::e_cstring::STRING, [_res = buffer + '\n']{ return _res + ((int)al_get_time() % 2 ? "_" : ""); });
						}
					}
				}
				break;
				case ALLEGRO_EVENT_MOUSE_AXES:
				{
					if ((here_i_am->get_direct<bool>(textinput::e_boolean::NON_AXIS_CANCEL_READING)) && (ev.mouse_event().dw != 0 || ev.mouse_event().dz != 0)) {
						set_reading(false);
					}
				}
				break;
				}
			}

			void TextInput::mouse_event(const sprite::e_tie_functional ev_type, const Tools::Any& data)
			{
				Text* here_i_am = ((Text*)this);

				switch (ev_type) {
				case sprite::e_tie_functional::COLLISION_MOUSE_CLICK:
					set_reading(true);
					break;
				case sprite::e_tie_functional::COLLISION_MOUSE_OFF:
					set_reading(false);
					break;
				}
			}

			Text& TextInput::main()
			{
				return *((Text*)this);
			}

			const Text& TextInput::main() const
			{
				return *((Text*)this);
			}

			void TextInput::set_reading(const bool readnow)
			{
				Text* here_i_am = ((Text*)this);

				if (readnow) {
					if (!here_i_am->get_direct<bool>(textinput::e_boolean_readonly::SELECTED)) {
						_rec_string();
						here_i_am->set(textinput::e_boolean_readonly::SELECTED, true);
					}
				}
				else {
					if (here_i_am->get_direct<bool>(textinput::e_boolean_readonly::SELECTED)) {
						here_i_am->set(textinput::e_boolean_readonly::SELECTED, false);
						_apply_latest();
					}
				}
			}

			TextInput::TextInput()
				: Button(), kbev(Interface::get_keyboard_event()), msev(Interface::get_mouse_event())
			{
				get_text().set<bool>(textinput::e_boolean_defaults);
				get_text().set<Tools::Cstring>(textinput::e_cstring_defaults);
				get_text().set<sprite::functional>(textinput::e_functional_defaults);

				event_handler.add(kbev);
				event_handler.add(msev);
				event_handler.set_run_autostart([&](const Interface::RawEvent& e) {handle_event(e); });
			}

		}
	}
}