#pragma once

#include <Windows.h>

#include <chrono>
#include <string>
#include <mutex>
#include <functional>

#include "../../Handling/Abort/abort.h" // fails calls throw
#include "../../Handling/Path/path.h" // handle_path used on init
#include "../../Tools/CString/cstring.h" // uses this as string internally to print colored chars
#include "../../Tools/Common/common.h" // sprintf_a

/*#define __FSR_DEF__ \
[]{ auto s = std::string(__FILE__); auto p = s.rfind('\\'); p = ((p != std::string::npos) ? p : s.rfind('/')); return ("&a" + ((p != std::string::npos && ((p + 1) < s.length())) ? s.substr(p + 1) : s)); }() \
+ std::string("&bL") + std::to_string(__LINE__) + std::string("&1>") + __FUNCDNAME__*/ 

namespace LSW {
	namespace v5 {
		namespace Interface {

			namespace logger {
				constexpr size_t max_lines_stored_by_memlog = 10;
				constexpr size_t each_line_stored_by_memlog = 140;
				constexpr size_t len_class = 45;
				constexpr size_t default_max_buffer_internally = 256;

				constexpr size_t macro_file_siz = 18;
				constexpr size_t macro_func_siz = 55;
			}

			enum class L { EL, SL, SLF, ELF };
			enum class E { INFO, WARN, ERRR, DEBUG };

			// valid on next printf call only (valid to: int, float, double, long, long long, size_t)
			
			/// <summary>
			/// <para>FormatAs is used to format a number, character or something in printf style.</para>
			/// <para>Example: you can use FormatAs("%.9lf") to print next Logger argument (double) with 9 decimal.</para>
			/// <para>So, LoggerObj &lt;&lt; ... &lt;&lt; FormatAs("%.9lf") &lt;&lt; a_double &lt;&lt; ...</para>
			/// </summary>
			class FormatAs {
				std::string format;
			public:
				FormatAs() = default;

				/// <summary>
				/// <para>Construct the format with a string.</para>
				/// </summary>
				/// <param name="{std::string}">The format string.</param>
				FormatAs(const std::string&);

				/// <summary>
				/// <para>Copy constructor.</para>
				/// </summary>
				/// <param name="{FormatAs}">A FormatAs to copy from.</param>
				FormatAs(const FormatAs&);

				/// <summary>
				/// <para>Get the string format so you can use it.</para>
				/// </summary>
				/// <returns>{std::string} The string format.</returns>
				const std::string& get_format() const;

				/// <summary>
				/// <para>Set format to empty so it can't be used anymore.</para>
				/// </summary>
				void clear();

				/// <summary>
				/// <para>If has set and hasn't clear()ed, true.</para>
				/// </summary>
				/// <returns>{bool} True if has something set.</returns>
				bool has_custom_format() const;
			};

			/// <summary>
			/// <para>Logger is a global logging system that allows multiple formats and custom ones too.</para>
			/// <para>Once initialized properly, it will log, debug and/or just show in console using a std::cout format.</para>
			/// <para>It has internal mutexes so multiple threads can log at the same time. Use L::SL/L::EL or L::SLF/L::ELF for logging StartLine/EndLine and for File copy (both console and file) (see the caps? SL/SLF = StartLine/StartLineFile?).</para>
			/// <para>EX: log = object of Logger:</para>
			/// <para>log &lt;&lt; L::SL &lt;&lt; "This shows up only in console" &lt;&lt; L::EL;</para>
			/// <para>log &lt;&lt; L::SLF &lt;&lt; "This shows up only in console and also save in log file" &lt;&lt; L::ELF;</para>
			/// <para>log &lt;&lt; L::SLF &lt;&lt; fsr(__FUNCSIG__, E::INFO) &lt;&lt; "This also uses the fsr tool for better info." &lt;&lt; L::ELF;</para>
			/// </summary>
			class Logger {
				struct __idata {
					Tools::cstring::C last_c = Tools::cstring::C::WHITE;
					bool bypass_last_c_once = false;

					FILE* fp = nullptr;
					std::string path = "log.log";
					std::mutex m;
#ifdef _DEBUG
					std::mutex dbgm;
#endif
					bool m_b = false;
					bool file_write_enabled = false;
					bool debug_to_file = true;
					bool skip_any_debug = false;

					/*ALLEGRO_EVENT_SOURCE evsrc = ALLEGRO_EVENT_SOURCE();

					Cstring memline[logger::max_lines_stored_by_memlog];
					size_t memlinecount = 0;*/

					Tools::Cstring memline_s;
					Tools::Cstring last_str;
					std::function<void(const Tools::Cstring&)> sendd;

					HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				};

				static __idata g;

				FormatAs latestFormat;

				static std::string _generate_date();

				void set_console_color(const Tools::cstring::C);
				void print_start();

				void send_last_and_cleanup();


				void print(const Tools::Cstring&);

				void fprint(FILE*&, const Tools::Cstring&);
			public:
				/// <summary>
				/// <para>Setup this as the logging file (it does handle path for you).</para>
				/// <para>This has to be set just once. Logging is global.</para>
				/// </summary>
				/// <param name="{std::string}">The path (you can use %tags%).</param>
				void init(std::string);

				/// <summary>
				/// <para>Flush log file.</para>
				/// </summary>
				void flush();

				/// <summary>
				/// <para>Whether debug info is saved into the file or just shown in console.</para>
				/// </summary>
				/// <param name="{bool}">Flush to file, if file set?</param>
				void debug_write_to_file(const bool);

				/// <summary>
				/// <para>Whether debug info is shown at all.</para>
				/// </summary>
				/// <param name="{bool}">Show all debug or ignore all.</param>
				void show_debug_anywhere(const bool);

				/// <summary>
				/// <para>Send debug information via debugging channel (when DEBUG mode).</para>
				/// <para>This uses OutputDebugInfo.</para>
				/// </summary>
				/// <param name="{std::string}">From where? (use fsr() for better results)</param>
				/// <param name="{std::string}">What to send to debug.</param>
				static void _debug(const std::string&, const std::string&);

				/// <summary>
				/// <para>Hook a function to be called every new line of logging.</para>
				/// </summary>
				/// <param name="{std::function}">A function to handle the logging data.</param>
				void hook(std::function<void(const Tools::Cstring&)>);

				/// <summary>
				/// <para>Unset the hook (optional logging function).</para>
				/// </summary>
				void unhook();

				/// <summary>
				/// <para>Get last line sent in console.</para>
				/// </summary>
				/// <returns>{Cstring} The string</returns>
				const Tools::Cstring& get_last_line() const;

				/// <summary>
				/// <para>Operator to write.</para>
				/// </summary>
				/// <param name="{L}">Start/end of a safe locked printing block.</param>
				/// <returns>{Logger} Itself.</returns>
				Logger& operator<<(const L&);

				/// <summary>
				/// <para>Operator to write.</para>
				/// </summary>
				/// <param name="{FormatAs}">Format next argument as.</param>
				/// <returns>{Logger} Itself.</returns>
				Logger& operator<<(const FormatAs&);

				/// <summary>
				/// <para>Operator to write.</para>
				/// </summary>
				/// <param name="{Cstring}">To be written.</param>
				/// <returns>{Logger} Itself.</returns>
				Logger& operator<<(const Tools::Cstring&);

				/// <summary>
				/// <para>Operator to write.</para>
				/// </summary>
				/// <param name="{std::string}">To be written.</param>
				/// <returns>{Logger} Itself.</returns>
				Logger& operator<<(const std::string&);

				/// <summary>
				/// <para>Operator to write.</para>
				/// </summary>
				/// <param name="{char[]}">To be written.</param>
				/// <returns>{Logger} Itself.</returns>
				Logger& operator<<(const char[]);

				/// <summary>
				/// <para>Operator to write.</para>
				/// </summary>
				/// <param name="{char}">To be written.</param>
				/// <returns>{Logger} Itself.</returns>
				Logger& operator<<(const char&);

				/// <summary>
				/// <para>Operator to write.</para>
				/// </summary>
				/// <param name="{int}">To be written.</param>
				/// <returns>{Logger} Itself.</returns>
				Logger& operator<<(const int&);

				/// <summary>
				/// <para>Operator to write.</para>
				/// </summary>
				/// <param name="{float}">To be written.</param>
				/// <returns>{Logger} Itself.</returns>
				Logger& operator<<(const float&);

				/// <summary>
				/// <para>Operator to write.</para>
				/// </summary>
				/// <param name="{double}">To be written.</param>
				/// <returns>{Logger} Itself.</returns>
				Logger& operator<<(const double&);
#ifdef LSW_X64
				/// <summary>
				/// <para>Operator to write.</para>
				/// </summary>
				/// <param name="{unsigned}">To be written.</param>
				/// <returns>{Logger} Itself.</returns>
				Logger& operator<<(const unsigned&);
#endif
				/// <summary>
				/// <para>Operator to write.</para>
				/// </summary>
				/// <param name="{long}">To be written.</param>
				/// <returns>{Logger} Itself.</returns>
				Logger& operator<<(const long&);

				/// <summary>
				/// <para>Operator to write.</para>
				/// </summary>
				/// <param name="{long long}">To be written.</param>
				/// <returns>{Logger} Itself.</returns>
				Logger& operator<<(const long long&);

				/// <summary>
				/// <para>Operator to write.</para>
				/// </summary>
				/// <param name="{size_t}">To be written.</param>
				/// <returns>{Logger} Itself.</returns>
				Logger& operator<<(const size_t&);
			};

			/// <summary>
			/// <para>FSR is a function to handle some common information for you.</para>
			/// <para>It's recommended to be called like fsr(__FUNCSIG__, E::INFO), where E is optional.</para>
			/// </summary>
			/// <param name="{std::string}">Normally __FUNCSIG__, aka function path.</param>
			/// <param name="{E}">Error level.</param>
			/// <returns>{Cstring} Color ready formatted string.</returns>
			//const Tools::Cstring fsr(Tools::Cstring, const E = E::INFO);

			// line, file, func, err
			const std::string _fsr(const size_t, const std::string&, const std::string&, const E = E::INFO);

#define fsr( ... ) LSW::v5::Interface::_fsr(__LINE__, __FILE__, __FUNCDNAME__, __VA_ARGS__)
#define debug(str) LSW::v5::Interface::Logger::_debug( _fsr(__LINE__ , __FILE__ , __FUNCDNAME__ , LSW::v5::Interface::E::DEBUG), str)

		}
	}
}