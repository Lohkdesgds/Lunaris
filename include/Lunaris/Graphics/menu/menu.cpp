#include "menu.h"

namespace Lunaris {

	LUNARIS_DECL void __display_menu_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_native_dialog_addon_initialized() && !al_init_native_dialog_addon()) throw std::runtime_error("Can't start Native Dialog!");
	}

	//LUNARIS_DECL __menu_each_generic::__menu_each_generic()
	//{ 
	//	__display_menu_allegro_start();
	//}
	//LUNARIS_DECL std::vector<ALLEGRO_MENU_INFO> __menu_each_generic::generate() const
	//{
	//	return {};
	//}
	//
	//LUNARIS_DECL __menu_each_generic::menu_each_type __menu_each_generic::get_type() const
	//{
	//	return __menu_each_generic::menu_each_type::_INVALID;
	//}
	//
	//LUNARIS_DECL bool __menu_each_generic::operator==(const uint16_t) const
	//{
	//	return false;
	//}
	//
	//LUNARIS_DECL bool __menu_each_generic::operator!=(const uint16_t) const
	//{
	//	return true;
	//}
	//
	//LUNARIS_DECL void __menu_each_generic::update_self(ALLEGRO_MENU*)
	//{
	//	return;
	//}
	//
	//LUNARIS_DECL __menu_each_generic* __menu_each_generic::export_new() const
	//{
	//	return nullptr;
	//}

	LUNARIS_DECL std::vector<ALLEGRO_MENU_INFO> menu_each_empty::generate() const
	{
		return { ALLEGRO_MENU_INFO{ nullptr, static_cast<uint16_t>(-1), 0, nullptr } };
	}

	LUNARIS_DECL __menu_each_generic::menu_each_type menu_each_empty::get_type() const
	{
		return menu_each_type::EMPTY;
	}

	LUNARIS_DECL bool menu_each_empty::operator==(const uint16_t var) const
	{
		return false;
	}

	LUNARIS_DECL bool menu_each_empty::operator!=(const uint16_t var) const
	{
		return true;
	}

	LUNARIS_DECL void menu_each_empty::update_self(ALLEGRO_MENU*)
	{
		return; // has nothing to do
	}

	LUNARIS_DECL menu_each_default::menu_each_default(const std::string& nam, const uint16_t i, const int flg)
		: name(nam), id(i), flags(flg)
	{
	}

	LUNARIS_DECL menu_each_default& menu_each_default::set_name(const std::string& var)
	{
		name = var;
		return *this;
	}

	LUNARIS_DECL menu_each_default& menu_each_default::set_id(const uint16_t var)
	{
		id = var;
		return *this;
	}

	LUNARIS_DECL menu_each_default& menu_each_default::set_flags(std::initializer_list<menu_item_flags> flgs)
	{
		flags = 0;
		for (const auto& ea : flgs) flags |= static_cast<int>(ea);
		return *this;
	}

	LUNARIS_DECL menu_each_default& menu_each_default::set_flags(const int flgs)
	{
		flags = flgs;
		return *this;
	}

	LUNARIS_DECL const std::string& menu_each_default::get_name() const
	{
		return name;
	}

	LUNARIS_DECL uint16_t menu_each_default::get_id() const
	{
		return id;
	}

	LUNARIS_DECL int menu_each_default::get_flags() const
	{
		return flags;
	}

	LUNARIS_DECL std::vector<ALLEGRO_MENU_INFO> menu_each_default::generate() const
	{
		return { ALLEGRO_MENU_INFO{ name.c_str(), id, flags, nullptr } };
	}

	LUNARIS_DECL __menu_each_generic::menu_each_type menu_each_default::get_type() const
	{
		return menu_each_type::DEFAULT;
	}

	LUNARIS_DECL bool menu_each_default::operator==(const uint16_t var) const
	{
		return id == var;
	}

	LUNARIS_DECL bool menu_each_default::operator!=(const uint16_t var) const
	{
		return id != var;
	}

	LUNARIS_DECL void menu_each_default::update_self(ALLEGRO_MENU* men)
	{
		ALLEGRO_MENU* chosen = nullptr;
		int index = 0;
		if (al_find_menu_item(men, id, &chosen, &index))
		{
			name = al_get_menu_item_caption(chosen, index);
			flags = al_get_menu_item_flags(chosen, index);
		}
	}

	LUNARIS_DECL menu_each_menu::menu_each_menu(std::initializer_list<__menu_each_generic> lst)
	{
		for (auto& it : lst) {
			switch (it.get_type()) {
			case __menu_each_generic::menu_each_type::DEFAULT:
			{
				menu_each_default* pp = (menu_each_default*)&it;
				sub_menus.push_back(std::shared_ptr<__menu_each_generic>(new menu_each_default(*pp)));
			}
			break;
			case __menu_each_generic::menu_each_type::MENU:
			{
				menu_each_menu* pp = (menu_each_menu*)&it;
				sub_menus.push_back(std::shared_ptr<__menu_each_generic>(new menu_each_menu(*pp)));
			}
			break;
			case __menu_each_generic::menu_each_type::EMPTY:
			{
				menu_each_empty* pp = (menu_each_empty*)&it;
				sub_menus.push_back(std::shared_ptr<__menu_each_generic>(new menu_each_empty(*pp)));
			}
			break;
			default:
				continue;
			}
		}
	}

	LUNARIS_DECL menu_each_menu::menu_each_menu(const std::string& nam, const uint16_t i, const int flg)
		: name(nam), id(i), flags(flg)
	{
	}

	LUNARIS_DECL menu_each_menu::menu_each_menu(const std::string& nam, const uint16_t i, const int flg, std::initializer_list<__menu_each_generic> lst)
		: name(nam), id(i), flags(flg)
	{
		for (auto& it : lst) {
			switch (it.get_type()) {
			case __menu_each_generic::menu_each_type::DEFAULT:
			{
				menu_each_default* pp = (menu_each_default*)&it;
				sub_menus.push_back(std::shared_ptr<__menu_each_generic>(new menu_each_default(*pp)));
			}
			break;
			case __menu_each_generic::menu_each_type::MENU:
			{
				menu_each_menu* pp = (menu_each_menu*)&it;
				sub_menus.push_back(std::shared_ptr<__menu_each_generic>(new menu_each_menu(*pp)));
			}
			break;
			case __menu_each_generic::menu_each_type::EMPTY:
			{
				menu_each_empty* pp = (menu_each_empty*)&it;
				sub_menus.push_back(std::shared_ptr<__menu_each_generic>(new menu_each_empty(*pp)));
			}
			break;
			default:
				continue;
			}
		}
	}

	LUNARIS_DECL menu_each_menu& menu_each_menu::set_name(const std::string& var)
	{
		name = var + "->";
		return *this;
	}

	LUNARIS_DECL menu_each_menu& menu_each_menu::set_id(const uint16_t var)
	{
		id = var;
		return *this;
	}

	LUNARIS_DECL menu_each_menu& menu_each_menu::set_flags(std::initializer_list<menu_item_flags> flgs)
	{
		flags = 0;
		for (const auto& ea : flgs) flags |= static_cast<int>(ea);
		return *this;
	}

	LUNARIS_DECL menu_each_menu& menu_each_menu::set_flags(const int flgs)
	{
		flags = flgs;
		return *this;
	}

	LUNARIS_DECL menu_each_menu& menu_each_menu::push(const menu_each_empty& oth)
	{
		sub_menus.push_back(std::shared_ptr<__menu_each_generic>(new menu_each_empty(oth)));
		return *this;
	}

	LUNARIS_DECL menu_each_menu& menu_each_menu::push(const menu_each_default& oth)
	{
		sub_menus.push_back(std::shared_ptr<__menu_each_generic>(new menu_each_default(oth)));
		return *this;
	}

	LUNARIS_DECL menu_each_menu& menu_each_menu::push(const menu_each_menu& oth)
	{
		sub_menus.push_back(std::shared_ptr<__menu_each_generic>(new menu_each_menu(oth)));
		return *this;
	}

	LUNARIS_DECL std::string menu_each_menu::get_name() const
	{
		if (size_t _tmp = name.rfind("->"); _tmp != std::string::npos && (_tmp + 2) == name.size()) {
			return name.substr(0, _tmp);
		}
		return "";
	}

	LUNARIS_DECL uint16_t menu_each_menu::get_id() const
	{
		return id;
	}

	LUNARIS_DECL int menu_each_menu::get_flags() const
	{
		return flags;
	}

	LUNARIS_DECL const std::vector<std::shared_ptr<__menu_each_generic>>& menu_each_menu::get_sub_menus() const
	{
		return sub_menus;
	}

	LUNARIS_DECL std::vector<std::shared_ptr<__menu_each_generic>>& menu_each_menu::get_sub_menus()
	{
		return sub_menus;
	}

	LUNARIS_DECL std::vector<ALLEGRO_MENU_INFO> menu_each_menu::generate() const
	{
		std::vector<ALLEGRO_MENU_INFO> vec;

		vec.push_back({ name.c_str(), id, flags, 0 });
		for (auto& it : sub_menus) {
			switch (it->get_type()) {
			case __menu_each_generic::menu_each_type::DEFAULT:
			{
				menu_each_default* pp = (menu_each_default*)&it;
			}
			break;
			case __menu_each_generic::menu_each_type::MENU:
			{
				menu_each_menu* pp = (menu_each_menu*)&it;
			}
			break;
			case __menu_each_generic::menu_each_type::EMPTY:
			{
				menu_each_empty* pp = (menu_each_empty*)&it;
			}
			break;
			default:
				continue;
			}

			auto __vc = it->generate();
			vec.insert(vec.end(), __vc.begin(), __vc.end());
		}
		vec.push_back(ALLEGRO_END_OF_MENU);

		return vec;
	}

	LUNARIS_DECL __menu_each_generic::menu_each_type menu_each_menu::get_type() const
	{
		return menu_each_type::MENU;
	}

	LUNARIS_DECL bool menu_each_menu::operator==(const uint16_t var) const
	{
		return id == var;
	}

	LUNARIS_DECL bool menu_each_menu::operator!=(const uint16_t var) const
	{
		return id != var;
	}

	LUNARIS_DECL void menu_each_menu::update_self(ALLEGRO_MENU* men)
	{
		ALLEGRO_MENU* chosen = nullptr;
		int index = 0;
		if (al_find_menu_item(men, id, &chosen, &index))
		{
			name = al_get_menu_item_caption(chosen, index);
			flags = al_get_menu_item_flags(chosen, index);
		}
		for (auto& o : sub_menus) {
			o->update_self(men);
		}
	}

	LUNARIS_DECL void menu::make_happen(const menu_type mod)
	{
		std::vector<ALLEGRO_MENU_INFO> vc;
		for (const auto& it : menus) {
			const std::vector<ALLEGRO_MENU_INFO> _t = it.generate();
			vc.insert(vc.end(), _t.begin(), _t.end());
		}
		vc.push_back(ALLEGRO_END_OF_MENU);

		if (ptr) al_disable_menu_event_source(ptr.get());

		auto _tmp = std::shared_ptr<ALLEGRO_MENU>(al_build_menu(vc.data()), al_destroy_menu);

		if (mod == menu_type::POPUP) {
			ptr = std::shared_ptr<ALLEGRO_MENU>(al_clone_menu_for_popup(_tmp.get()), al_destroy_menu);
		}
		else {
			ptr = std::move(_tmp);
		}
		latest = mod;

		if (!(ev_source = al_enable_menu_event_source(ptr.get()))) throw std::runtime_error("Could not load menu properly");
	}

	LUNARIS_DECL menu::menu(display& d, std::initializer_list<menu_each_menu> lst, const menu_type mod)
		: disp(d), menus(lst.begin(), lst.end())
	{
		__display_menu_allegro_start();

		make_happen(mod);
	}

	LUNARIS_DECL menu::~menu()
	{
		hide();
		ptr.reset();
	}

	LUNARIS_DECL menu_each_menu& menu::index(const size_t p)
	{
		if (p >= menus.size()) throw std::runtime_error("out of range");
		return menus[p];
	}

	LUNARIS_DECL size_t menu::size() const
	{
		return menus.size();
	}

	LUNARIS_DECL void menu::update()
	{
		if (!ptr) return;
		for (auto& o : menus) {
			o.update_self(ptr.get());
		}
	}

	LUNARIS_DECL void menu::show()
	{
		showing = true;
		if (latest == menu_type::BAR) al_set_display_menu(disp.get_raw_display(), ptr.get());
		else al_popup_menu(ptr.get(), disp.get_raw_display());
	}

	LUNARIS_DECL void menu::hide()
	{
		showing = false;
		if (latest == menu_type::BAR) al_set_display_menu(disp.get_raw_display(), nullptr);
	}

	LUNARIS_DECL void menu::reset(const menu_type mod)
	{
		bool was_showing = showing;
		hide();
		make_happen(mod);
		if (was_showing) show();
	}

	LUNARIS_DECL menu::operator std::vector<ALLEGRO_EVENT_SOURCE*>() const
	{
		return ev_source ? std::vector<ALLEGRO_EVENT_SOURCE*>{ ev_source } : std::vector<ALLEGRO_EVENT_SOURCE*>{};
	}

	LUNARIS_DECL ALLEGRO_EVENT_SOURCE* menu::get_event_source() const
	{
		return ev_source;
	}

	LUNARIS_DECL menu_event::menu_event(menu& rf, const ALLEGRO_EVENT& ev)
		: _ref(rf)
	{
		if (ev.type != ALLEGRO_EVENT_MENU_CLICK) return;
		if (!(source = (ALLEGRO_MENU*)ev.user.data3)) return; // set source
		id = static_cast<uint16_t>(ev.user.data1);
		from = al_get_menu_item_caption(source, id);
		flags = al_get_menu_item_flags(source, id);
	}

	LUNARIS_DECL bool menu_event::valid() const
	{
		return source != nullptr;
	}

	LUNARIS_DECL const std::string& menu_event::get_name() const
	{
		return from;
	}

	LUNARIS_DECL uint16_t menu_event::get_id() const
	{
		return id;
	}

	LUNARIS_DECL int menu_event::get_flags() const
	{
		return flags;
	}

	LUNARIS_DECL bool menu_event::has_flag(const menu_item_flags f) const
	{
		return flags & static_cast<int>(f);
	}

	LUNARIS_DECL bool menu_event::is_checkbox() const
	{
		return has_flag(menu_item_flags::AS_CHECKBOX);
	}

	LUNARIS_DECL bool menu_event::is_checked() const
	{
		return has_flag(menu_item_flags::AS_CHECKBOX) && has_flag(menu_item_flags::CHECKED);
	}

	LUNARIS_DECL bool menu_event::is_enabled() const
	{
		return !has_flag(menu_item_flags::DISABLED);
	}

	LUNARIS_DECL bool menu_event::is_disabled() const
	{
		return has_flag(menu_item_flags::DISABLED);
	}

	LUNARIS_DECL void menu_event::patch_name(const std::string& var)
	{
		from = var;
		al_set_menu_item_caption(source, id, from.c_str());
	}

	LUNARIS_DECL void menu_event::patch_flags(const int var)
	{
		flags = var;
		al_set_menu_item_flags(source, id, flags);
	}

	LUNARIS_DECL void menu_event::patch_toggle_flag(const menu_item_flags var)
	{
		flags ^= static_cast<int>(var);
		al_set_menu_item_flags(source, id, flags);
	}

	LUNARIS_DECL menu& menu_event::get_source()
	{
		return _ref;
	}

}