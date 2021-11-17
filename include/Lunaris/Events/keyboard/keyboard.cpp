#include "keyboard.h"

namespace Lunaris {
	
	LUNARIS_DECL std::string keyboard::gen_output() const
	{
		std::string outt;
		for (size_t p = 0; p < data.size(); p++) tool_conv(outt, p);
		return outt;
	}

	LUNARIS_DECL void keyboard::handle_events(const ALLEGRO_EVENT& ev)
	{
		if (ev.type != ALLEGRO_EVENT_KEY_CHAR) return;

		const bool was_enter = ev.keyboard.keycode == ALLEGRO_KEY_ENTER || ev.keyboard.keycode == ALLEGRO_KEY_PAD_ENTER;

		if (ev.keyboard.unichar >= 32)
		{
			data.push_back(ev.keyboard.unichar);
			if (each_key) each_key(*this, ev.keyboard.unichar);

			if (each_combined) {
				std::string rightnow = gen_output();
				each_combined(*this, rightnow);
			}

		}
		else if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE)
		{
			if (data.size()) data.pop_back();

			if (each_key) each_key(*this, '\b');

			if (each_combined) {
				std::string rightnow = gen_output();
				each_combined(*this, rightnow);
			}
		}
		else if (was_enter || ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
		{
			if (enter_combined || each_key) {

				std::string rightnow = gen_output();

				if (each_combined) each_combined(*this, rightnow);
				if (enter_combined) enter_combined(*this, rightnow);
			}
		}
	}

	LUNARIS_DECL void keyboard::tool_conv(std::string& str, const size_t pos) const
	{
		if (pos >= data.size()) return;

		const int& arg = data[pos];

		char multibyte[8] = { 0 };
		size_t len = al_utf8_encode(multibyte, arg); // data[pos] is for sure >= 32.

		if (len > 8) throw std::runtime_error("Unexpected invalid UTF8 translation.");

		for (size_t a = 0; a < len; a++) str += multibyte[a];
	}

	LUNARIS_DECL keyboard::keyboard() : generic_event_handler()
	{
		// because of this line here we don't need to check if m_movable exists (it must exist)
		install(events::KEYBOARD);
		get_core().set_event_handler([this](const ALLEGRO_EVENT& ev) { handle_events(ev); });
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("%p is KEYBOARD object", get_core_ptr());
#endif
	}

	LUNARIS_DECL void keyboard::hook_each_key_event(const std::function<void(keyboard&, const int)> f)
	{
		std::lock_guard<std::recursive_mutex> luck(get_core().m_safe);
		each_key = f;
	}

	LUNARIS_DECL void keyboard::hook_each_key_phrase_event(const std::function<void(keyboard&, const std::string&)> f)
	{
		std::lock_guard<std::recursive_mutex> luck(get_core().m_safe);
		each_combined = f;
	}

	LUNARIS_DECL void keyboard::hook_enter_line_phrase_event(const std::function<void(keyboard&, const std::string&)> f)
	{
		std::lock_guard<std::recursive_mutex> luck(get_core().m_safe);
		enter_combined = f;
	}

	LUNARIS_DECL void keyboard::unhook_each_key_event()
	{
		std::lock_guard<std::recursive_mutex> luck(get_core().m_safe);
		each_key = {};
	}

	LUNARIS_DECL void keyboard::unhook_each_key_phrase_event()
	{
		std::lock_guard<std::recursive_mutex> luck(get_core().m_safe);
		each_combined = {};
	}

	LUNARIS_DECL void keyboard::unhook_enter_line_phrase_event()
	{
		std::lock_guard<std::recursive_mutex> luck(get_core().m_safe);
		enter_combined = {};
	}

	LUNARIS_DECL void keyboard::clear()
	{
		std::lock_guard<std::recursive_mutex> luck(get_core().m_safe);
		data.clear();
	}

}