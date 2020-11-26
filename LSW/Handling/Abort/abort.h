//
// abort.h
// *******
//
// Abort is used as an option to std::exception.
// - Includes AutoAbort (automatic timed abort).
// 
// Abort can be static_cast'ed or +variable to generate a std::string
//
// Revision by LSW
// 

#pragma once

#define WIN32_LEAN_AND_MEAN

#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <iostream>

namespace LSW {
	namespace v5 {
		namespace Handling {

			namespace abort {
				enum class abort_level { GIVEUP, FATAL_ERROR, OTHER };
			}

			/// <summary>
			/// <para>Abort is a exception handling class.</para>
			/// <para>All errors thrown in ths library should be Abort. You can get it as std::exception, but it only has what()...</para>
			/// <para>Abort give more information about where, what exactly and how bad is the error.</para>
			/// </summary>
			class Abort : public std::exception {
				std::string					s_from_where,
											s_details;
				abort::abort_level			abort_level = abort::abort_level::FATAL_ERROR;
			public:
				Abort() = delete;

				/// <summary>
				/// <para>Default constructor for a throw scenario.</para>
				/// </summary>
				/// <param name="{std::string}">Normally filled with __FUNCSIG__, the WHERE.</param>
				/// <param name="{std::string}">What happened? This is the description part.</param>
				/// <param name="{abort::abort_level}">Is it impossible to continue from here (FATAL_ERROR)? Or is it just a "get me out of here" (GIVEUP)?</param>
				Abort(const std::string&, const std::string&, const abort::abort_level = abort::abort_level::FATAL_ERROR);

				/// <summary>
				/// <para>Copy constructor.</para>
				/// <param name="{Abort}">Abort to copy</param>
				/// </summary>
				Abort(const Abort&);

				/// <summary>
				/// <para>Gets from where the exception has been thrown.</para>
				/// </summary>
				/// <returns>{std::string} Probably __FUNCSIG__ format.</returns>
				const std::string& get_from() const;

				/// <summary>
				/// <para>Gets the details about the throw.</para>
				/// </summary>
				/// <returns>{std::string} Details about the throw.</returns>
				const std::string& get_details() const;

				/// <summary>
				/// <para>Gets the level of the issue.</para>
				/// </summary>
				/// <returns>{abort::abort_level} The level of the issue.</returns>
				const abort::abort_level& get_level() const;

				/// <summary>
				/// <para>If you just want a simple string with all information, use this.</para>
				/// <para>Format: "From: FROM;\nDetails: DETAILS;\nLevel: LEVEL_AS_STRING;"</para>
				/// </summary>
				/// <returns>{std::string} Itself as one string with all information.</returns>
				operator std::string() const;
			};

			std::string operator+(const Abort&);
		}
	}
}