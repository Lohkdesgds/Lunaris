#include "keys.h"

namespace Lunaris {

	LUNARIS_DECL void keys::handle_events(const ALLEGRO_EVENT& ev)
	{
		if (ev.type != ALLEGRO_EVENT_KEY_UP && ev.type != ALLEGRO_EVENT_KEY_DOWN) return;

		keychain[ev.keyboard.keycode] = (ev.type == ALLEGRO_EVENT_KEY_DOWN);
		if (event_handler) event_handler({ ev.keyboard.keycode, ev.type == ALLEGRO_EVENT_KEY_DOWN });
	}

	LUNARIS_DECL keys::keys() : generic_event_handler()
	{
		install(events::KEYBOARD);
		get_core().set_event_handler([this](const ALLEGRO_EVENT& ev) { handle_events(ev); });
		PRINT_DEBUG("%p is KEYS object", get_core_ptr());
	}

	LUNARIS_DECL void keys::hook_event(const std::function<void(const key_event&)> f)
	{
		std::lock_guard<std::recursive_mutex> luck(get_core().m_safe);
		event_handler = f;
	}

	LUNARIS_DECL void keys::unhook_event()
	{
		std::lock_guard<std::recursive_mutex> luck(get_core().m_safe);
		event_handler = {};
	}

	LUNARIS_DECL bool keys::is_key_pressed(const int id)
	{
		return keychain[id];
	}

}