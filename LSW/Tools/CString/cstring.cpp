#include "cstring.h"

namespace LSW {
	namespace v5 {
		namespace Tools {

			Cstring Cstring::_update(const Cstring& rstr)
			{
				if (!rstr.size()) return Cstring{};
				cstring::C curr_color = rstr.front().cr;
				bool apply_new = false; // maybe add new color in line different than the one that was there when this became true, this = false?
				Cstring finalt;
				bool ignore_once = false;
				bool percentage_c = false;
				bool has_backslash_to_add = false;

				for (auto& i : rstr) {
					has_backslash_to_add = false;

					if (i.ch == '&' && !ignore_once) {
						percentage_c = true;
						continue;
					}

					if (i.ch == '\\' && !ignore_once) {
						ignore_once = true;
						continue;
					}
					else {
						if (ignore_once && (i.ch != '&' && i.ch != '\\')) has_backslash_to_add = true;
						ignore_once = false;
					}

					if (percentage_c) { // &color (0-9,a-f)
						percentage_c = false;
						char k = toupper(i.ch);
						if (k >= '0' && k <= '9') {
							curr_color = static_cast<cstring::C>(k - '0');
							last_added_color = curr_color;
							apply_new = true;
						}
						else if (k >= 'A' && k <= 'F') {
							curr_color = static_cast<cstring::C>(k - 'A' + 10);
							last_added_color = curr_color;
							apply_new = true;
						}
						else {
							finalt.push_back({ '&', apply_new ? curr_color : i.cr }); // wasn't &color, so + &
							finalt.push_back({ i.ch, apply_new ? curr_color : i.cr });
						}
						continue;
					}

					if (has_backslash_to_add) finalt.push_back({ '\\', apply_new ? curr_color : i.cr });
					finalt.push_back({ i.ch, apply_new ? curr_color : i.cr });
				}
				if (percentage_c) {
					finalt.push_back({ '&', apply_new ? curr_color : rstr.back().cr });
				}
				if (ignore_once) {
					finalt.push_back({ '\\', apply_new ? curr_color : rstr.back().cr });
				}

				return std::move(finalt);
			}
			
			Cstring::Cstring(Cstring&& m) noexcept
			{
				*this = std::move(m);
			}
			
			Cstring::Cstring(const Cstring& c)
			{
				*this = c;
			}

			Cstring::Cstring(const std::string& u)
			{
				clear();
				append(u);
			}

			Cstring::Cstring(const char_c& c)
			{
				clear();
				append(c);
			}

			Cstring::Cstring(const float& u)
			{
				clear();
				append(u);
			}

			Cstring::Cstring(const double& u)
			{
				clear();
				append(u);
			}

			Cstring::Cstring(const char* u)
			{
				clear();
				append(u);
			}

			Cstring::Cstring(const char& u)
			{
				clear();
				append(u);
			}

			Cstring::Cstring(const int& u)
			{
				clear();
				append(u);
			}

			Cstring::Cstring(const unsigned& u)
			{
				clear();
				append(u);
			}

			Cstring::Cstring(const long& u)
			{
				clear();
				append(u);
			}

			Cstring::Cstring(const long long& u)
			{
				clear();
				append(u);
			}

			Cstring::Cstring(const unsigned long& u)
			{
				clear();
				append(u);
			}

			Cstring::Cstring(const unsigned long long& u)
			{
				clear();
				append(u);
			}

			const size_t Cstring::find(const char c) const
			{
				for (size_t p = 0; p < str.size(); p++) {
					if (str[p].ch == c) return p;
				}
				return static_cast<size_t>(-1);
			}

			const size_t Cstring::find(const char_c c) const
			{
				for (size_t p = 0; p < str.size(); p++) {
					if (str[p] == c) return p;
				}
				return static_cast<size_t>(-1);
			}

			const size_t Cstring::rfind(const char c) const
			{
				if (str.size()) {
					for (size_t p = str.size() - 1; p > 0; p--) {
						if (str[p].ch == c) return p;
					}
					if (str[0].ch == c) return 0;
				}
				return static_cast<size_t>(-1);
			}

			const size_t Cstring::rfind(const char_c c) const
			{
				if (str.size()) {
					for (size_t p = str.size() - 1; p > 0; p--) {
						if (str[p] == c) return p;
					}
					if (str[0] == c) return 0;
				}
				return static_cast<size_t>(-1);
			}

			const size_t Cstring::find(const std::string& s) const
			{
				if (const size_t s_len = s.length(); s_len) {

					if (s_len > str.size()) return static_cast<int>(-1); // no way to find				

					for (size_t p = 0; p < (str.size() - s_len) + 1; p++) { // if both have same size, run once
						if (str[p].ch == s[0]) {
							bool bad = false;
							for (size_t start = 1; start < s_len && (start + p) < str.size() && !bad; start++) { // just to be sure, if someday I change the code...
								bad |= str[p + start].ch != s[start];
							}
							if (!bad) return p;
						}
					}
				}
				return static_cast<size_t>(-1);
			}

			const size_t Cstring::find(const Cstring& s) const
			{
				if (const size_t s_len = s.size(); s_len) {

					if (s_len > str.size()) return static_cast<int>(-1); // no way to find				

					for (size_t p = 0; p < (str.size() - s_len) + 1; p++) { // if both have same size, run once
						if (str[p] == s[0]) {
							bool bad = false;
							for (size_t start = 1; start < s_len && (start + p) < str.size() && !bad; start++) { // just to be sure, if someday I change the code...
								bad |= str[p + start] != s[start];
							}
							if (!bad) return p;
						}
					}
				}
				return static_cast<size_t>(-1);
			}

			Cstring Cstring::substr(const size_t start, const size_t len) const
			{
				Cstring cpy;

				for (size_t p = 0; (p + start) < str.size() && p < len; p++) cpy += str[p + start];

				if (start + len + 1 < str.size()) cpy.last_added_color = str[start + len + 1].cr;
				else cpy.last_added_color = last_added_color;

				return cpy;
			}

			std::string Cstring::s_str() const
			{
				std::string cpy;
				for (auto& i : str) cpy += i.ch;
				return cpy;
			}

			char_c Cstring::pop()
			{
				if (str.size() > 0) {
					char_c cpy = str.back();
					str.erase(str.end()-1);
					if (str.size() > 0) last_added_color = str.back().cr;
					return cpy;
				}
				else return char_c();
			}

			char_c Cstring::pop_front()
			{
				if (str.size() > 0) {
					char_c cpy = str.front();
					str.erase(str.begin());
					if (str.size() > 0) last_added_color = str.back().cr;
					return cpy;
				}
				else return char_c();
			}

			size_t Cstring::pop_utf8()
			{
				size_t pop_len = 0;
				if (str.size() > 0) {
					char_c* cp = str.data() + str.size();
					pop_len++;

					while (--cp >= str.data() && ((cp->ch & 0b10000000) && !(cp->ch & 0b01000000))) { pop_len++; last_added_color = cp->cr; }

					if (cp >= str.data())
						str.resize(cp - str.data());

					if (size()) last_added_color = back().cr;
				}
				return pop_len;
			}

			size_t Cstring::pop_front_utf8()
			{
				// https://www.instructables.com/Programming--how-to-detect-and-read-UTF-8-charact/
				size_t pop_len = 0;
				if (str.size() > 0) {
					char_c* cp = str.data();
					pop_len = 1;

					if (cp->ch & 0b10000000) { // detected
						while (++cp < str.data() + str.size() && ((cp->ch & 0b10000000) && !(cp->ch & 0b01000000))) { pop_len++; last_added_color = cp->cr; }
					}

					if (cp >= str.data())
						str.erase(str.begin(), str.begin() + pop_len);

				}
				return pop_len;
			}
			
			size_t Cstring::size() const
			{
				return str.size();
			}

			size_t Cstring::size_utf8() const
			{
				size_t len = 0;
				for (auto& i : str) {
					if ((i.ch & 0xC0) != 0x80) ++len;
				}
				return len;
			}

			Cstring& Cstring::refresh()
			{
				*this = _update(*this);
				return *this;
			}

			Cstring& Cstring::append(const Cstring& oth)
			{
				//str.clear();
				for (auto& i : oth) str.push_back(i);
				last_added_color = oth.last_added_color;

				return *this;
			}

			Cstring& Cstring::append(const std::string& rstr)
			{
				//str.clear();
				if (!rstr.size()) return *this;
				cstring::C curr_color = last_added_color;
				bool ignore_once = false;
				bool percentage_c = false;
				bool has_backslash_to_add = false;

				for (auto& i : rstr) {
					has_backslash_to_add = false;

					if (i == '&' && !ignore_once) {
						percentage_c = true;
						continue;
					}

					if (i == '\\' && !ignore_once) {
						ignore_once = true;
						continue;
					}
					else {
						if (ignore_once && (i != '&' && i != '\\')) has_backslash_to_add = true;
						ignore_once = false;
					}

					if (percentage_c) { // &color (0-9,a-f)
						percentage_c = false;
						char k = toupper(i);
						if (k >= '0' && k <= '9') {
							curr_color = static_cast<cstring::C>(k - '0');
							last_added_color = curr_color;
						}
						else if (k >= 'A' && k <= 'F') {
							curr_color = static_cast<cstring::C>(k - 'A' + 10);
							last_added_color = curr_color;
						}
						else {
							str.push_back({ '&', curr_color }); // wasn't &color, so + &
							str.push_back({ i, curr_color });
						}
						continue;
					}

					if (has_backslash_to_add) str.push_back({ '\\', curr_color });
					append({ i, curr_color });
				}
				if (percentage_c) {
					append({ '&', curr_color });
				}
				if (ignore_once) {
					append({ '\\', curr_color });
				}

				//auto& cpyy = str.at(str.size() - 1);

				return *this;
			}

			Cstring& Cstring::append(const char_c& a)
			{
				str.push_back(a);
				last_added_color = str.back().cr;
				return *this;
			}

			Cstring& Cstring::append(const float& a)
			{
				if (fabs(a) < 1e30f) return this->append(Tools::sprintf_a("%.3f", a));
				return this->append(a > 0 ? "+++" : "---");
			}

			Cstring& Cstring::append(const double& a)
			{
				if (fabs(a) < 1e100) return this->append(Tools::sprintf_a("%.5lf", a));
				return this->append(a > 0 ? "+++" : "---");
			}

			Cstring& Cstring::append(const char* a)
			{
				return this->append(std::string(a));
			}

			Cstring& Cstring::append(const char& a)
			{
				return this->append(std::string(&a, sizeof(char)));
			}

			Cstring& Cstring::append(const cstring::C& c)
			{
				last_added_color = c;
				return *this;
			}

			Cstring& Cstring::append(const int& a)
			{
				return this->append(std::to_string(a));
			}

			Cstring& Cstring::append(const unsigned& a)
			{
				return this->append(std::to_string(a));
			}

			Cstring& Cstring::append(const long& a)
			{
				return this->append(std::to_string(a));
			}

			Cstring& Cstring::append(const long long& a)
			{
				return this->append(std::to_string(a));
			}

			Cstring& Cstring::append(const unsigned long& a)
			{
				return this->append(std::to_string(a));
			}

			Cstring& Cstring::append(const unsigned long long& a)
			{
				return this->append(std::to_string(a));
			}

			Cstring& Cstring::operator+=(const Cstring& a)
			{
				return this->append(a);
			}

			Cstring& Cstring::operator+=(const std::string& a)
			{
				return this->append(a);
			}

			Cstring& Cstring::operator+=(const char_c& a)
			{
				return this->append(a);
			}

			Cstring& Cstring::operator+=(const float& a)
			{
				return this->append(a);
			}

			Cstring& Cstring::operator+=(const double& a)
			{
				return this->append(a);
			}

			Cstring& Cstring::operator+=(const char* a)
			{
				return this->append(a);
			}

			Cstring& Cstring::operator+=(const char& a)
			{
				return this->append(a);
			}

			Cstring& Cstring::operator+=(const cstring::C& a)
			{
				return this->append(a);
			}

			Cstring& Cstring::operator+=(const int& a)
			{
				return this->append(a);
			}

			Cstring& Cstring::operator+=(const unsigned& a)
			{
				return this->append(a);
			}

			Cstring& Cstring::operator+=(const long& a)
			{
				return this->append(a);
			}

			Cstring& Cstring::operator+=(const long long& a)
			{
				return this->append(a);
			}

			Cstring& Cstring::operator+=(const unsigned long& a)
			{
				return this->append(a);
			}

			Cstring& Cstring::operator+=(const unsigned long long& a)
			{
				return this->append(a);
			}

			Cstring Cstring::operator+(const Cstring& a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			Cstring Cstring::operator+(const std::string& a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			Cstring Cstring::operator+(const char_c& a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			Cstring Cstring::operator+(const float& a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			Cstring Cstring::operator+(const double& a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			Cstring Cstring::operator+(const char* a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			Cstring Cstring::operator+(const char& a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			Cstring Cstring::operator+(const cstring::C& a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			Cstring Cstring::operator+(const int& a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			Cstring Cstring::operator+(const unsigned& a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			Cstring Cstring::operator+(const long& a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			Cstring Cstring::operator+(const long long& a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			Cstring Cstring::operator+(const unsigned long& a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			Cstring Cstring::operator+(const unsigned long long& a) const
			{
				Cstring u = *this;
				return std::move(u.append(a));
			}

			void Cstring::operator=(const Cstring& c)
			{
				str = c.str;
				last_added_color = c.last_added_color;
			}

			void Cstring::operator=(Cstring&& m) noexcept
			{
				str = std::move(m.str);
				last_added_color = std::move(last_added_color);
			}

			void Cstring::operator=(const std::string& a)
			{
				str.clear();
				append(a);
			}

			void Cstring::operator=(const char_c& a)
			{
				str.clear();
				append(a);
			}

			void Cstring::operator=(const float& a)
			{
				str.clear();
				append(a);
			}

			void Cstring::operator=(const double& a)
			{
				str.clear();
				append(a);
			}

			void Cstring::operator=(const char* a)
			{
				str.clear();
				append(a);
			}

			void Cstring::operator=(const char& a)
			{
				str.clear();
				append(a);
			}

			void Cstring::operator=(const cstring::C& a)
			{
				str.clear();
				append(a);
			}

			void Cstring::operator=(const int& a)
			{
				str.clear();
				append(a);
			}

			void Cstring::operator=(const unsigned& a)
			{
				str.clear();
				append(a);
			}

			void Cstring::operator=(const long& a)
			{
				str.clear();
				append(a);
			}

			void Cstring::operator=(const long long& a)
			{
				str.clear();
				append(a);
			}

			void Cstring::operator=(const unsigned long& a)
			{
				str.clear();
				append(a);
			}

			void Cstring::operator=(const unsigned long long& a)
			{
				str.clear();
				append(a);
			}

			void Cstring::push_back(char_c&& u)
			{
				append(u);
			}

			std::vector<char_c>::iterator Cstring::begin()
			{
				return str.begin();
			}

			std::vector<char_c>::iterator Cstring::end()
			{
				return str.end();
			}

			std::vector<char_c>::const_iterator Cstring::begin() const
			{
				return str.begin();
			}

			std::vector<char_c>::const_iterator Cstring::end() const
			{
				return str.end();
			}

			char_c& Cstring::operator[](const size_t p)
			{
				if (p >= size()) throw Handling::Abort(__FUNCSIG__, "Out of range");
				return str[p];
			}
			
			const char_c& Cstring::operator[](const size_t p) const
			{
				if (p >= size()) throw Handling::Abort(__FUNCSIG__, "Out of range");
				return str[p];
			}
			
			const char_c& Cstring::front() const
			{
				if (size() == 0) throw Handling::Abort(__FUNCSIG__, "Empty string doesn't have front()!");
				return (*this)[0];
			}
			
			const char_c& Cstring::back() const
			{
				if (size() == 0) throw Handling::Abort(__FUNCSIG__, "Empty string doesn't have back()!");
				return (*this)[size()-1];
			}
			
			void Cstring::clear()
			{
				str.clear();
			}
			
			char_c* Cstring::data()
			{
				return str.data();
			}
			
			const char_c* Cstring::data() const
			{
				return str.data();
			}
			
			const cstring::C& Cstring::next_color() const
			{
				return last_added_color;
			}
			
			Cstring Cstring::filter_ascii_range(const char a, const char b) const
			{
				Cstring nstr;
				for (auto& i : str) {
					if (i.ch >= a && i.ch <= b) {
						nstr.append(i);
					}
				}
				nstr.last_added_color = last_added_color;
				return nstr;
			}

			Cstring operator+(const std::string& str, const Cstring& cstr)
			{
				Cstring conv = str;
				return conv + cstr;
			}

		}
	}
}