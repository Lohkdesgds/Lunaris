#include "path.h"

namespace Lunaris {

	LUNARIS_DECL void __path_allegro_start() {
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
	}

	LUNARIS_DECL char sep_path_to_char(const path_sep_e mod)
	{
		return mod == path_sep_e::NATIVE ? ALLEGRO_NATIVE_PATH_SEP : (mod == path_sep_e::FORWARD ? '/' : '\\');
	}

	LUNARIS_DECL void set_app_name(const std::string& nam)
	{
		__path_allegro_start();
		al_set_app_name(nam.c_str());
	}

	LUNARIS_DECL std::string get_app_name()
	{
		__path_allegro_start();
		return al_get_app_name();
	}

	LUNARIS_DECL std::string fix_path_to(std::string str, const path_sep_e mod)
	{
		char finl = sep_path_to_char(mod);
		const char possibilities[] = { '\\', '/' };

		for (auto& i : str) {
			if (std::find(std::begin(possibilities), std::end(possibilities), i) != std::end(possibilities)) i = finl;
		}

		return str;
	}

	LUNARIS_DECL std::string get_standard_path(int pth, const path_sep_e mod)
	{
		__path_allegro_start();
		auto path = al_get_standard_path(pth);
		std::string path_str = al_path_cstr(path, sep_path_to_char(mod));
		al_destroy_path(path);
		return path_str;
	}

	LUNARIS_DECL bool make_path(std::string path_str, const path_sep_e mod, const bool autofix)
	{
		char sep = sep_path_to_char(mod);
		if (autofix) path_str = fix_path_to(path_str, mod);

		bool has_to_check = false;

		for (size_t p = 0; p < path_str.size(); p++) {
			size_t newp = path_str.find(sep, p);
			if (newp < p || newp == std::string::npos) break; // find out of range?
			else p = newp;

			std::string tmp = path_str.substr(0, p);

			std::error_code err;
			if (!std::filesystem::create_directories(tmp, err) && err.value() != 0) {
				//cout << console::color::GOLD << "Create directories error:" << err.message();
				has_to_check = true;
			}
		}

		if (!has_to_check) return true;
		
		size_t rf = path_str.rfind(sep);
		if (rf == std::string::npos) return false;
		std::string tmp = path_str.substr(0, rf);
		return std::filesystem::exists(tmp);
	}

	LUNARIS_DECL size_t remove_all(std::string path, const bool autofix)
	{
		if (autofix) path = fix_path_to(path, path_sep_e::NATIVE);
		std::error_code err;
		size_t amount = std::filesystem::remove_all(path, err);
		if (err.value() != 0) return static_cast<size_t>(-1);
		return amount;
	}
}