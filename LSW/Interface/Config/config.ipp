#pragma once

namespace LSW {
	namespace v5 {
		namespace Interface {
			
			template<typename T>
			inline void Config::ensure(const std::string& sec, const std::string& key, const T& defval, const config::config_section_mode mode)
			{
				if (!has(sec, key, mode)) {
					set(sec, mode);
					set(sec, key, defval);
				}
			}

			template<typename T>
			inline void Config::ensure(const std::string& sec, const std::string& key, const std::initializer_list<T>& defval, const config::config_section_mode mode)
			{
				if (!has(sec, key, mode)) {
					set(sec, mode);
					set(sec, key, defval);
				}
			}

			template<>
			inline bool Config::get_as(const std::string& sec, const std::string& key) const
			{
				auto res = get(sec, key);
				return res == "true";
			}

			template<>
			inline char Config::get_as(const std::string& sec, const std::string& key) const
			{
				auto res = get(sec, key);
				if (res.length() > 1) throw Handling::Abort(__FUNCSIG__, "Fatal error trying to recover get_as config.");
				return res[0];
			}
			
			template<>
			inline int Config::get_as(const std::string& sec, const std::string& key) const
			{
				auto res = get(sec, key);
				return std::atoi(res.c_str());
			}
			
			template<>
			inline float Config::get_as(const std::string& sec, const std::string& key) const
			{
				auto res = get(sec, key);
				return std::atof(res.c_str());
			}
			
			template<>
			inline double Config::get_as(const std::string& sec, const std::string& key) const
			{
				auto res = get(sec, key);
				double varr;
				if (!sscanf_s(res.c_str(), "%lf", &varr)) throw Handling::Abort(__FUNCSIG__, "Invalid key while trying to recover get_as config.");
				return varr;
			}
			
			template<>
			inline unsigned Config::get_as(const std::string& sec, const std::string& key) const
			{
				auto res = get(sec, key);
				unsigned varr;
				if (!sscanf_s(res.c_str(), "%u", &varr)) throw Handling::Abort(__FUNCSIG__, "Invalid key while trying to recover get_as config.");
				return varr;
			}
			
			template<>
			inline long Config::get_as(const std::string& sec, const std::string& key) const
			{
				auto res = get(sec, key);
				long varr;
				if (!sscanf_s(res.c_str(), "%ld", &varr)) throw Handling::Abort(__FUNCSIG__, "Invalid key while trying to recover get_as config.");
				return varr;
			}
			
			template<>
			inline long long Config::get_as(const std::string& sec, const std::string& key) const
			{
				auto res = get(sec, key);
				long long varr;
				if (!sscanf_s(res.c_str(), "%lld", &varr)) throw Handling::Abort(__FUNCSIG__, "Invalid key while trying to recover get_as config.");
				return varr;
			}

			template<> 
			inline unsigned long long Config::get_as(const std::string& sec, const std::string& key) const
			{
				auto res = get(sec, key);
				unsigned long long varr;
				if (!sscanf_s(res.c_str(), "%llu", &varr)) throw Handling::Abort(__FUNCSIG__, "Invalid key while trying to recover get_as config.");
				return varr;
			}

			template<>
			inline std::vector<std::string> Config::get_array(const std::string& sec, const std::string& key)  const
			{
				if (!_has(sec)) throw Handling::Abort(__FUNCSIG__, "There is no section called " + sec + " in this config file (" + path + ")");
				std::vector<std::string> vec;
				auto& s = _get(sec);
				for (auto& i : s.key_and_value) {
					if (i.first == key) {
						auto str = i.second;
						if (str.empty() || str.front() != '{' || str.back() != '}') throw Handling::Abort(__FUNCSIG__, "Fatal error trying to recover array-type config.");

						str.erase(str.begin());
						str.pop_back();

						std::stringstream ss(str);
						std::string _buf;

						while (std::getline(ss, _buf, ';')) {
							vec.push_back(_buf);
						}

						return std::move(vec);
					}
				}
				return vec;
			}

			template<>
			inline std::vector<bool> Config::get_array(const std::string& sec, const std::string& key)  const
			{
				if (!_has(sec)) throw Handling::Abort(__FUNCSIG__, "There is no section called " + sec + " in this config file (" + path + ")");
				std::vector<bool> vec;
				auto& s = _get(sec);
				for (auto& i : s.key_and_value) {
					if (i.first == key) {
						auto str = i.second;
						if (str.empty() || str.front() != '{' || str.back() != '}') throw Handling::Abort(__FUNCSIG__, "Fatal error trying to recover array-type config.");

						str.erase(str.begin());
						str.pop_back();

						std::stringstream ss(str);
						std::string _buf;

						while (std::getline(ss, _buf, ';')) {
							vec.push_back(_buf == "true");
						}

						return std::move(vec);
					}
				}
				return vec;
			}
			
			template<>
			inline std::vector<char> Config::get_array(const std::string& sec, const std::string& key)  const
			{
				if (!_has(sec)) throw Handling::Abort(__FUNCSIG__, "There is no section called " + sec + " in this config file (" + path + ")");
				std::vector<char> vec;
				auto& s = _get(sec);
				for (auto& i : s.key_and_value) {
					if (i.first == key) {
						auto str = i.second;
						if (str.empty() || str.front() != '{' || str.back() != '}') throw Handling::Abort(__FUNCSIG__, "Fatal error trying to recover array-type config.");

						str.erase(str.begin());
						str.pop_back();

						std::stringstream ss(str);
						std::string _buf;

						while (std::getline(ss, _buf, ';')) {
							if (_buf.length() > 1) throw Handling::Abort(__FUNCSIG__, "Invalid key while trying to recover array-type config.");
							vec.push_back(_buf[0]);
						}

						return std::move(vec);
					}
				}
				return vec;
			}

			template<>
			inline std::vector<int> Config::get_array(const std::string& sec, const std::string& key) const
			{
				if (!_has(sec)) throw Handling::Abort(__FUNCSIG__, "There is no section called " + sec + " in this config file (" + path + ")");
				std::vector<int> vec;
				auto& s = _get(sec);
				for (auto& i : s.key_and_value) {
					if (i.first == key) {
						auto str = i.second;
						if (str.empty() || str.front() != '{' || str.back() != '}') throw Handling::Abort(__FUNCSIG__, "Fatal error trying to recover array-type config.");

						str.erase(str.begin());
						str.pop_back();

						std::stringstream ss(str);
						std::string _buf;

						while (std::getline(ss, _buf, ';')) {
							vec.push_back(std::atoi(_buf.c_str()));
						}

						return std::move(vec);
					}
				}
				return vec;
			}
			
			template<>
			inline std::vector<float> Config::get_array(const std::string& sec, const std::string& key)  const
			{
				if (!_has(sec)) throw Handling::Abort(__FUNCSIG__, "There is no section called " + sec + " in this config file (" + path + ")");
				std::vector<float> vec;
				auto& s = _get(sec);
				for (auto& i : s.key_and_value) {
					if (i.first == key) {
						auto str = i.second;
						if (str.empty() || str.front() != '{' || str.back() != '}') throw Handling::Abort(__FUNCSIG__, "Fatal error trying to recover array-type config.");

						str.erase(str.begin());
						str.pop_back();

						std::stringstream ss(str);
						std::string _buf;

						while (std::getline(ss, _buf, ';')) {
							vec.push_back(std::atof(_buf.c_str()));
						}

						return std::move(vec);
					}
				}
				return vec;
			}
			
			template<>
			inline std::vector<double> Config::get_array(const std::string& sec, const std::string& key)  const
			{
				if (!_has(sec)) throw Handling::Abort(__FUNCSIG__, "There is no section called " + sec + " in this config file (" + path + ")");
				std::vector<double> vec;
				auto& s = _get(sec);
				for (auto& i : s.key_and_value) {
					if (i.first == key) {
						auto str = i.second;
						if (str.empty() || str.front() != '{' || str.back() != '}') throw Handling::Abort(__FUNCSIG__, "Fatal error trying to recover array-type config.");

						str.erase(str.begin());
						str.pop_back();

						std::stringstream ss(str);
						std::string _buf;

						while (std::getline(ss, _buf, ';')) {
							double varr;
							if (!sscanf_s(_buf.c_str(), "%lf", &varr)) throw Handling::Abort(__FUNCSIG__, "Invalid key while trying to recover array-type config.");
							vec.push_back(varr);
						}

						return std::move(vec);
					}
				}
				return vec;
			}

			template<> 
			inline std::vector<unsigned> Config::get_array(const std::string& sec, const std::string& key)  const
			{
				if (!_has(sec)) throw Handling::Abort(__FUNCSIG__, "There is no section called " + sec + " in this config file (" + path + ")");
				std::vector<unsigned> vec;
				auto& s = _get(sec);
				for (auto& i : s.key_and_value) {
					if (i.first == key) {
						auto str = i.second;
						if (str.empty() || str.front() != '{' || str.back() != '}') throw Handling::Abort(__FUNCSIG__, "Fatal error trying to recover array-type config.");

						str.erase(str.begin());
						str.pop_back();

						std::stringstream ss(str);
						std::string _buf;

						while (std::getline(ss, _buf, ';')) {
							unsigned varr;
							if (!sscanf_s(_buf.c_str(), "%u", &varr)) throw Handling::Abort(__FUNCSIG__, "Invalid key while trying to recover array-type config.");
							vec.push_back(varr);
						}

						return std::move(vec);
					}
				}
				return vec;
			}
			
			template<>
			inline std::vector<long> Config::get_array(const std::string& sec, const std::string& key) const
			{
				if (!_has(sec)) throw Handling::Abort(__FUNCSIG__, "There is no section called " + sec + " in this config file (" + path + ")");
				std::vector<long> vec;
				auto& s = _get(sec);
				for (auto& i : s.key_and_value) {
					if (i.first == key) {
						auto str = i.second;
						if (str.empty() || str.front() != '{' || str.back() != '}') throw Handling::Abort(__FUNCSIG__, "Fatal error trying to recover array-type config.");

						str.erase(str.begin());
						str.pop_back();

						std::stringstream ss(str);
						std::string _buf;

						while (std::getline(ss, _buf, ';')) {
							long varr;
							if (!sscanf_s(_buf.c_str(), "%ld", &varr)) throw Handling::Abort(__FUNCSIG__, "Invalid key while trying to recover array-type config.");
							vec.push_back(varr);
						}

						return std::move(vec);
					}
				}
				return vec;
			}
			
			template<>
			inline std::vector<long long> Config::get_array(const std::string& sec, const std::string& key)  const
			{
				if (!_has(sec)) throw Handling::Abort(__FUNCSIG__, "There is no section called " + sec + " in this config file (" + path + ")");
				std::vector<long long> vec;
				auto& s = _get(sec);
				for (auto& i : s.key_and_value) {
					if (i.first == key) {
						auto str = i.second;
						if (str.empty() || str.front() != '{' || str.back() != '}') throw Handling::Abort(__FUNCSIG__, "Fatal error trying to recover array-type config.");

						str.erase(str.begin());
						str.pop_back();

						std::stringstream ss(str);
						std::string _buf;

						while (std::getline(ss, _buf, ';')) {
							long long varr;
							if (!sscanf_s(_buf.c_str(), "%lld", &varr)) throw Handling::Abort(__FUNCSIG__, "Invalid key while trying to recover array-type config.");
							vec.push_back(varr);
						}

						return std::move(vec);
					}
				}
				return vec;
			}
			
			template<>
			inline std::vector<unsigned long long> Config::get_array(const std::string& sec, const std::string& key)  const
			{
				if (!_has(sec)) throw Handling::Abort(__FUNCSIG__, "There is no section called " + sec + " in this config file (" + path + ")");
				std::vector<unsigned long long> vec;
				auto& s = _get(sec);
				for (auto& i : s.key_and_value) {
					if (i.first == key) {
						auto str = i.second;
						if (str.empty() || str.front() != '{' || str.back() != '}') throw Handling::Abort(__FUNCSIG__, "Fatal error trying to recover array-type config.");

						str.erase(str.begin());
						str.pop_back();

						std::stringstream ss(str);
						std::string _buf;

						while (std::getline(ss, _buf, ';')) {
							unsigned long long varr;
							if (!sscanf_s(_buf.c_str(), "%llu", &varr)) throw Handling::Abort(__FUNCSIG__, "Invalid key while trying to recover array-type config");
							vec.push_back(varr);
						}

						return std::move(vec);
					}
				}
				return vec;
			}

		}
	}
}