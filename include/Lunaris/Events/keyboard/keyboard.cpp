#include "keyboard.h"

namespace Lunaris {
	
	std::string keyboard::gen_output() const
	{
		std::string outt;
		for (size_t p = 0; p < data.size(); p++) tool_conv(outt, p);
		return outt;
	}

	void keyboard::handle_events(const ALLEGRO_EVENT& ev)
	{
		if (ev.type != ALLEGRO_EVENT_KEY_CHAR) return;

		auto safe = get_lock();

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

	void keyboard::tool_conv(std::string& str, const size_t pos) const
	{
		if (pos >= data.size()) return;

		const int& arg = data[pos];

		char multibyte[8] = { 0 };
		size_t len = al_utf8_encode(multibyte, arg); // data[pos] is for sure >= 32.

		if (len > 8) throw std::runtime_error("Unexpected invalid UTF8 translation.");

		for (size_t a = 0; a < len; a++) str += multibyte[a];
	}

	keyboard::keyboard() : __common_event()
	{
		__keyboard_allegro_start();

		al_register_event_source(get_event_queue(), al_get_keyboard_event_source());
	}

	void keyboard::hook_each_key_event(const std::function<void(keyboard&, const int)> f)
	{
		auto lucky = get_lock();
		each_key = f;
	}

	void keyboard::hook_each_key_phrase_event(const std::function<void(keyboard&, const std::string&)> f)
	{
		auto lucky = get_lock();
		each_combined = f;
	}

	void keyboard::hook_enter_line_phrase_event(const std::function<void(keyboard&, const std::string&)> f)
	{
		auto lucky = get_lock();
		enter_combined = f;
	}

	void keyboard::unhook_each_key_event()
	{
		auto lucky = get_lock();
		each_key = {};
	}

	void keyboard::unhook_each_key_phrase_event()
	{
		auto lucky = get_lock();
		each_combined = {};
	}

	void keyboard::unhook_enter_line_phrase_event()
	{
		auto lucky = get_lock();
		enter_combined = {};
	}

	void keyboard::clear()
	{
		auto lucky = get_lock();
		data.clear();
	}

}