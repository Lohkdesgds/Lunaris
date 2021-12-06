#include "mouse.h"

namespace Lunaris {

    LUNARIS_DECL mouse::mouse_event::mouse_event(mouse& m)
        : mouse_itself(m)
    {
    }

    LUNARIS_DECL bool mouse::mouse_event::is_button_pressed(const int id) const
    {
        if (id > 32) return false;
        return buttons_pressed & (1 << id);
    }

    LUNARIS_DECL bool mouse::mouse_event::got_scroll_event() const
    {
        return scroll_event > 0;
    }

    LUNARIS_DECL int mouse::mouse_event::scroll_event_id(const int id) const
    {
        if (id > 8) return 0;
        if (scroll_event & (1 << (id * 2))) return 1;
        else if (scroll_event & (1 << ((id * 2) + 1))) return -1;
        return 0;
    }

    LUNARIS_DECL mouse& mouse::mouse_event::get_source()
    {
        return mouse_itself;
    }

	LUNARIS_DECL void mouse::set_mouse_axis_plus(const int axis, const int val)
    {
        mouse_rn.scroll_event |= (1 << (val > 0 ? (axis * 2) : ((axis * 2) + 1)));
    }

	LUNARIS_DECL void mouse::handle_events(const ALLEGRO_EVENT& ev)
    {
        switch (ev.type) {
        case ALLEGRO_EVENT_MOUSE_AXES:
        {
            if (ev.mouse.dw != 0) set_mouse_axis_plus(0, ev.mouse.dw);
            if (ev.mouse.dz != 0) set_mouse_axis_plus(1, ev.mouse.dz);

            ALLEGRO_TRANSFORM curr_transf = current_transform_getter();

            float max_x = -1.0f, max_y = -1.0f;
            float quick_mx, quick_my;
            quick_mx = ev.mouse.x;
            quick_my = ev.mouse.y;

            al_invert_transform(&curr_transf);

            al_transform_coordinates(&curr_transf, &max_x, &max_y);
            al_transform_coordinates(&curr_transf, &quick_mx, &quick_my);

            mouse_rn.raw_mouse_event = ev.mouse;
            mouse_rn.real_posx = quick_mx;
            mouse_rn.real_posy = quick_my;
            mouse_rn.relative_posx = mouse_rn.real_posx * 1.0 / fabsf(max_x);
            mouse_rn.relative_posy = mouse_rn.real_posy * 1.0 / fabsf(max_y);

            if (event_handler) event_handler(ALLEGRO_EVENT_MOUSE_AXES, mouse_rn);
            mouse_rn.scroll_event = 0; // always reset
        }
            break;
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            mouse_rn.raw_mouse_event = ev.mouse;
            if (ev.mouse.button <= 32) {
                mouse_rn.buttons_pressed |= 1 << (ev.mouse.button - 1); // starts at #1

                if (event_handler) event_handler(ALLEGRO_EVENT_MOUSE_BUTTON_DOWN, mouse_rn);

                mouse_rn.scroll_event = 0; // always reset
            }
            break;
        case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
            mouse_rn.raw_mouse_event = ev.mouse;
            if (ev.mouse.button <= 32) {
                mouse_rn.buttons_pressed &= ~(1 << (ev.mouse.button - 1)); // starts at #1

                if (event_handler) event_handler(ALLEGRO_EVENT_MOUSE_BUTTON_UP, mouse_rn);

                mouse_rn.scroll_event = 0; // always reset
            }
            break;
        }
    }

    LUNARIS_DECL mouse::mouse(std::function<ALLEGRO_TRANSFORM(void)> f) 
        : generic_event_handler(), mouse_rn(*this)
    {
        if (!f) throw std::invalid_argument("Mouse needs a valid ALLEGRO_TRANSFORM source. This is normally given by a display (it should be castable directly).");

        current_transform_getter = f;
        install(events::MOUSE);
        get_core().set_event_handler([this](const ALLEGRO_EVENT& ev) { handle_events(ev); });
#ifdef LUNARIS_VERBOSE_BUILD
        PRINT_DEBUG("%p is MOUSE object", get_core_ptr());
#endif
    }

	LUNARIS_DECL void mouse::hook_event(const std::function<void(const int, const mouse_event&)> f)
    {
        std::lock_guard<std::recursive_mutex> luck(get_core().m_safe);
        event_handler = f;
    }

	LUNARIS_DECL void mouse::unhook_event()
    {
        std::lock_guard<std::recursive_mutex> luck(get_core().m_safe);
        event_handler = {};
    }

    LUNARIS_DECL const mouse::mouse_event& mouse::current_mouse() const
    {
        return mouse_rn;
    }

}