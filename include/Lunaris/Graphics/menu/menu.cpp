#include "menu.h"

namespace Lunaris {

	LUNARIS_DECL void __display_menu_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_native_dialog_addon_initialized() && !al_init_native_dialog_addon()) throw std::runtime_error("Can't start Native Dialog!");
	}

    LUNARIS_DECL __menu_structure::__menu_structure(__menu_structure&& oth) noexcept
    {
        menu = oth.menu;
        parent = oth.parent;
        idx = oth.idx;
        id = oth.id;
        sub_menus = std::move(oth.sub_menus);

        oth.menu = nullptr;
        oth.parent = nullptr;
        oth.idx = -1;
        oth.id = 0;
        oth.sub_menus.clear();
    }

    LUNARIS_DECL void __menu_structure::operator=(__menu_structure&& oth) noexcept
    {
        if (parent && idx >= 0) {
            al_remove_menu_item(parent, -idx); // delete this and all related to this
        }

        menu = oth.menu;
        parent = oth.parent;
        idx = oth.idx;
        id = oth.id;
        sub_menus = std::move(oth.sub_menus);

        oth.menu = nullptr;
        oth.parent = nullptr;
        oth.idx = -1;
        oth.id = 0;
        oth.sub_menus.clear();
    }

    LUNARIS_DECL void __menu_structure::rebuild(const int forced_flag)
    {
        if (!parent) return;

        if (id == static_cast<uint16_t>(-1)) {
            if (!al_remove_menu_item(parent, -idx)) throw std::runtime_error("Can't find itself!");
            al_insert_menu_item(parent, -idx, nullptr, 0, 0, nullptr, nullptr);
            sub_menus.clear(); // no sub menu anymore
            return;
        }

        std::string _nam = "<empty>";
        if (const char* __tmp = al_get_menu_item_caption(parent, -idx); __tmp) _nam = __tmp;

        int _flags = forced_flag;
        if (_flags == 0) _flags = al_get_menu_item_flags(parent, -idx);
        if (_flags < 0) _flags = 0;

        if (menu) {
            if (!al_remove_menu_item(parent, -idx)) throw std::runtime_error("Can't find itself.");
        }

        if (!sub_menus.empty()) {
            menu = recursive_rebuild(sub_menus);
        }
        else menu = nullptr;

        al_insert_menu_item(parent, -idx, _nam.c_str(), id, _flags, nullptr, menu);
    }

    LUNARIS_DECL __menu_structure* __menu_structure::get_self() const
    {
        return (__menu_structure*)this;
    }

    LUNARIS_DECL ALLEGRO_MENU* __menu_structure::recursive_rebuild(const std::map<int, __menu_structure>& vc)
    {
        if (vc.empty()) return nullptr;

        ALLEGRO_MENU* bottom = al_create_menu();
        if (!bottom) throw std::runtime_error("Failed to create menu! FATAL ERROR!");

        for (const auto& it : vc) {

            if (it.second.idx < 0) { // empty space
                al_insert_menu_item(
                    bottom,
                    -it.first,
                    nullptr,
                    it.second.id,
                    0,
                    nullptr,
                    nullptr);
            }
            else {
                al_insert_menu_item(
                    bottom,
                    -it.first,
                    al_get_menu_item_caption(it.second.parent, -it.second.idx),
                    it.second.id,
                    al_get_menu_item_flags(it.second.parent, -it.second.idx),
                    nullptr,
                    it.second.sub_menus.size() ? recursive_rebuild(it.second.sub_menus) : nullptr);
            }
        }

        return bottom;
    }

    LUNARIS_DECL menu_quick::menu_quick(const std::string& s, const uint16_t i, const menu_flags f, const std::vector<menu_quick> l)
        : caption(s), id(i), flags(f), lst(l)
    {
    }

    LUNARIS_DECL menu_quick::menu_quick(const __menu_structure& ms) : 
        caption((ms.id == static_cast<uint16_t>(-1) || ms.idx < 0) ? std::string{} : (ms.parent ? (al_get_menu_item_caption(ms.parent, -ms.idx)) : std::string{})),
        id(ms.id),
        flags(static_cast<menu_flags>((ms.id == static_cast<uint16_t>(-1) || ms.idx < 0) ?  0 : (ms.parent ? (al_get_menu_item_flags(ms.parent, -ms.idx)) : 0))),
        lst(ms.sub_menus.empty() ? std::vector<menu_quick>{} : [&ms]() { std::vector<menu_quick> mq; for (const auto& m : ms.sub_menus) { mq.push_back(menu_quick{ m.second }); } return mq; }())
    {
    }

    LUNARIS_DECL menu_each_menu::menu_each_menu(const std::string& s, const uint16_t i, const menu_flags f, const std::vector<menu_quick> l)
        : menu_quick(s, i, f, l)
    {
    }

    LUNARIS_DECL menu_each_menu::menu_each_menu(const std::string& s, const uint16_t i, const std::vector<menu_quick> l)
        : menu_quick(s, i, static_cast<menu_flags>(0), l)
    {
    }

    LUNARIS_DECL menu_each_menu::menu_each_menu()
        : menu_quick("<empty>", 0, static_cast<menu_flags>(0), std::vector<menu_quick>{})
    {
    }

    LUNARIS_DECL menu_each_menu& menu_each_menu::set_name(const std::string& s)
    {
        caption = s;
        return *this;
    }

    LUNARIS_DECL menu_each_menu& menu_each_menu::set_caption(const std::string& s)
    {
        caption = s;
        return *this;
    }

    LUNARIS_DECL menu_each_menu& menu_each_menu::set_id(const uint16_t i)
    {
        id = i;
        return *this;
    }

    LUNARIS_DECL menu_each_menu& menu_each_menu::set_flags(menu_flags f)
    {
        flags = f;
        return *this;
    }

    LUNARIS_DECL menu_each_menu& menu_each_menu::set_flags(const int f)
    {
        flags = static_cast<menu_flags>(f);
        return *this;
    }

    LUNARIS_DECL menu_each_menu& menu_each_menu::push(const menu_quick& m)
    {
        lst.push_back(m);
        return *this;
    }

    LUNARIS_DECL menu_each_empty::menu_each_empty()
        : menu_quick(std::string{}, static_cast<uint16_t>(-1), static_cast<menu_flags>(0), std::vector<menu_quick>{})
    {
    }

    LUNARIS_DECL menu_each_default::menu_each_default(const std::string& s, const uint16_t i, const menu_flags f)
        : menu_quick(s, i, f, std::vector<menu_quick>{})
    {
    }

    LUNARIS_DECL menu_each_default::menu_each_default()
        : menu_quick("<empty>", 0, static_cast<menu_flags>(0), std::vector<menu_quick>{})
    {
    }

    LUNARIS_DECL menu_each_default& menu_each_default::set_name(const std::string& s)
    {
        caption = s;
        return *this;
    }

    LUNARIS_DECL menu_each_default& menu_each_default::set_caption(const std::string& s)
    {
        caption = s;
        return *this;
    }

    LUNARIS_DECL menu_each_default& menu_each_default::set_id(const uint16_t i)
    {
        id = i;
        return *this;
    }

    LUNARIS_DECL menu_each_default& menu_each_default::set_flags(menu_flags f)
    {
        flags = f;
        return *this;
    }

    LUNARIS_DECL menu_each_default& menu_each_default::set_flags(const int f)
    {
        flags = static_cast<menu_flags>(f);
        return *this;
    }

    LUNARIS_DECL menu_handler::menu_handler(__menu_structure& s, std::atomic<uint16_t>& i)
        : curr(s), _counter(i)
    {
    }

    LUNARIS_DECL void menu_handler::set_name(const std::string& str)
    {
        if (!str.empty())
            al_set_menu_item_caption(curr.parent, -curr.idx, str.c_str());
    }

    LUNARIS_DECL void menu_handler::set_caption(const std::string& str)
    {
        if (!str.empty())
            al_set_menu_item_caption(curr.parent, -curr.idx, str.c_str());
    }

    LUNARIS_DECL void menu_handler::set_flags(const menu_flags flg)
    {
        al_set_menu_item_flags(curr.parent, -curr.idx, al_get_menu_item_flags(curr.parent, -curr.idx) | static_cast<int>(flg));
        if (static_cast<int>(flg) & ALLEGRO_MENU_ITEM_CHECKBOX)
            curr.rebuild(al_get_menu_item_flags(curr.parent, -curr.idx) | static_cast<int>(flg));
    }

    LUNARIS_DECL void menu_handler::unset_flags(const menu_flags flg)
    {
        al_set_menu_item_flags(curr.parent, -curr.idx, al_get_menu_item_flags(curr.parent, -curr.idx) & (~static_cast<int>(flg)));
        if (static_cast<int>(flg) & ALLEGRO_MENU_ITEM_CHECKBOX)
            curr.rebuild(al_get_menu_item_flags(curr.parent, -curr.idx) & (~static_cast<int>(flg)));
    }

    LUNARIS_DECL void menu_handler::toggle_flags(const menu_flags flg)
    {
        al_set_menu_item_flags(curr.parent, -curr.idx, al_get_menu_item_flags(curr.parent, -curr.idx) ^ static_cast<int>(flg));
        if (static_cast<int>(flg) & ALLEGRO_MENU_ITEM_CHECKBOX)
            curr.rebuild(al_get_menu_item_flags(curr.parent, -curr.idx) ^ static_cast<int>(flg));
    }

    LUNARIS_DECL std::string menu_handler::get_name() const
    {
        if (curr.parent) {
            if (const char* uknow = al_get_menu_item_caption(curr.parent, -curr.idx); uknow) return uknow;
        }
        return {};
    }

    LUNARIS_DECL std::string menu_handler::get_caption() const
    {
        if (curr.parent) {
            if (const char* uknow = al_get_menu_item_caption(curr.parent, -curr.idx); uknow) return uknow;
        }
        return {};
    }

    LUNARIS_DECL menu_flags menu_handler::get_flags() const
    {
        return static_cast<menu_flags>(curr.parent ? al_get_menu_item_flags(curr.parent, -curr.idx) : 0);
    }

    LUNARIS_DECL uint16_t menu_handler::get_id() const
    {
        return curr.id;
    }

    LUNARIS_DECL int menu_handler::get_index_pos() const
    {
        return curr.idx;
    }

    LUNARIS_DECL void menu_handler::reset_id(const uint16_t nid)
    {
        if (curr.id == nid) return;
        curr.id = nid;
        curr.rebuild();
    }

    LUNARIS_DECL void menu_handler::reset_to_division()
    {
        reset_id(static_cast<uint16_t>(-1));
    }

    LUNARIS_DECL menu_handler menu_handler::operator[](const int ix)
    {
        if (ix < 0) throw std::out_of_range("Index pos can't be lower than 0");
        if (curr.id == static_cast<uint16_t>(-1)) throw std::runtime_error("An empty space cannot have any menus.");
        if (!curr.sub_menus.empty() && (curr.sub_menus.rbegin()->first < (ix - 1))) throw std::out_of_range("Please do not increase 2 steps at a time! One by one, always.");
        __menu_structure& c2 = curr.sub_menus[ix];
        int rs = 0;
        if (!c2.parent) {
            c2.id = ++_counter;
            c2.idx = ix;

            if (!curr.menu) {
                curr.menu = al_create_menu();
                if (!curr.menu) throw std::bad_alloc();

                std::string _nam = "<empty>";
                if (const char* __tmp = al_get_menu_item_caption(curr.parent, -curr.idx); __tmp) _nam = __tmp;
                int _flags = al_get_menu_item_flags(curr.parent, -curr.idx);
                if (_flags < 0) _flags = 0;

                al_remove_menu_item(curr.parent, -curr.idx);

                rs = al_insert_menu_item(curr.parent, -curr.idx, _nam.c_str(), curr.id, _flags, nullptr, curr.menu);
                if (rs == 0) {
                    rs = al_append_menu_item(curr.parent, _nam.c_str(), curr.id, _flags, nullptr, curr.menu);
                }
            }
            rs = al_insert_menu_item(curr.menu, -c2.idx, "<empty>", c2.id, 0, nullptr, nullptr);
            if (rs < 0) {
                rs = al_append_menu_item(curr.menu, "<empty>", c2.id, 0, nullptr, nullptr);
            }

            c2.menu = nullptr;
            c2.parent = curr.menu;
        }

        return menu_handler{ c2, _counter };
    }

    LUNARIS_DECL menu_handler menu_handler::operator[](const std::string& key)
    {
        for (const auto& it : curr.sub_menus)
        {
            if (it.second.parent) {
                if (const char* uknow = al_get_menu_item_caption(it.second.parent, -it.second.idx); uknow && key == uknow) {
                    return this->operator[](it.first);
                }
            }
        }
        if (curr.sub_menus.empty()) {
            menu_handler nh = this->operator[](0);
            nh.set_caption(key);
            return nh;
        }
        else {
            menu_handler nh = this->operator[](curr.sub_menus.rbegin()->first + 1);
            nh.set_caption(key);
            return nh;            
        }
    }

    LUNARIS_DECL bool menu_handler::remove(const int ix)
    {
        if (ix < 0) return false;
        if (curr.sub_menus.empty() || (curr.sub_menus.rbegin()->first < ix)) return false;

        al_remove_menu_item(curr.menu, -ix);
        curr.sub_menus.erase(ix);

        if (curr.sub_menus.empty()) {
            curr.rebuild();
            return true;
        }

        std::vector<int> mov;
        for (const auto& i : curr.sub_menus) {
            if (i.first > ix) mov.push_back(i.first);
        }

        for (const auto& j : mov) {
            curr.sub_menus[j - 1] = std::move(curr.sub_menus[j]);
            curr.sub_menus[j - 1].idx = j - 1;
            curr.sub_menus.erase(j);
        }

        return true;
    }

    LUNARIS_DECL bool menu_handler::remove(const std::string& key)
    {
        for (const auto& it : curr.sub_menus)
        {
            if (it.second.parent) {
                if (const char* uknow = al_get_menu_item_caption(it.second.parent, -it.second.idx); uknow && key == uknow) {
                    return remove(it.first);                    
                }
            }
        }
        return false;
    }

    LUNARIS_DECL bool menu_handler::pop_back()
    {
        if (size() > 0) {
            return remove(static_cast<int>(size()) - 1);
        }
        return false;
    }

    LUNARIS_DECL bool menu_handler::pop_front()
    {
        if (size() > 0) {
            return remove(0);
        }
        return false;
    }

    LUNARIS_DECL size_t menu_handler::size() const
    {
        return curr.sub_menus.size();
    }

    LUNARIS_DECL int menu_handler::push(const menu_quick& mq)
    {
        const int ix = (curr.sub_menus.size() > 0) ? (curr.sub_menus.rbegin()->first + 1) : 0;
        __menu_structure& c2 = curr.sub_menus[ix];

        if (!curr.menu) { // must have menu
            curr.menu = al_create_menu();
            if (!curr.menu) throw std::bad_alloc();

            std::string _nam = "<empty>";
            if (const char* __tmp = al_get_menu_item_caption(curr.parent, -curr.idx); __tmp) _nam = __tmp;
            int _flags = al_get_menu_item_flags(curr.parent, -curr.idx);
            if (_flags < 0) _flags = 0;

            al_remove_menu_item(curr.parent, -curr.idx);
            al_insert_menu_item(curr.parent, -curr.idx, _nam.c_str(), curr.id, _flags, nullptr, curr.menu);
        }


        c2.id = mq.id;
        c2.parent = curr.menu;
        c2.idx = ix;

        if (c2.id == static_cast<uint16_t>(-1)) {
            al_insert_menu_item(c2.parent, -c2.idx, nullptr, 0, 0, nullptr, nullptr);
            return ix;
        }
        
        if (mq.lst.size() > 0) {
            c2.menu = al_create_menu();
        }
        else c2.menu = nullptr;

        al_insert_menu_item(c2.parent, -c2.idx, mq.caption.c_str(), c2.id, static_cast<int>(mq.flags), nullptr, c2.menu);

        menu_handler mh{ c2, _counter };
        if (mq.lst.size()) mh.push(mq.lst);

        return ix;
    }

    LUNARIS_DECL void menu_handler::push(const std::vector<menu_quick>& mq)
    {
        for (const auto& it : mq) push(it);
    }

    LUNARIS_DECL uint16_t menu::find_greatest_id(const __menu_structure& ms) const
    {
        uint16_t v = 0;
        for (const auto& i : ms.sub_menus) {
            if (i.second.id != static_cast<uint16_t>(-1) && i.second.id > v) v = i.second.id;
            if (!i.second.sub_menus.empty()) {
                if (uint16_t ct = find_greatest_id(i.second); ct > v) v = ct;
            }
        }
        return v;
    }

    LUNARIS_DECL __menu_structure* menu::find_anywhere(const std::map<int, __menu_structure>& m, const std::function<bool(const std::pair<const int, __menu_structure>&)>& f)
    {
        for (auto e = m.cbegin(); e != m.cend(); ++e) {
            if (f(*e)) return e->second.get_self();
            if (!e->second.sub_menus.empty()) {
                if (__menu_structure* ptr = find_anywhere(e->second.sub_menus, f); ptr) return ptr;
            }
        }

        return nullptr;
    }

    LUNARIS_DECL bool menu::make_self_as()
    {
        if (_menu.menu) {
            ev_source = al_enable_menu_event_source(_menu.menu);
            return true;
        }
        if ((_menu.menu = (mmt == menu_type::BAR ? al_create_menu() : al_create_popup_menu())) != nullptr)
        {
            ev_source = al_enable_menu_event_source(_menu.menu);
            return true;
        }
        return false;
    }

    LUNARIS_DECL menu::menu(const menu_type mt)
        : mmt(mt)
    {
        __display_menu_allegro_start();
        if (!make_self_as()) throw std::runtime_error("Can't create menu!");        
    }

    LUNARIS_DECL menu::menu(ALLEGRO_DISPLAY* nd, const menu_type mt)
        : mmt(mt)
    {
        __display_menu_allegro_start();
        if (!make_self_as()) throw std::runtime_error("Can't create menu!");
        last_applied_display = nd;
    }

    LUNARIS_DECL menu::menu(menu&& oth) noexcept
        : mmt(oth.mmt)
    {
        __display_menu_allegro_start();
        _menu = std::move(oth._menu);
        last_applied_display = oth.last_applied_display;
        make_self_as();
        oth._menu.menu = nullptr; // really really sure
        oth.ev_source = nullptr;
        oth.last_applied_display = nullptr;
    }

    LUNARIS_DECL menu::menu(const std::initializer_list<menu_quick> lst)
        : mmt(menu_type::BAR)
    {
        __display_menu_allegro_start();
        if (!make_self_as()) throw std::runtime_error("Can't create menu!");
        for (const auto& each : lst) push(each);
    }

    LUNARIS_DECL menu::menu(const menu_type mt, const std::initializer_list<menu_quick> lst)
        : mmt(mt)
    {
        __display_menu_allegro_start();
        if (!make_self_as()) throw std::runtime_error("Can't create menu!");
        for (const auto& each : lst) push(each);
    }

    LUNARIS_DECL menu::menu(ALLEGRO_DISPLAY* nd, const std::initializer_list<menu_quick> lst)
        : mmt(menu_type::BAR)
    {
        __display_menu_allegro_start();
        if (!make_self_as()) throw std::runtime_error("Can't create menu!");
        for (const auto& each : lst) push(each);
        last_applied_display = nd;
    }

    LUNARIS_DECL menu::menu(ALLEGRO_DISPLAY* nd, const menu_type mt, const std::initializer_list<menu_quick> lst)
        : mmt(mt)
    {
        __display_menu_allegro_start();
        if (!make_self_as()) throw std::runtime_error("Can't create menu!");
        for (const auto& each : lst) push(each);
        last_applied_display = nd;
    }
     
    LUNARIS_DECL menu::~menu()
    {
        if (!_menu.menu) return;
        al_disable_menu_event_source(_menu.menu);
        ev_source = nullptr;
        al_destroy_menu(_menu.menu);
        _menu.menu = nullptr;
    }
     
    LUNARIS_DECL menu_handler menu::operator[](const int ix)
    {
        if (ix < 0) throw std::out_of_range("Index pos can't be lower than 0");
        if (!_menu.sub_menus.empty() && (_menu.sub_menus.rbegin()->first < (ix - 1))) throw std::out_of_range("Please do not increase 2 steps directly! One by one, always.");
        __menu_structure& c2 = _menu.sub_menus[ix];
        int rs = 0;
        if (!c2.parent) {

            c2.id = ++_counter;
            c2.parent = _menu.menu;
            c2.idx = ix;
            c2.menu = nullptr;// al_create_menu();

            if (!c2.menu) throw std::bad_alloc();
            rs = al_insert_menu_item(_menu.menu, -c2.idx, "<empty>", c2.id, 0, nullptr, c2.menu);
            if (rs < 0) {
                rs = al_append_menu_item(_menu.menu, "<empty>", c2.id, 0, nullptr, c2.menu);
            }
        }

        return menu_handler{ c2, _counter };
    }

    LUNARIS_DECL menu_handler menu::operator[](const std::string& key)
    {
        for (const auto& it : _menu.sub_menus)
        {
            if (it.second.parent) {
                if (const char* uknow = al_get_menu_item_caption(it.second.parent, -it.second.idx); uknow && key == uknow) {
                    return this->operator[](it.first);
                }
            }
        }
        if (_menu.sub_menus.empty()) {
            menu_handler nh = this->operator[](0);
            nh.set_caption(key);
            return nh;
        }
        else {
            menu_handler nh = this->operator[](_menu.sub_menus.rbegin()->first + 1);
            nh.set_caption(key);
            return nh;
        }
    }

    LUNARIS_DECL bool menu::remove(const int ix)
    {
        if (ix < 0) return false;
        if (_menu.sub_menus.empty() || ix > _menu.sub_menus.rbegin()->first) return false;
        al_remove_menu_item(_menu.menu, -ix);
        _menu.sub_menus.erase(ix);

        std::vector<int> mov;
        for (const auto& i : _menu.sub_menus) {
            if (i.first > ix) mov.push_back(i.first);
        }

        for (const auto& j : mov) {
            _menu.sub_menus[j - 1] = std::move(_menu.sub_menus[j]);
            _menu.sub_menus[j - 1].idx = j - 1;
            _menu.sub_menus.erase(j);
        }

        return true;
    }

    LUNARIS_DECL bool menu::remove(const std::string& key)
    {
        for (const auto& it : _menu.sub_menus)
        {
            if (it.second.parent) {
                if (const char* uknow = al_get_menu_item_caption(it.second.parent, -it.second.idx); uknow && key == uknow) {
                    return remove(it.first);
                }
            }
        }
        return false;
    }

    LUNARIS_DECL bool menu::pop_back()
    {
        if (size() > 0) {
            remove(static_cast<int>(size()) - 1);
            return true;
        }
        return false;
    }

    LUNARIS_DECL bool menu::pop_front()
    {
        if (size() > 0) {
            remove(0);
            return true;
        }
        return false;
    }

    LUNARIS_DECL size_t menu::size() const
    {
        return _menu.sub_menus.size();
    }

    LUNARIS_DECL bool menu::empty() const
    {
        return _menu.sub_menus.size() == 0;
    }

    LUNARIS_DECL bool menu::valid() const
    {
        return _menu.sub_menus.size() > 0 && _menu.menu;
    }

    LUNARIS_DECL menu menu::duplicate_as(const menu_type mt) const
    {
        menu nmen(last_applied_display, mt);
        menu_quick mq(_menu);
        for (const auto& m : mq.lst) nmen.push(m);
        return nmen;
    }

    LUNARIS_DECL menu_handler menu::find_id(const uint16_t ix)
    {
        __menu_structure* fnd = find_anywhere(_menu.sub_menus, [&](const std::pair<const int, __menu_structure>& r) { return r.second.id == ix; });
        if (fnd) return menu_handler{ *fnd, _counter };
        throw std::out_of_range("not found");
        return menu_handler{ *fnd, _counter }; // never goes here
    }

    LUNARIS_DECL menu_handler menu::find(const std::string& ix)
    {
        __menu_structure* fnd = find_anywhere(_menu.sub_menus, [&](const std::pair<const int, __menu_structure>& r) { if (const char* uknow = al_get_menu_item_caption(r.second.parent, -r.second.idx); uknow && ix == uknow) { return true; } return false; });
        if (fnd) return menu_handler{ *fnd, _counter };
        throw std::out_of_range("not found");
        return menu_handler{ *fnd, _counter }; // never goes here
    }
     
    LUNARIS_DECL void menu::show(ALLEGRO_DISPLAY* d)
    {
        if (!d) d = last_applied_display;

        if (d && _menu.menu) {
            if (last_applied_display != d) al_remove_display_menu(last_applied_display);

            last_applied_display = d;

            if (mmt == menu_type::POPUP) al_popup_menu(_menu.menu, d);
            else al_set_display_menu(d, _menu.menu);
        }
    }

    LUNARIS_DECL void menu::hide(ALLEGRO_DISPLAY* d)
    {
        if (!d) d = last_applied_display;

        if (d) al_remove_display_menu(d);
    }

    LUNARIS_DECL int menu::push(const menu_quick& mq)
    {
        const int ix = (_menu.sub_menus.size() > 0) ? (_menu.sub_menus.rbegin()->first + 1) : 0;
        __menu_structure& c2 = _menu.sub_menus[ix];

        c2.id = mq.id;
        c2.parent = _menu.menu;
        c2.idx = ix;
        c2.menu = mq.lst.empty() ? nullptr : al_create_menu(); // must be menu

        al_insert_menu_item(c2.parent, -c2.idx, mq.caption.c_str(), c2.id, static_cast<int>(mq.flags), nullptr, c2.menu);

        if (mq.lst.size()) {
            menu_handler mh{ c2, _counter };
            mh.push(mq.lst);
        }

        return ix;
    }

    LUNARIS_DECL void menu::push(const std::vector<menu_quick>& mq)
    {
        for (const auto& it : mq) push(it);
    }

    LUNARIS_DECL menu::operator std::vector<ALLEGRO_EVENT_SOURCE*>() const
    {
        return ev_source ? std::vector<ALLEGRO_EVENT_SOURCE*>{ ev_source } : std::vector<ALLEGRO_EVENT_SOURCE*>{};
    }

    LUNARIS_DECL ALLEGRO_EVENT_SOURCE* menu::get_event_source() const
    {
        return ev_source;
    }

    LUNARIS_DECL menu_event::menu_event(menu& mn, const ALLEGRO_EVENT& ev)
        : _ref(mn)
    {
        if (ev.type != ALLEGRO_EVENT_MENU_CLICK) return;

        if (!(source = (ALLEGRO_MENU*)ev.user.data3)) return; // set source


        uint16_t _id = static_cast<uint16_t>(ev.user.data1);
        std::string _from;
        if (const char* _f = al_get_menu_item_caption(source, _id); _f) _from = _f;

        if (_from.empty()) {
            try {
                handl = std::unique_ptr<menu_handler>(new menu_handler{ _ref.find(_from) });
            }
            catch (...) {} // not found means that is null
        }
        if (!handl) {
            try {
                handl = std::unique_ptr<menu_handler>(new menu_handler{ _ref.find_id(_id) });
            }
            catch (...) {} // not found means that is null
        }
    }
     
    LUNARIS_DECL menu* menu_event::operator->() 
    {
        return &_ref;
    }
     
    LUNARIS_DECL menu* menu_event::operator->() const
    {
        return &_ref;
    }
     
    LUNARIS_DECL bool menu_event::valid() const
    {
        return source && handl;
    }
     
    LUNARIS_DECL bool menu_event::empty() const
    {
        return !valid();
    }
    
    LUNARIS_DECL std::string menu_event::get_name() const
    {
        if (handl) return handl->get_caption();
        return {};
    }

    LUNARIS_DECL std::string menu_event::get_caption() const
    {
        if (handl) return handl->get_caption();
        return {};
    }

    LUNARIS_DECL uint16_t menu_event::get_id() const
    {
        if (handl) return handl->get_id();
        return 0;
    }

    LUNARIS_DECL menu_flags menu_event::get_flags() const
    {
        if (handl) return handl->get_flags();
        return static_cast<menu_flags>(0);
    }
     
    LUNARIS_DECL bool menu_event::has_flag(const menu_flags f) const
    {
        if (handl) return static_cast<int>(handl->get_flags() & f) != 0;
        return false;
    }

    LUNARIS_DECL bool menu_event::is_checkbox() const
    {
    	return has_flag(menu_flags::AS_CHECKBOX);
    }
    
    LUNARIS_DECL bool menu_event::is_checked() const
    {
    	return has_flag(menu_flags::AS_CHECKBOX) && has_flag(menu_flags::CHECKED);
    }
    
    LUNARIS_DECL bool menu_event::is_enabled() const
    {
    	return !has_flag(menu_flags::DISABLED);
    }
    
    LUNARIS_DECL bool menu_event::is_disabled() const
    {
    	return has_flag(menu_flags::DISABLED);
    }

    LUNARIS_DECL void menu_event::patch_name(const std::string& str)
    {
        if (handl) handl->set_caption(str);
    }

    LUNARIS_DECL void menu_event::patch_caption(const std::string& str)
    {
        if (handl) handl->set_caption(str);
    }

    LUNARIS_DECL void menu_event::patch_flags(const menu_flags f)
    {
        if (handl) handl->set_flags(f);
    }

    LUNARIS_DECL void menu_event::patch_toggle_flag(const menu_flags f)
    {
        if (handl) handl->toggle_flags(f);
    }
     
    LUNARIS_DECL menu& menu_event::get_source()
    {
        return _ref;
    }

}