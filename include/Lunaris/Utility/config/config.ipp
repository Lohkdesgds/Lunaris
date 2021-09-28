#pragma once

namespace Lunaris {
			
	template<typename T>
	inline void config::ensure(const std::string& sec, const std::string& key, const T& defval, const config_section_mode mode)
	{
		if (!has(sec, key, mode)) {
			set(sec, mode);
			set(sec, key, defval);
		}
	}

	template<typename T>
	inline void config::ensure(const std::string& sec, const std::string& key, const std::initializer_list<T>& defval, const config_section_mode mode)
	{
		if (!has(sec, key, mode)) {
			set(sec, mode);
			set(sec, key, defval);
		}
	}

	template<typename T>
	inline void config::ensure(const std::string& sec, const std::string& key, const std::vector<T>& defval, const config_section_mode mode)
	{
		if (!has(sec, key, mode)) {
			set(sec, mode);
			set(sec, key, defval);
		}
	}

	template<>
	inline bool config::get_as(const std::string& sec, const std::string& key) const
	{
		auto res = get(sec, key);
		return res == "true";
	}

	template<>
	inline char config::get_as(const std::string& sec, const std::string& key) const
	{
		auto res = get(sec, key);
		if (res.length() > 1) throw std::runtime_error("Fatal error trying to recover get_as config.");
		return res[0];
	}
			
	template<>
	inline int32_t config::get_as(const std::string& sec, const std::string& key) const
	{
		return std::stoi(get(sec, key));
	}
			
	template<>
	inline float config::get_as(const std::string& sec, const std::string& key) const
	{
		return std::stof(get(sec, key));
	}
			
	template<>
	inline double config::get_as(const std::string& sec, const std::string& key) const
	{
		return std::stod(get(sec, key));
	}
			
	template<>
	inline unsigned config::get_as(const std::string& sec, const std::string& key) const
	{
		return static_cast<unsigned>(std::stoul(get(sec, key)));
	}
			
	template<>
	inline long config::get_as(const std::string& sec, const std::string& key) const
	{
		return std::stol(get(sec, key));
	}
			
	template<>
	inline long long config::get_as(const std::string& sec, const std::string& key) const
	{
		return std::stoll(get(sec, key));
	}

	template<> 
	inline unsigned long long config::get_as(const std::string& sec, const std::string& key) const
	{
		return std::stoull(get(sec, key));
	}

	template<>
	inline std::vector<std::string> config::get_array(const std::string& sec, const std::string& key)  const
	{
		if (!_has(sec)) throw std::runtime_error("There is no section called " + sec + " in this config file (" + path + ")");
		std::vector<std::string> vec;
		auto& s = _get(sec);
		for (const auto& i : s.key_and_value) {
			if (i.first == key) {
				auto str = i.second;
				if (str.empty() || str.front() != '{' || str.back() != '}') throw std::runtime_error("Fatal error trying to recover array-type config.");

				str.erase(str.begin());
				str.pop_back();

				std::stringstream ss(str);
				std::string _buf;

				while (std::getline(ss, _buf, ';')) {
					vec.push_back(_buf);
				}

				return vec;
			}
		}
		return vec;
	}

	template<>
	inline std::vector<bool> config::get_array(const std::string& sec, const std::string& key)  const
	{
		if (!_has(sec)) throw std::runtime_error("There is no section called " + sec + " in this config file (" + path + ")");
		std::vector<bool> vec;
		auto& s = _get(sec);
		for (const auto& i : s.key_and_value) {
			if (i.first == key) {
				auto str = i.second;
				if (str.empty() || str.front() != '{' || str.back() != '}') throw std::runtime_error("Fatal error trying to recover array-type config.");

				str.erase(str.begin());
				str.pop_back();

				std::stringstream ss(str);
				std::string _buf;

				while (std::getline(ss, _buf, ';')) {
					vec.push_back(_buf == "true");
				}

				return vec;
			}
		}
		return vec;
	}
			
	template<>
	inline std::vector<char> config::get_array(const std::string& sec, const std::string& key)  const
	{
		if (!_has(sec)) throw std::runtime_error("There is no section called " + sec + " in this config file (" + path + ")");
		std::vector<char> vec;
		auto& s = _get(sec);
		for (const auto& i : s.key_and_value) {
			if (i.first == key) {
				auto str = i.second;
				if (str.empty() || str.front() != '{' || str.back() != '}') throw std::runtime_error("Fatal error trying to recover array-type config.");

				str.erase(str.begin());
				str.pop_back();

				std::stringstream ss(str);
				std::string _buf;

				while (std::getline(ss, _buf, ';')) {
					if (_buf.length() > 1) throw std::runtime_error("Invalid key while trying to recover array-type config.");
					vec.push_back(_buf[0]);
				}

				return vec;
			}
		}
		return vec;
	}

	template<>
	inline std::vector<int32_t> config::get_array(const std::string& sec, const std::string& key) const
	{
		if (!_has(sec)) throw std::runtime_error("There is no section called " + sec + " in this config file (" + path + ")");
		std::vector<int32_t> vec;
		auto& s = _get(sec);
		for (const auto& i : s.key_and_value) {
			if (i.first == key) {
				auto str = i.second;
				if (str.empty() || str.front() != '{' || str.back() != '}') throw std::runtime_error("Fatal error trying to recover array-type config.");

				str.erase(str.begin());
				str.pop_back();

				std::stringstream ss(str);
				std::string _buf;

				while (std::getline(ss, _buf, ';')) {
					vec.push_back(std::stoi(_buf));
				}

				return vec;
			}
		}
		return vec;
	}
			
	template<>
	inline std::vector<float> config::get_array(const std::string& sec, const std::string& key)  const
	{
		if (!_has(sec)) throw std::runtime_error("There is no section called " + sec + " in this config file (" + path + ")");
		std::vector<float> vec;
		auto& s = _get(sec);
		for (const auto& i : s.key_and_value) {
			if (i.first == key) {
				auto str = i.second;
				if (str.empty() || str.front() != '{' || str.back() != '}') throw std::runtime_error("Fatal error trying to recover array-type config.");

				str.erase(str.begin());
				str.pop_back();

				std::stringstream ss(str);
				std::string _buf;

				while (std::getline(ss, _buf, ';')) {
					vec.push_back(std::stof(_buf));
				}

				return vec;
			}
		}
		return vec;
	}
			
	template<>
	inline std::vector<double> config::get_array(const std::string& sec, const std::string& key)  const
	{
		if (!_has(sec)) throw std::runtime_error("There is no section called " + sec + " in this config file (" + path + ")");
		std::vector<double> vec;
		auto& s = _get(sec);
		for (const auto& i : s.key_and_value) {
			if (i.first == key) {
				auto str = i.second;
				if (str.empty() || str.front() != '{' || str.back() != '}') throw std::runtime_error("Fatal error trying to recover array-type config.");

				str.erase(str.begin());
				str.pop_back();

				std::stringstream ss(str);
				std::string _buf;

				while (std::getline(ss, _buf, ';')) {
					vec.push_back(std::stod(_buf));
				}

				return vec;
			}
		}
		return vec;
	}

	template<> 
	inline std::vector<unsigned> config::get_array(const std::string& sec, const std::string& key)  const
	{
		if (!_has(sec)) throw std::runtime_error("There is no section called " + sec + " in this config file (" + path + ")");
		std::vector<unsigned> vec;
		auto& s = _get(sec);
		for (const auto& i : s.key_and_value) {
			if (i.first == key) {
				auto str = i.second;
				if (str.empty() || str.front() != '{' || str.back() != '}') throw std::runtime_error("Fatal error trying to recover array-type config.");

				str.erase(str.begin());
				str.pop_back();

				std::stringstream ss(str);
				std::string _buf;

				while (std::getline(ss, _buf, ';')) {
					vec.push_back(static_cast<unsigned>(std::stoul(_buf)));
				}

				return vec;
			}
		}
		return vec;
	}
			
	template<>
	inline std::vector<long> config::get_array(const std::string& sec, const std::string& key) const
	{
		if (!_has(sec)) throw std::runtime_error("There is no section called " + sec + " in this config file (" + path + ")");
		std::vector<long> vec;
		auto& s = _get(sec);
		for (const auto& i : s.key_and_value) {
			if (i.first == key) {
				auto str = i.second;
				if (str.empty() || str.front() != '{' || str.back() != '}') throw std::runtime_error("Fatal error trying to recover array-type config.");

				str.erase(str.begin());
				str.pop_back();

				std::stringstream ss(str);
				std::string _buf;

				while (std::getline(ss, _buf, ';')) {
					vec.push_back(std::stol(_buf));
				}

				return vec;
			}
		}
		return vec;
	}
			
	template<>
	inline std::vector<long long> config::get_array(const std::string& sec, const std::string& key)  const
	{
		if (!_has(sec)) throw std::runtime_error("There is no section called " + sec + " in this config file (" + path + ")");
		std::vector<long long> vec;
		auto& s = _get(sec);
		for (const auto& i : s.key_and_value) {
			if (i.first == key) {
				auto str = i.second;
				if (str.empty() || str.front() != '{' || str.back() != '}') throw std::runtime_error("Fatal error trying to recover array-type config.");

				str.erase(str.begin());
				str.pop_back();

				std::stringstream ss(str);
				std::string _buf;

				while (std::getline(ss, _buf, ';')) {
					vec.push_back(std::stoll(_buf));
				}

				return vec;
			}
		}
		return vec;
	}
			
	template<>
	inline std::vector<unsigned long long> config::get_array(const std::string& sec, const std::string& key)  const
	{
		if (!_has(sec)) throw std::runtime_error("There is no section called " + sec + " in this config file (" + path + ")");
		std::vector<unsigned long long> vec;
		auto& s = _get(sec);
		for (const auto& i : s.key_and_value) {
			if (i.first == key) {
				auto str = i.second;
				if (str.empty() || str.front() != '{' || str.back() != '}') throw std::runtime_error("Fatal error trying to recover array-type config.");

				str.erase(str.begin());
				str.pop_back();

				std::stringstream ss(str);
				std::string _buf;

				while (std::getline(ss, _buf, ';')) {
					vec.push_back(std::stoull(_buf));
				}

				return vec;
			}
		}
		return vec;
	}

}