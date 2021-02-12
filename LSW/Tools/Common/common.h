#pragma once

#include <chrono>
#include <string>
#include <sstream>
#include <vector>
#include <numeric>
#include <thread>
#include <random>
#include <any>
#include <stdarg.h>
#include <limits.h>

#include "../../Handling/Abort/abort.h"

#undef max

#define MILLI_NOW std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
// Check windows

// Any error here is because of Unicode. Use Multi-byte instead.

namespace LSW {
	namespace v5 {
		namespace Tools {

			namespace common {
				constexpr int known_size_len = 8;
				inline const char* known_size_ends[known_size_len] = { "k", "M", "G", "T", "P", "E", "Z", "Y" };
			}

			/* * * * * * * * * * *
			 >>>> CAST TOOLS <<<<
			* * * * * * * * * * */

			// like autocast, but for type
			template<typename T> using r_cast_t = std::conditional_t<std::is_pointer<T>::value || std::is_array<T>::value, std::add_pointer_t<std::remove_all_extents_t<std::remove_cv_t<std::remove_reference_t<std::remove_pointer_t<T>>>>>, std::remove_all_extents_t<std::remove_cv_t<std::remove_reference_t<std::remove_pointer_t<T>>>>>;


			template<typename, typename = void>
			constexpr bool is_type_complete_v = false;

			template<typename T>
			constexpr bool is_type_complete_v
				<T, std::void_t<decltype(sizeof(T))>> = true;

			//// end of cast tools ////

			class custom_random {
				std::random_device rd;
				std::mt19937::result_type seed;
				std::mt19937 gen;
				std::uniform_int_distribution<unsigned> distrib;
			public:
				custom_random();

				unsigned random();
			};

			/// <summary>
			/// <para>Generate strings with a simple format.</para>
			/// </summary>
			/// <param name="{std::string}">The string format (#### are replaced with the numbers) e.g. START###END.</param>
			/// <param name="{size_t}">Until what?</param>
			/// <param name="{size_t}">Starts with what number?</param>
			/// <returns>{std::vector} Generated strings</returns>
			std::vector<std::string> generate_string_format(const std::string, const size_t, const size_t = 0);

			/// <summary>
			/// <para>Project set to unicode? This may be useful. Emojis and special characters won't work properly.</para>
			/// </summary>
			/// <param name="{std::string}">A basic simple string.</param>
			/// <returns>{std::wstring} Converted string.</returns>
			std::wstring force_unicode(const std::string&);

			/// <summary>
			/// <para>Gets the thread ID.</para>
			/// <returns>{uint64_t} The thread ID.</returns>
			// gets Thread ID
			uint64_t get_thread_id();

			/// <summary>
			/// <para>Get time now since epoch in milliseconds.</para>
			/// </summary>
			/// <returns>{unsigned long long} Current time since epoch in milliseconds.</returns>
			unsigned long long now();

			/// <summary>
			/// <para>Sleep for some time, in seconds.</para>
			/// </summary>
			/// <param name="{std::chrono::seconds}">Time, in seconds.</param>
			void sleep_for(const std::chrono::seconds);

			/// <summary>
			/// <para>Sleep for some time, in milliseconds.</para>
			/// </summary>
			/// <param name="{std::chrono::milliseconds}">Time, in milliseconds.</param>
			void sleep_for(const std::chrono::milliseconds);

			/// <summary>
			/// <para>Sleep for some time, in microseconds.</para>
			/// </summary>
			/// <param name="{std::chrono::microseconds}">Time, in microseconds.</param>
			void sleep_for(const std::chrono::microseconds);

			/// <summary>
			/// <para>Transform bits into a vector of bools (up to 32 bits).</para>
			/// <para>You can transform a char to 8 bits (bools).</para>
			/// </summary>
			/// <param name="{int} The value itself."></param>
			/// <param name="{size_t} How many bits?"></param>
			/// <returns>{std::vector} The vector of bits.</returns>
			std::vector<bool> translate_binary(const int, const size_t);

			/// <summary>
			/// <para>Transforms a double to a string formatted like "1.42k". BASE 10.</para>
			/// <para>Magnitudes: k, M, G, T, P, E, Z and Y.</para>
			/// </summary>
			/// <param name="{double}">The value.</param>
			/// <param name="{size_t}">Numbers after the dot.</param>
			/// <param name="{bool}">Do you prefer 1.3k (false) or 1.3 k (true)? (the space between the number and magnetude).</param>
			/// <param name="{std::string}">Anything between the value and the magnitude?</param>
			/// <returns>{std::string} The string generated.</returns>
			std::string byte_auto_string(double, const size_t = 1, const bool = true, const std::string& = "");

			// https://stackoverflow.com/a/26914562
			/// <summary>
			/// <para>Transforms a UTF8 string to a UTF16.</para>
			/// </summary>
			/// <param name="{std::string}">UTF8 string.</param>
			/// <returns>{std::wstring} UTF16 string</returns>
			//std::wstring wideup(const std::string&);

			/// <summary>
			/// <para>Limits the value using sin().</para>
			/// </summary>
			/// <param name="{double}">Your value to limit.</param>
			/// <param name="{double}">Multiplier (range, [-multiplier, multiplier]).</param>
			/// <returns>{double} The value after conversion.</returns>
			const double limit_maximize(double, const double = 1.0);

			/// <summary>
			/// <para>Generates a random number.</para>
			/// </summary>
			/// <returns>{unsigned} A random number.</returns>
			const unsigned int random();

			/// <summary>
			/// <para>Generates a random number-based string based on time and RNG.</para>
			/// <para>It should not repeat in 115 days. You won't play the game for so long, right?</para>
			/// </summary>
			/// <returns>{std::string} A random generated string.</returns>
			std::string generate_random_unique_string();

			/// <summary>
			/// <para>Breaks a config-like data.</para>
			/// <para>Configurations normally looks like KEY:VALUE#COMMENT (w/ breakline, default).</para>
			/// </summary>
			/// <param name="{std::string}">The input itself.</param>
			/// <param name="{std::string}">Separator tag (like := ).</param>
			/// <param name="{std::string}">Comment tag (like #).</param>
			/// <param name="{std::string}">Division between configurations tag (like literally breakline).</param>
			/// <returns>{std::vector} Vector of pairs (key, value) (comments are ignored).</returns>
			std::vector<std::pair<std::string, std::string>> break_lines_config(const std::string, const std::string = ":= ", const std::string = "#", const std::string = "\r\n");

			/// <summary>
			/// <para>Do you want a fixed sized string? If smaller, characters are added; else, sliced.</para>
			/// </summary>
			/// <param name="{std::string}">Original string.</param>
			/// <param name="{size_t}">Desired size.</param>
			/// <param name="{char}">Fill.</param>
			/// <returns>{std::string} Fixed size generated string.</returns>
			std::string fixed_size_string(std::string, const size_t, const char = '_');


			/// <summary>
			/// <para>Works like a printf_s, but the output is std::string.</para>
			/// </summary>
			/// <param name="{const char*}">The format.</param>
			/// <param name="{...}">The arguments.</param>
			/// <returns>{std::string} Returns the string generated.</returns>
			std::string sprintf_a(const char*, ...);

			/// <summary>
			/// <para>Compare internal data.</para>
			/// <para>THIS IS EXPERIMENTAL AND ONLY WORKS WITH POD/SMALL DATA!</para>
			/// </summary>
			/// <param name="{std::any}">One.</param>
			/// <param name="{std::any}">Another.</param>
			/// <returns>{bool} True if equal.</returns>
			bool operator==(const std::any&, const std::any&);

		}
	}
}