#include "keys.h"

namespace Lunaris {


	void keys::handle_events(const ALLEGRO_EVENT& ev)
	{
		if (ev.type != ALLEGRO_EVENT_KEY_UP && ev.type != ALLEGRO_EVENT_KEY_DOWN) return;

		auto lucky = get_lock();

		keychain[ev.keyboard.keycode] = (ev.type == ALLEGRO_EVENT_KEY_DOWN);
		if (event_handler) event_handler({ ev.keyboard.keycode, ev.type == ALLEGRO_EVENT_KEY_DOWN });
	}

	keys::keys() : __common_event()
	{
		__keyboard_allegro_start();
		
		al_register_event_source(get_event_queue(), al_get_keyboard_event_source());
	}

	keys::~keys()
	{
		this->stop(); // stop before this is destroyed
	}

	void keys::hook_event(const std::function<void(const key_event&)> f)
	{
		auto lucky = get_lock();
		event_handler = f;
	}

	void keys::unhook_event()
	{
		auto lucky = get_lock();
		event_handler = {};
	}

	bool keys::is_key_pressed(const int id)
	{
		return keychain[id];
	}

}