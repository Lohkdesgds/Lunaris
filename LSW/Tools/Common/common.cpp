#include "common.h"

namespace LSW {
	namespace v5 {
		namespace Tools {

			custom_random::custom_random()
			{
				seed = rd() ^ (
					static_cast<std::mt19937::result_type>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()) +
					static_cast<std::mt19937::result_type>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count())
					);
				gen = std::mt19937(seed);
			}

			unsigned custom_random::random()
			{
				return distrib(gen);
			}

			std::vector<std::string> generate_string_format(const std::string format, const size_t max, const size_t startat)
			{
				std::vector<std::string> rett;

				std::string start;
				size_t amountofzeros = 1;
				std::string end;

				short step = 0; // start, zeros, end

				// split
				for (auto& i : format)
				{
					switch (step) {
					case 0:
						if (i != '#') {
							start += i;
						}
						else {
							step++; // skip one amountofzeros
						}
						break;
					case 1:
						if (i == '#') {
							amountofzeros++;
						}
						else {
							step++;
							end += i;
						}
						break;
					case 2:
						end += i;
						break;
					}
				}

				std::string realformat = "%s%0" + std::to_string(amountofzeros) + "zu%s";

				char buff[512];

				for (size_t c = startat; c < max; c++) {
					sprintf_s(buff, realformat.c_str(), start.c_str(), c, end.c_str());
					rett.push_back(buff);
				}

				return rett;
			}

			std::wstring force_unicode(const std::string& s)
			{
				std::wstring _b;
				for (auto& i : s) _b += static_cast<wchar_t>(i);
				return std::move(_b);
			}

			uint64_t get_thread_id()
			{
				std::stringstream ss;
				ss << std::this_thread::get_id();
				return std::stoull(ss.str());
			}

			unsigned long long now()
			{
				return std::chrono::duration_cast<std::chrono::duration<unsigned long long, std::milli>>(std::chrono::system_clock::now().time_since_epoch()).count();
			}

			void sleep_for(const std::chrono::seconds dt)
			{
				std::this_thread::sleep_for(dt); // safe
			}

			void sleep_for(const std::chrono::milliseconds dt)
			{
				if (dt.count() > 10) return std::this_thread::sleep_for(dt);
				const auto tn = std::chrono::high_resolution_clock::now();
				while (std::chrono::high_resolution_clock::now() - tn < dt) std::this_thread::yield();
			}

			void sleep_for(const std::chrono::microseconds dt)
			{
				const auto tn = std::chrono::high_resolution_clock::now();
				while (std::chrono::high_resolution_clock::now() - tn < dt) std::this_thread::yield();
			}

			std::vector<bool> translate_binary(const int v, const size_t lim)
			{
				std::vector<bool> b;

				for (size_t actual_bin = 0; actual_bin < lim; actual_bin++)
				{
					b.push_back(v & (1 << actual_bin));
				}

				return b;
			}

			std::string byte_auto_string(double end, const size_t t, const bool space, const std::string& middle)
			{
				int prefix = -1;

				while (prefix < (common::known_size_len - 1) && (end /= (1e3)) >= 1.0)
					prefix++;
				
				end *= 1e3;

				/*char buf[1 << 7];
				char format[1 << 5];

				if (space) sprintf_s(format, "%c.%zulf %cs", '%', t, '%');
				else sprintf_s(format, "%c.%zulf%cs", '%', t, '%');


				sprintf_s(buf, format, end, prefix >= 0 ? common::known_size_ends[prefix] : "");

				return buf;*/

				return sprintf_a("%.*lf%s%s", t, end, ((space ? " " : "") + middle).c_str(), prefix >= 0 ? common::known_size_ends[prefix] : "");
			}

			const double limit_maximize(double gotten, const double prop)
			{
				return sin(gotten * 3.1415) * prop;
			}

			const unsigned int random()
			{
				static custom_random _rand_class; // sorry
				return _rand_class.random();
			}
			
			std::string generate_random_unique_string()
			{
				auto cool_n = random() % 1000; // 3 digits max
				auto now = std::chrono::system_clock::now().time_since_epoch();
				unsigned now_c = now.count() % static_cast<int>(1e10); // 115 days to repeat. Will you play for 115 days? even then there's a random number. You'll have to try REALLY HARD
				char sign[32];
				sprintf_s(sign, "%010u%03u", now_c, cool_n);
				return sign;
			}

			std::vector<std::pair<std::string, std::string>> break_lines_config(const std::string str, const std::string spr, const std::string comment, const std::string endline)
			{
				std::vector<std::pair<std::string, std::string>> vectu;
				std::pair<std::string, std::string> pair;
				bool flipswitch = false;
				bool ignore_now = false;

				for (size_t ii = 0; ii < str.size(); ii++)
				{
					if (endline.find(str[ii]) != std::string::npos) {
						if (pair.first.length() || pair.second.length()) vectu.push_back(pair);
						pair = std::pair<std::string, std::string>();
						flipswitch = false;
						ignore_now = false;
						continue;
					}
					if (comment.find(str[ii]) != std::string::npos) {
						ignore_now = true;
					}

					if (!ignore_now) {
						if (spr.find(str[ii]) != std::string::npos && !flipswitch) {
							while (ii < str.size() && spr.find(str[++ii]) != std::string::npos);
							ii--;
							flipswitch = true;
							continue;
						}

						if (!flipswitch) {
							pair.first += str[ii];
						}
						else {
							pair.second += str[ii];
						}
					}
				}

				if (pair.first.length() || pair.second.length()) {
					vectu.push_back(pair);
				}

				return vectu;
			}
			
			std::string fixed_size_string(std::string str, const size_t len, const char fill)
			{
				size_t p = str.size();
				for (; p < len; p++) str += '_';
				if (p > len) str = str.substr(0, len);
				return str;
			}

			std::string sprintf_a(const char* format, ...)
			{
				std::string str;

				va_list args;
				va_start(args, format);

				auto readd = vsnprintf(nullptr, 0, format, args); // str.data(), str.size()
				if (readd <= 0) throw Handling::Abort(__FUNCSIG__, "Invalid string input");

				str.resize(static_cast<size_t>(readd) + 1);
				readd = vsnprintf(str.data(), str.size(), format, args);

				va_end(args);

				str.resize(readd); // set what is there
				str.shrink_to_fit(); // clean up the rest of it (won't clear after it)
				return str;
			}

			bool operator==(const std::any& a, const std::any& b)
			{
				return a.type() == b.type() && memcmp(&a, &b, sizeof(std::any)) == 0;
			}

		}
	}
}