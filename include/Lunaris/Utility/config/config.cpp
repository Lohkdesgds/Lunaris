#include "config.h"

namespace Lunaris {

	void __config_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
	}

			
	void config::_set(section& i, const std::string& key, const std::string& val)
	{
		if (key.empty() || val.empty()) return;

		for (auto& j : i.key_and_value) {
			if (j.first == key) {
				j.second = val;
				return;
			}
		}
		i.key_and_value.push_back({ key, val });
		return;
	}
			
	config::section& config::_get_to_set(const std::string& sec, const config::config_section_mode mode)
	{
		for (auto& i : conff) {
			if (i.section_name == sec) {
				return i;
			}
		}
		conff.push_back({ mode, sec });
		return conff.back();
	}

	const config::section& config::_get(const std::string& sec) const
	{
		for (const auto& i : conff) {
			if (i.section_name == sec) {
				return i;
			}
		}
		throw std::runtime_error(("There is no section called " + sec + "!").c_str());
		return conff.front();
	}

	bool config::_has(const std::string& sec) const
	{
		for (const auto& i : conff) {
			if (i.section_name == sec) return true;
		}
		return false;
	}

	config::config(config&& c) noexcept
	{
		path = std::move(c.path);
		autosave = c.autosave;
		conff = std::move(c.conff);
	}

	config::~config()
	{
		if (autosave) flush();
	}

	void config::auto_save(bool b)
	{
		autosave = b;
	}

	bool config::load(std::string str)
	{
		__config_allegro_start();

		if (str.empty()) {
			return false;
		}

		ALLEGRO_CONFIG* conf = nullptr;
		conf = al_load_config_file(str.c_str());
		if (!conf) {
			return false;
		}

		path = str;

		ALLEGRO_CONFIG_SECTION* sect = nullptr;
		ALLEGRO_CONFIG_ENTRY* entr = nullptr;
		conff.clear();

		{
			const char* section = nullptr;

			do {
				if (!section) section = al_get_first_config_section(conf, &sect);
				else section = al_get_next_config_section(&sect);
				if (!section) break;

				conff.push_back({ config::config_section_mode::SAVE, section });
				auto& backk = conff.back();

				const char* key = nullptr;
				do {
					if (!entr) key = al_get_first_config_entry(conf, section, &entr);
					else key = al_get_next_config_entry(&entr);
					if (!key) break;

					const char* result = al_get_config_value(conf, section, key);

					backk.key_and_value.push_back({ key, result });

				} while (key);

			} while (section);

		}
		al_destroy_config(conf);
		return true;
	}

	void config::save_path(std::string str)
	{
		if (str.empty()) {
			throw std::runtime_error("Null path is not a valid path at save_path!");
		}
		path = str;
	}

	bool config::flush()
	{
		__config_allegro_start();

		if (path.empty()) return false;

		ALLEGRO_CONFIG* conf = al_create_config();
		if (!conf) return false;

		for (const auto& i : conff) {
			if (i.mode != config::config_section_mode::SAVE) continue;
			if (!i.section_comment.empty()) al_add_config_comment(conf, i.section_name.c_str(), i.section_comment.c_str());
			for (auto& j : i.key_and_value) {
				al_set_config_value(conf, i.section_name.c_str(), j.first.c_str(), j.second.c_str());
			}
		}

		if (!al_save_config_file(path.c_str(), conf)) {
			al_destroy_config(conf);
			return false;
		}
		al_destroy_config(conf);
		return true;
	}

	void config::set(const std::string& sec, const config::config_section_mode mode)
	{
		this->_get_to_set(sec, mode).mode = mode;
	}

	bool config::has(const std::string& sec, const config::config_section_mode mode) const
	{
		for (const auto& i : conff) {
			if (i.section_name == sec) {
				return i.mode == mode;
			}
		}
		return false;
	}

	bool config::has(const std::string& sec, const std::string& key, const config::config_section_mode mode) const
	{
		for (const auto& i : conff) {
			if (i.section_name == sec &&  i.mode == mode){
				for (auto& j : i.key_and_value) {
					if (j.first == key) {
						return true;
					}
				}
			}
		}
		return false;
	}

	bool config::has(const std::string& sec) const
	{
		for (const auto& i : conff) {
			if (i.section_name == sec) {
				return true;
			}
		}
		return false;
	}

	bool config::has(const std::string& sec, const std::string& key) const
	{
		for (const auto& i : conff) {
			if (i.section_name == sec) {
				for (auto& j : i.key_and_value) {
					if (j.first == key) {
						return true;
					}
				}
			}
		}
		return false;
	}

	void config::set(const std::string& sec, const std::string& key, const std::string& val)
	{
		auto& u = _get_to_set(sec);
		_set(u, key, val);
	}

	void config::set(const std::string& sec, const std::string& key, const bool& val)
	{
		auto& u = _get_to_set(sec);
		_set(u, key, val ? "true" : "false");
	}

	void config::set(const std::string& sec, const std::string& key, const char& val)
	{
		auto& u = _get_to_set(sec);
		_set(u, key, std::to_string(val));
	}

	void config::set(const std::string& sec, const std::string& key, const int& val)
	{
		auto& u = _get_to_set(sec);
		_set(u, key, std::to_string(val));
	}

	void config::set(const std::string& sec, const std::string& key, const float& val)
	{
		auto& u = _get_to_set(sec);
		_set(u, key, std::to_string(val));
	}

	void config::set(const std::string& sec, const std::string& key, const double& val)
	{
		auto& u = _get_to_set(sec);
		_set(u, key, std::to_string(val));
	}

	void config::set(const std::string& sec, const std::string& key, const unsigned& val)
	{
		auto& u = _get_to_set(sec);
		_set(u, key, std::to_string(val));
	}

	void config::set(const std::string& sec, const std::string& key, const long& val)
	{
		auto& u = _get_to_set(sec);
		_set(u, key, std::to_string(val));
	}

	void config::set(const std::string& sec, const std::string& key, const long long& val)
	{
		auto& u = _get_to_set(sec);
		_set(u, key, std::to_string(val));
	}

	void config::set(const std::string& sec, const std::string& key, const unsigned long long& val)
	{
		auto& u = _get_to_set(sec);
		_set(u, key, std::to_string(val));
	}

	void config::set(const std::string& sec, const std::string& key, const std::initializer_list<std::string>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			if (i.find(';') != std::string::npos) throw std::runtime_error("Invalid character ';' in multiple values configuration!");
			res += i + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::initializer_list<bool>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::string(i ? "true" : "false") + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::initializer_list<char>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::initializer_list<int>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::initializer_list<float>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::initializer_list<double>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::initializer_list<unsigned>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::initializer_list<long>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::initializer_list<long long>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::initializer_list<unsigned long long>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::vector<std::string>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			if (i.find(';') != std::string::npos)
				throw std::runtime_error("Invalid character ';' in multiple values configuration!");
			res += i + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::vector<bool>& vals)
	{
		std::string res;
		for (auto i : vals) {
			res += std::string(i ? "true" : "false") + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::vector<char>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::vector<int>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::vector<float>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::vector<double>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::vector<unsigned>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::vector<long>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::vector<long long>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::set(const std::string& sec, const std::string& key, const std::vector<unsigned long long>& vals)
	{
		std::string res;
		for (const auto& i : vals) {
			res += std::to_string(i) + ';';
		}
		if (res.length() > 1) res.pop_back();
		set(sec, key, "{" + res + "}");
	}

	void config::comment(const std::string& sec, std::string comment)
	{
		for (auto& i : comment) {
			if (i == '\n') i = ' ';
			else if (i == '#') i = '%';
		}

		for (auto& i : conff) {
			if (i.section_name == sec) {
				i.section_comment = comment;
				return;
			}
		}
		conff.push_back({ config::config_section_mode::SAVE, sec });
		auto& backk = conff.back();
		backk.section_comment = comment;
	}

	std::string config::get(const std::string& sec, const std::string& key) const
	{
		for (const auto& i : conff) {
			if (sec == i.section_name) {
				for (auto& j : i.key_and_value) {
					if (j.first == key) return j.second;
				}
			}
		}
		return "";
	}

	void config::operator=(config&& c) noexcept
	{
		path = std::move(c.path);
		autosave = c.autosave;
		conff = std::move(c.conff);
	}

	// instantiation (better compile times)

	template void config::ensure(const std::string&, const std::string&, const bool&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const char&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const int32_t&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const float&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const double&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const unsigned&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const long&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const long long&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const unsigned long long&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const int64_t&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const uint64_t&, const config::config_section_mode);

	template void config::ensure(const std::string&, const std::string&, const std::initializer_list<bool>&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const std::initializer_list<char>&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const std::initializer_list<int32_t>&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const std::initializer_list<float>&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const std::initializer_list<double>&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const std::initializer_list<unsigned>&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const std::initializer_list<long>&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const std::initializer_list<long long>&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const std::initializer_list<unsigned long long>&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const std::initializer_list<int64_t>&, const config::config_section_mode);
	template void config::ensure(const std::string&, const std::string&, const std::initializer_list<uint64_t>&, const config::config_section_mode);

	template bool config::get_as(const std::string&, const std::string&) const;
	template char config::get_as(const std::string&, const std::string&) const;
	template int32_t config::get_as(const std::string&, const std::string&) const;
	template float config::get_as(const std::string&, const std::string&) const;
	template double config::get_as(const std::string&, const std::string&) const;
	template unsigned config::get_as(const std::string&, const std::string&) const;
	template long config::get_as(const std::string&, const std::string&) const;
	template long long config::get_as(const std::string&, const std::string&) const;
	template unsigned long long config::get_as(const std::string&, const std::string&) const;

	template std::vector<bool> config::get_array(const std::string&, const std::string&) const;
	template std::vector<char> config::get_array(const std::string&, const std::string&) const;
	template std::vector<int32_t> config::get_array(const std::string&, const std::string&) const;
	template std::vector<float> config::get_array(const std::string&, const std::string&) const;
	template std::vector<double> config::get_array(const std::string&, const std::string&) const;
	template std::vector<unsigned> config::get_array(const std::string&, const std::string&) const;
	template std::vector<long> config::get_array(const std::string&, const std::string&) const;
	template std::vector<long long> config::get_array(const std::string&, const std::string&) const;
	template std::vector<unsigned long long> config::get_array(const std::string&, const std::string&) const;

}