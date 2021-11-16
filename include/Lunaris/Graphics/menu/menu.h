#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Utility/memory.h>
#include <Lunaris/Events/generic_event_handler.h>
#include <Lunaris/Graphics/display.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>

#include <stdexcept>
#include <string>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>

namespace Lunaris {

	void __display_menu_allegro_start();


	enum class menu_item_flags {
		DISABLED = ALLEGRO_MENU_ITEM_DISABLED,
		AS_CHECKBOX = ALLEGRO_MENU_ITEM_CHECKBOX,
		CHECKED = ALLEGRO_MENU_ITEM_CHECKED
	};


	struct __menu_each_generic {
	public:
		enum class menu_each_type { _INVALID = -1, DEFAULT, MENU, EMPTY };

		virtual std::vector<ALLEGRO_MENU_INFO> generate() const = 0;
		virtual menu_each_type get_type() const = 0;
		virtual bool operator==(const uint16_t) const = 0;
		virtual bool operator!=(const uint16_t) const = 0;
		virtual void update_self(ALLEGRO_MENU*) = 0;
	};

	class menu_each_empty : public __menu_each_generic {
	public:
		std::vector<ALLEGRO_MENU_INFO> generate() const;
		menu_each_type get_type() const;
		bool operator==(const uint16_t) const;
		bool operator!=(const uint16_t) const;
		void update_self(ALLEGRO_MENU*);
	};

	class menu_each_default : public __menu_each_generic {
		std::string name;
		uint16_t id = 0;
		int flags = 0;
	public:
		menu_each_default() = default;
		menu_each_default(const std::string&, const uint16_t, const int = 0);

		menu_each_default& set_name(const std::string&);
		menu_each_default& set_id(const uint16_t);
		menu_each_default& set_flags(std::vector<menu_item_flags>);
		menu_each_default& set_flags(const int);

		const std::string& get_name() const;
		uint16_t get_id() const;
		int get_flags() const;

		std::vector<ALLEGRO_MENU_INFO> generate() const;
		menu_each_type get_type() const;
		bool operator==(const uint16_t) const;
		bool operator!=(const uint16_t) const;
		void update_self(ALLEGRO_MENU*);
	};

	class menu_each_menu : public __menu_each_generic {
		std::string name;
		uint16_t id = 0;
		int flags = 0;
		std::vector<std::shared_ptr<__menu_each_generic>> sub_menus;
	public:
		menu_each_menu() = default;
		menu_each_menu(std::vector<__menu_each_generic>);
		menu_each_menu(const std::string&, const uint16_t, const int = 0);
		menu_each_menu(const std::string&, const uint16_t, const int, std::vector<__menu_each_generic>);

		menu_each_menu& set_name(const std::string&);
		menu_each_menu& set_id(const uint16_t);
		menu_each_menu& set_flags(std::vector<menu_item_flags>);
		menu_each_menu& set_flags(const int);
		menu_each_menu& push(const menu_each_empty&);
		menu_each_menu& push(const menu_each_default&);
		menu_each_menu& push(const menu_each_menu&);

		std::string get_name() const;
		uint16_t get_id() const;
		int get_flags() const;
		const std::vector<std::shared_ptr<__menu_each_generic>>& get_sub_menus() const;
		std::vector<std::shared_ptr<__menu_each_generic>>& get_sub_menus();

		std::vector<ALLEGRO_MENU_INFO> generate() const;
		menu_each_type get_type() const;
		bool operator==(const uint16_t) const;
		bool operator!=(const uint16_t) const;
		void update_self(ALLEGRO_MENU*);
	};

	class menu : public NonCopyable {
	public:
		enum class menu_type { 
			BAR,	// Expected bar within display 
			POPUP	// Pop up right next to the mouse
		};
	private:
		std::vector<menu_each_menu> menus;
		std::shared_ptr<ALLEGRO_MENU> ptr;
		ALLEGRO_EVENT_SOURCE* ev_source = nullptr;
		display& disp;
		bool showing = false;
		menu_type latest = menu_type::BAR;

		void make_happen(const menu_type);
	public:
		menu(display&, std::vector<menu_each_menu>, const menu_type = menu_type::BAR);
		~menu();

		menu_each_menu& index(const size_t);
		size_t size() const;
		// update from allegro itself
		void update();

		// patch allegro side. update() if you want up to date remake information
		bool patch_name_of(const uint16_t, const std::string&);
		// patch allegro side. update() if you want up to date remake information
		bool patch_flags_of(const uint16_t, const int);
		// patch allegro side. update() if you want up to date remake information
		bool patch_toggle_flag(const uint16_t, const menu_item_flags);

		void show();
		void hide();
		void remake_as(const menu_type);

		operator std::vector<ALLEGRO_EVENT_SOURCE*>() const;
		ALLEGRO_EVENT_SOURCE* get_event_source() const;
	};

	class menu_event : public NonCopyable, public NonMovable {
		// cached stuff:
		ALLEGRO_MENU* source = nullptr;
		std::string from;
		uint16_t id = 0;
		int flags = 0;
		menu& _ref;
	public:
		menu_event(menu&, const ALLEGRO_EVENT&);

		bool valid() const;

		const std::string& get_name() const;
		uint16_t get_id() const;
		int get_flags() const;

		bool has_flag(const menu_item_flags) const;
		bool is_checkbox() const;
		bool is_checked() const;
		bool is_enabled() const;
		bool is_disabled() const;

		void patch_name(const std::string&);
		void patch_flags(const int);
		void patch_toggle_flag(const menu_item_flags);

		menu& get_source();
	};

	using menu_event_handler = specific_event_handler<menu_event, menu>;

}