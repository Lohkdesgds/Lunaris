#include "logger.h"
#include "../../_Macros/system_work.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			Logger::__idata Logger::g;

			FormatAs::FormatAs(const std::string& form)
			{
				if (form.find('%') == std::string::npos) throw Handling::Abort(__FUNCSIG__, "Invalid FORMAT! You have to use something like %d or %05.3lf as on a printf!");
				format = form;
			}

			FormatAs::FormatAs(const FormatAs& fa)
			{
				format = fa.format;
			}

			const std::string& FormatAs::get_format() const
			{
				return format;
			}

			void FormatAs::clear()
			{
				format.clear();
			}

			bool FormatAs::has_custom_format() const
			{
				return !format.empty();
			}

			std::string Logger::_generate_date()
			{
				char temp[26];

				time_t rawtime;
				tm ti;

				time(&rawtime);
				localtime_s(&ti, &rawtime);
				//ti = localtime(&rawtime);

				sprintf_s(temp, "[%02d/%02d/%02d-%02d:%02d:%02d]", (ti.tm_year + 1900) % 100, ti.tm_mon + 1, ti.tm_mday, ti.tm_hour, ti.tm_min, ti.tm_sec);

				return temp;
			}

			void Logger::set_console_color(const Tools::cstring::C cl)
			{
				if (cl != g.last_c || g.bypass_last_c_once) {
					g.bypass_last_c_once = false;
					int clr_c = static_cast<int>(cl);
					SetConsoleTextAttribute(g.hConsole, clr_c);
					g.last_c = cl;
				}
			}

			void Logger::print_start()
			{
				Tools::Cstring str;

				if (!g.fp && g.file_write_enabled) {
					str.append("&4[&eFILEERRR&4]");
				}
				str += Tools::cstring::C::GOLD;
				str.append(_generate_date());

				print(str);
			}

			void Logger::send_last_and_cleanup()
			{
				g.last_str = std::move(g.memline_s);
				if (g.sendd) g.sendd(g.last_str);
				//g.memline_s.clear();
			}

			void Logger::print(const Tools::Cstring& cstr) 
			{
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				int last_clr_c = -1;
				for (auto& i : cstr) {
					int clr_c = static_cast<int>(i.cr);
					if (last_clr_c != clr_c) {
						SetConsoleTextAttribute(hConsole, clr_c);
						last_clr_c = clr_c;
					}
					putchar(i.ch);
				}
			}
			
			void Logger::fprint(FILE*& fp, const Tools::Cstring& cstr) 
			{
				if (!fp) return;
				for (auto& i : cstr) {
					fputc(i.ch, fp);
				}
			}

			void Logger::init(std::string path)
			{
				if (g.fp) {
					throw Handling::Abort(__FUNCSIG__, "Path already set! Please don't change on the fly.", Handling::abort::abort_level::GIVEUP); // change later so you can?
				}

				g.m.lock();
				g.path = std::move(path);

				Handling::handle_path(g.path);

				if (!g.fp) {
					auto err = fopen_s(&g.fp, g.path.c_str(), "wb");
					if (err) {
						g.m.unlock();
						throw Handling::Abort(__FUNCSIG__, "Failed to open log: '" + g.path + "'", Handling::abort::abort_level::GIVEUP);
						return;
					}
				}

				g.m.unlock();
			}
			
			void Logger::flush()
			{
				if (g.fp) {
					g.m.lock();
					fflush(g.fp);
					g.m.unlock();
				}
			}
			
			void Logger::debug_write_to_file(const bool b)
			{
				g.debug_to_file = b;
			}

			void Logger::show_debug_anywhere(const bool b)
			{
				g.skip_any_debug = !b;
			}
			
			void Logger::_debug(const std::string& whr, const std::string& str)
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-outputdebugstringw
				// "Applications should send very minimal debug output and provide a way for the user to enable or disable its use." -> it disables the feature if in release mode then.
#ifdef _DEBUG
				g.dbgm.lock();
#ifdef UNICODE
				OutputDebugString(Tools::force_unicode("@\tLSWv5\t>\t" + _generate_date() + "\t\t\t" + whr + ": " + str + "\n").c_str());
#else
				OutputDebugString(("@\tLSWv5\t>\t" + _generate_date() + "\t\t\t" + whr + ": " + str + "\n").c_str());
#endif
				g.dbgm.unlock();
#endif
				Logger logg;
				if (!g.skip_any_debug) {
					if (g.debug_to_file) logg << L::SLF << whr << "&7" << str << L::ELF;
					else				 logg << L::SL << whr << "&7" << str << L::EL;
				}
			}
			
			void Logger::hook(std::function<void(const Tools::Cstring&)> f)
			{
				g.sendd = f;
			}
			
			void Logger::unhook()
			{
				hook(std::function<void(const Tools::Cstring&)>());
			}
			
			const Tools::Cstring& Logger::get_last_line()
			{
				return g.last_str;
			}
			
			Logger& Logger::operator<<(const L& u)
			{
				switch (u)
				{
				case L::SL: // START LINE
					g.m.lock();
					g.m_b = true;
					print_start();
					break;
				case L::EL: // END LINE
					*this << ('\n');
					if (!g.m_b) {
						*this << ("\n---------- FATAL WARN ----------\nMUTEX WAS NOT LOCKED! THE CODE HAS ERRORS!");
						flush();
						throw Handling::Abort(__FUNCSIG__, "FATAL ERROR MUTEX SHOULDN'T BE UNLOCKED IF IT WASN'T PREVIOUSLY!", Handling::abort::abort_level::FATAL_ERROR);
					}
					g.m_b = false;
					g.m.unlock();	/// yes, visual studio thinks this is an epic WARN, but it will never fail if you use gfile << L::SL(F) << fsr() << ... << L::EL(F)
					send_last_and_cleanup(); // send last coloured string to event and clear
					break;
				case L::SLF: // START LINE AND SAVE ON FILE
					g.m.lock();
					g.m_b = true;
					g.file_write_enabled = true;
					print_start();
					break;
				case L::ELF: // END LINE AND SAVE ON FILE
					*this << ('\n');
					if (!g.m_b) {
						*this << ("\n---------- FATAL WARN ----------\nMUTEX WAS NOT LOCKED! THE CODE HAS ERRORS!");
						flush();
						throw Handling::Abort(__FUNCSIG__, "FATAL ERROR MUTEX SHOULDN'T BE UNLOCKED IF IT WASN'T PREVIOUSLY!", Handling::abort::abort_level::FATAL_ERROR);
					}
					g.file_write_enabled = false;
					g.m.unlock();	/// yes, visual studio thinks this is an epic WARN, but it will never fail if you use gfile << L::SL(F) << fsr() << ... << L::EL(F)
					flush();
					send_last_and_cleanup(); // send last coloured string to event and clear
					break;
				}
				return *this;       /// yes, visual studio thinks this is an epic WARN, but it will never fail if you use gfile << L::SL(F) << fsr() << ... << L::EL(F)
			}

			Logger& Logger::operator<<(const FormatAs& nfa)
			{
				latestFormat = nfa;
				return *this;
			}

			Logger& Logger::operator<<(const Tools::Cstring& clstr)
			{
				print(clstr);
				if (g.file_write_enabled) fprint(g.fp, clstr);
				if (clstr.size()) {
					//g.last_c = clstr[clstr.size() - 1].cr;
					g.last_c = clstr.next_color();
				}
				g.memline_s += clstr;
				return *this;
			}
			
			Logger& Logger::operator<<(const std::string& o) // don't forget template lmao
			{
				Tools::Cstring cstr;
				char format[3];
				sprintf_s(format, "&%x", static_cast<int>(g.last_c));
				cstr = (format + o).c_str();
				return (this->operator<<(cstr));
			}
			
			Logger& Logger::operator<<(const char o[])
			{
				return (this->operator<<(std::string(o)));
			}
			
			Logger& Logger::operator<<(const char& o)
			{
				return (this->operator<<(std::string(&o, 1)));
			}
			
			Logger& Logger::operator<<(const int& o)
			{
				std::string buf;

				if (latestFormat.has_custom_format()) {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, latestFormat.get_format().c_str(), o)));
					sprintf_s(buf.data(), buf.size(), latestFormat.get_format().c_str(), o);
					latestFormat.clear();
				}
				else {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, "%d", o)));
					sprintf_s(buf.data(), buf.size(), "%d", o);
				}

				return (this->operator<<(buf));
			}
			
			Logger& Logger::operator<<(const float& o)
			{
				std::string buf;

				if (latestFormat.has_custom_format()) {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, latestFormat.get_format().c_str(), o)));
					sprintf_s(buf.data(), buf.size(), latestFormat.get_format().c_str(), o);
					latestFormat.clear();
				}
				else {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, "%.3f", o)));
					sprintf_s(buf.data(), buf.size(), "%.3f", o);
				}

				return (this->operator<<(buf));
			}
			
			Logger& Logger::operator<<(const double& o)
			{
				std::string buf;

				if (latestFormat.has_custom_format()) {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, latestFormat.get_format().c_str(), o)));
					sprintf_s(buf.data(), buf.size(), latestFormat.get_format().c_str(), o);
					latestFormat.clear();
				}
				else {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, "%.5lf", o)));
					sprintf_s(buf.data(), buf.size(), "%.5lf", o);
				}

				return (this->operator<<(buf));
			}
#ifdef LSW_X64
			Logger& Logger::operator<<(const unsigned& o)
			{
				std::string buf;

				if (latestFormat.has_custom_format()) {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, latestFormat.get_format().c_str(), o)));
					sprintf_s(buf.data(), buf.size(), latestFormat.get_format().c_str(), o);
					latestFormat.clear();
				}
				else {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, "%u", o)));
					sprintf_s(buf.data(), buf.size(), "%u", o);
				}

				return (this->operator<<(buf));
			}
#endif
			Logger& Logger::operator<<(const long& o)
			{
				std::string buf;

				if (latestFormat.has_custom_format()) {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, latestFormat.get_format().c_str(), o)));
					sprintf_s(buf.data(), buf.size(), latestFormat.get_format().c_str(), o);
					latestFormat.clear();
				}
				else {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, "%ld", o)));
					sprintf_s(buf.data(), buf.size(), "%ld", o);
				}

				return (this->operator<<(buf));
			}
			
			Logger& Logger::operator<<(const long long& o)
			{
				std::string buf;

				if (latestFormat.has_custom_format()) {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, latestFormat.get_format().c_str(), o)));
					sprintf_s(buf.data(), buf.size(), latestFormat.get_format().c_str(), o);
					latestFormat.clear();
				}
				else {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, "%lld", o)));
					sprintf_s(buf.data(), buf.size(), "%lld", o);
				}

				return (this->operator<<(buf));
			}
			
			Logger& Logger::operator<<(const size_t& o)
			{
				std::string buf;

				if (latestFormat.has_custom_format()) {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, latestFormat.get_format().c_str(), o)));
					sprintf_s(buf.data(), buf.size(), latestFormat.get_format().c_str(), o);
					latestFormat.clear();
				}
				else {
					buf.resize(1 + static_cast<size_t>(snprintf(nullptr, 0, "%zu", o)));
					sprintf_s(buf.data(), buf.size(), "%zu", o);
				}

				return (this->operator<<(buf));
			}

			const std::string _fsr(const size_t l, const std::string& fp, const std::string& func, const E e)
			{
				std::string back_str;
				switch (e) {
				case E::INFO:
					back_str = "&2[INFO]";
					break;
				case E::WARN:
					back_str = "&c[WARN]";
					break;
				case E::ERRR:
					back_str = "&4[ERRR]";
					break;
				case E::DEBUG:
					back_str = "&5[DEBG]";
					break;
				}
				return Tools::sprintf_a("%s&8[&bL%04zu&8>&a%s&8|&9%s&8]&f ",
					back_str.c_str(),
					l,
					Tools::fixed_size_string(
						[&] { auto s = std::string(fp); auto p = s.rfind('\\'); p = ((p != std::string::npos) ? p : s.rfind('/')); return ("&a" + ((p != std::string::npos && ((p + 1) < s.length())) ? s.substr(p + 1) : s)); }(),
						logger::macro_file_siz
					).c_str(),
					Tools::fixed_size_string(func, logger::macro_func_siz).c_str()
				);
			}

		}
	}
}