#include "config.h"

namespace LSW {
	namespace v5 {
		namespace Interface {
			
			void Config::_set(section& i, const std::string& key, const std::string& val)
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
			
			Config::section& Config::_get_to_set(const std::string& sec, const config::config_section_mode mode)
			{
				for (auto& i : conff) {
					if (i.section_name == sec) {
						return i;
					}
				}
				conff.push_back({ mode, sec });
				return conff.back();
			}

			const Config::section& Config::_get(const std::string& sec) const
			{
				for (const auto& i : conff) {
					if (i.section_name == sec) {
						return i;
					}
				}
				throw Handling::Abort(__FUNCSIG__, "There is no section called " + sec + "!", Handling::abort::abort_level::GIVEUP);
				return conff.front();
			}

			bool Config::_has(const std::string& sec) const
			{
				for (const auto& i : conff) {
					if (i.section_name == sec) return true;
				}
				return false;
			}

			Config::Config(Config&& c) noexcept
			{
				path = std::move(c.path);
				autosave = c.autosave;
				conff = std::move(c.conff);
			}

			Config::~Config()
			{
				if (autosave) flush();
			}

			void Config::auto_save(bool b)
			{
				autosave = b;
			}

			bool Config::load(std::string str)
			{
				if (str.empty()) {
					debug("A configuration could not be loaded because path was null!");
					//throw Handling::Abort(__FUNCSIG__, "A configuration could not be loaded because path was null!");
					return false;
				}

				Handling::handle_path(str);
				ALLEGRO_CONFIG* conf = nullptr;
				conf = al_load_config_file(str.c_str());
				if (!conf) {
					debug("Failed to load configuration file: " + str);
					//throw Handling::Abort(__FUNCSIG__, "Failed to open config '" + str + "'!");
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

							debug(std::string(section) + " > " + key + " = " + result);

							backk.key_and_value.push_back({ key, result });

						} while (key);

					} while (section);

				}
				al_destroy_config(conf);
				return true;
			}

			void Config::save_path(std::string str)
			{
				if (str.empty()) {
					debug("Null path is not a valid path at save_path!");
					throw Handling::Abort(__FUNCSIG__, "Null path is not a valid path at save_path!");
				}

				Handling::handle_path(str);
				path = str;
			}

			void Config::flush()
			{
				if (path.empty()) {
					debug("A configuration could not be saved because path was null!");
					return;
				}

				ALLEGRO_CONFIG* conf = al_create_config();
				if (!conf) {
					debug("Failed to create a buffer to flush config.");
					throw Handling::Abort(__FUNCSIG__, "Failed to create a buffer to flush config.");
				}

				for (const auto& i : conff) {
					if (i.mode != config::config_section_mode::SAVE) continue;
					if (!i.section_comment.empty()) al_add_config_comment(conf, i.section_name.c_str(), i.section_comment.c_str());
					for (auto& j : i.key_and_value) {
						al_set_config_value(conf, i.section_name.c_str(), j.first.c_str(), j.second.c_str());
					}
				}

				if (!al_save_config_file(path.c_str(), conf)) {
					al_destroy_config(conf);
					debug("Failed to save configuration file: " + path);
					throw Handling::Abort(__FUNCSIG__, "Failed to save config '" + path + "'!");
				}
				al_destroy_config(conf);

				debug("Saved configuration file: " + path);
			}

			void Config::set(const std::string& sec, const config::config_section_mode mode)
			{
				this->_get_to_set(sec, mode);
			}

			bool Config::has(const std::string& sec, const config::config_section_mode mode) const
			{
				for (const auto& i : conff) {
					if (i.section_name == sec) {
						return i.mode == mode;
					}
				}
				return false;
			}

			bool Config::has(const std::string& sec, const std::string& key, const config::config_section_mode mode) const
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

			bool Config::has(const std::string& sec) const
			{
				for (const auto& i : conff) {
					if (i.section_name == sec) {
						return true;
					}
				}
				return false;
			}

			bool Config::has(const std::string& sec, const std::string& key) const
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

			void Config::set(const std::string& sec, const std::string& key, const std::string& val)
			{
				auto& u = _get_to_set(sec);
				_set(u, key, val);
			}

			void Config::set(const std::string& sec, const std::string& key, const bool& val)
			{
				auto& u = _get_to_set(sec);
				_set(u, key, val ? "true" : "false");
			}

			void Config::set(const std::string& sec, const std::string& key, const char& val)
			{
				auto& u = _get_to_set(sec);
				_set(u, key, std::to_string(val));
			}

			void Config::set(const std::string& sec, const std::string& key, const int& val)
			{
				auto& u = _get_to_set(sec);
				_set(u, key, std::to_string(val));
			}

			void Config::set(const std::string& sec, const std::string& key, const float& val)
			{
				auto& u = _get_to_set(sec);
				_set(u, key, std::to_string(val));
			}

			void Config::set(const std::string& sec, const std::string& key, const double& val)
			{
				auto& u = _get_to_set(sec);
				_set(u, key, std::to_string(val));
			}

			void Config::set(const std::string& sec, const std::string& key, const unsigned& val)
			{
				auto& u = _get_to_set(sec);
				_set(u, key, std::to_string(val));
			}

			void Config::set(const std::string& sec, const std::string& key, const long& val)
			{
				auto& u = _get_to_set(sec);
				_set(u, key, std::to_string(val));
			}

			void Config::set(const std::string& sec, const std::string& key, const long long& val)
			{
				auto& u = _get_to_set(sec);
				_set(u, key, std::to_string(val));
			}

			void Config::set(const std::string& sec, const std::string& key, const unsigned long long& val)
			{
				auto& u = _get_to_set(sec);
				_set(u, key, std::to_string(val));
			}

			void Config::set(const std::string& sec, const std::string& key, const std::initializer_list<std::string>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					if (i.find(';') != std::string::npos) throw Handling::Abort(__FUNCSIG__, "Invalid character ';' in multiple values configuration!");
					res += i + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::initializer_list<bool>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::string(i ? "true" : "false") + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::initializer_list<char>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::initializer_list<int>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::initializer_list<float>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::initializer_list<double>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::initializer_list<unsigned>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::initializer_list<long>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::initializer_list<long long>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::initializer_list<unsigned long long>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::vector<std::string>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					if (i.find(';') != std::string::npos)
						throw Handling::Abort(__FUNCSIG__, "Invalid character ';' in multiple values configuration!");
					res += i + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::vector<bool>& vals)
			{
				std::string res;
				for (auto i : vals) {
					res += std::string(i ? "true" : "false") + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::vector<char>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::vector<int>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::vector<float>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::vector<double>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::vector<unsigned>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::vector<long>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::vector<long long>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::set(const std::string& sec, const std::string& key, const std::vector<unsigned long long>& vals)
			{
				std::string res;
				for (const auto& i : vals) {
					res += std::to_string(i) + ';';
				}
				if (res.length() > 1) res.pop_back();
				set(sec, key, "{" + res + "}");
			}

			void Config::comment(const std::string& sec, std::string comment)
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

			std::string Config::get(const std::string& sec, const std::string& key) const
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

			void Config::operator=(Config&& c) noexcept
			{
				path = std::move(c.path);
				autosave = c.autosave;
				conff = std::move(c.conff);
			}

			// instantiation (better compile times)

			template void Config::ensure(const std::string&, const std::string&, const bool&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const char&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const int&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const float&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const double&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const unsigned&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const long&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const long long&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const unsigned long long&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const int64_t&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const uint64_t&, const config::config_section_mode);

			template void Config::ensure(const std::string&, const std::string&, const std::initializer_list<bool>&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const std::initializer_list<char>&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const std::initializer_list<int>&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const std::initializer_list<float>&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const std::initializer_list<double>&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const std::initializer_list<unsigned>&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const std::initializer_list<long>&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const std::initializer_list<long long>&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const std::initializer_list<unsigned long long>&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const std::initializer_list<int64_t>&, const config::config_section_mode);
			template void Config::ensure(const std::string&, const std::string&, const std::initializer_list<uint64_t>&, const config::config_section_mode);

			template bool Config::get_as(const std::string&, const std::string&) const;
			template char Config::get_as(const std::string&, const std::string&) const;
			template int Config::get_as(const std::string&, const std::string&) const;
			template float Config::get_as(const std::string&, const std::string&) const;
			template double Config::get_as(const std::string&, const std::string&) const;
			template unsigned Config::get_as(const std::string&, const std::string&) const;
			template long Config::get_as(const std::string&, const std::string&) const;
			template long long Config::get_as(const std::string&, const std::string&) const;
			template unsigned long long Config::get_as(const std::string&, const std::string&) const;

			template std::vector<bool> Config::get_array(const std::string&, const std::string&) const;
			template std::vector<char> Config::get_array(const std::string&, const std::string&) const;
			template std::vector<int> Config::get_array(const std::string&, const std::string&) const;
			template std::vector<float> Config::get_array(const std::string&, const std::string&) const;
			template std::vector<double> Config::get_array(const std::string&, const std::string&) const;
			template std::vector<unsigned> Config::get_array(const std::string&, const std::string&) const;
			template std::vector<long> Config::get_array(const std::string&, const std::string&) const;
			template std::vector<long long> Config::get_array(const std::string&, const std::string&) const;
			template std::vector<unsigned long long> Config::get_array(const std::string&, const std::string&) const;

		}
	}
}