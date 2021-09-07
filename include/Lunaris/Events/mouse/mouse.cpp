#include "mouse.h"

namespace Lunaris {

    bool mouse::mouse_event::is_button_pressed(const int id) const
    {
        if (id > 32) return false;
        return buttons_pressed & (1 << id);
    }

    bool mouse::mouse_event::got_scroll_event() const
    {
        return scroll_event > 0;
    }

    int mouse::mouse_event::scroll_event_id(const int id) const
    {
        if (id > 8) return 0;
        if (scroll_event & (1 << (id * 2))) return 1;
        else if (scroll_event & (1 << ((id * 2) + 1))) return -1;
        return 0;
    }

    void mouse::set_mouse_axis_plus(const int axis, const int val)
    {
        //printf_s("SET %d -> %d", axis, val);
        mouse_rn.scroll_event |= (1 << (val > 0 ? (axis * 2) : ((axis * 2) + 1)));
    }

    void mouse::handle_events(const ALLEGRO_EVENT& ev)
    {
        auto lucky = get_lock();

        switch (ev.type) {
        case ALLEGRO_EVENT_TIMER:
        {
            if (!had_mouse_event) return;
            had_mouse_event = false;

            ALLEGRO_TRANSFORM curr_transf = current_transform_getter();

            float max_x = -1.0f, max_y = -1.0f;

            al_invert_transform(&curr_transf);

            al_transform_coordinates(&curr_transf, &max_x, &max_y);
            al_transform_coordinates(&curr_transf, &quick_cpy_mouse[0], &quick_cpy_mouse[1]);

            mouse_rn.real_posx = quick_cpy_mouse[0];
            mouse_rn.real_posy = quick_cpy_mouse[1];
            mouse_rn.relative_posx = mouse_rn.real_posx * 1.0 / fabsf(max_x);
            mouse_rn.relative_posy = mouse_rn.real_posy * 1.0 / fabsf(max_y);

            if (event_handler) event_handler(ALLEGRO_EVENT_MOUSE_AXES, mouse_rn);
            mouse_rn.scroll_event = 0; // always reset
        }
            break;
        case ALLEGRO_EVENT_MOUSE_AXES:
            quick_cpy_mouse[0] = ev.mouse.x;
            quick_cpy_mouse[1] = ev.mouse.y;
            if (ev.mouse.dw != 0) set_mouse_axis_plus(0, ev.mouse.dw);
            if (ev.mouse.dz != 0) set_mouse_axis_plus(1, ev.mouse.dz);
            had_mouse_event = true;
            break;
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            if (ev.mouse.button <= 32) {
                mouse_rn.buttons_pressed |= 1 << (ev.mouse.button - 1); // starts at #1
                if (event_handler) event_handler(ALLEGRO_EVENT_MOUSE_BUTTON_DOWN, mouse_rn);
                mouse_rn.scroll_event = 0; // always reset
            }
            break;
        case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
            if (ev.mouse.button <= 32) {
                mouse_rn.buttons_pressed &= ~(1 << (ev.mouse.button - 1)); // starts at #1
                if (event_handler) event_handler(ALLEGRO_EVENT_MOUSE_BUTTON_UP, mouse_rn);
                mouse_rn.scroll_event = 0; // always reset
            }
            break;
        }
    }

    mouse::mouse(const std::function<ALLEGRO_TRANSFORM(void)> f) : __common_event()
    {
        if (!f) throw std::runtime_error("No function detected in mouse! Mouse needs a valid function to get transformation of current display so it can translate coords!");

        __mouse_allegro_start();

        current_transform_getter = f;

        if (!(timer = al_create_timer(default_mouse_update_rate))) {
            throw std::bad_alloc();
        }

        al_register_event_source(get_event_queue(), al_get_mouse_event_source());
        al_register_event_source(get_event_queue(), al_get_timer_event_source(timer));


        al_start_timer(timer);
    }

    mouse::~mouse()
    {
        if (timer) {
            this->stop(); // stop before destroying timer
            al_destroy_timer(timer);
            timer = nullptr;
        }
    }

    void mouse::hook_event(const std::function<void(const int, const mouse_event&)> f)
    {
        auto lucky = get_lock();
        event_handler = f;
    }

    void mouse::unhook_event()
    {
        auto lucky = get_lock();
        event_handler = {};
    }

    const mouse::mouse_event& mouse::current_mouse() const
    {
        return mouse_rn;
    }

}