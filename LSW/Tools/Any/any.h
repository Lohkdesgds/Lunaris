#pragma once

#include <any>
#include <string>
#include "../Common/common.h"

namespace LSW {
	namespace v5 {
		namespace Tools {

			/// <summary>
			/// <para>Custom std::any that works with char arrays (transforming them to std::string)</para>
			/// <para>operator= and get template changed if T is char* or const char*</para>
			/// </summary>
			class Any : public std::any {
				std::string if_string_buf;
			public:
				using std::any::operator=;

				/// <summary>
				/// <para>Somehow just setting to use any's directly fail.</para>
				/// </summary>
				/// <param name="{args}">Arguments to init a std::any.</param>
				template<typename... Args> Any(Args...);

				/// <summary>
				/// <para>Set a string as variable (internally it will be saved as std::string).</para>
				/// </summary>
				/// <param name="{char*}">String.</param>
				/// <returns>{Any&} itself</returns>
				Any& operator=(const char*);

				/// <summary>
				/// <para>Compare internal data.</para>
				/// <para>THIS IS EXPERIMENTAL AND ONLY WORKS WITH POD/SMALL DATA!</para>
				/// </summary>
				/// <returns>{bool} True if equal.</returns>
				bool operator==(const Any&) const;

				/// <summary>
				/// <para>Get value directly.</para>
				/// </summary>
				/// <returns>{T} The value, if valid.</returns>
				template<typename T>
				T get();

				/// <summary>
				/// <para>Get value directly.</para>
				/// </summary>
				/// <returns>{char*} The value, if valid.</returns>
				template<>
				char* get();

				/// <summary>
				/// <para>Get value directly.</para>
				/// </summary>
				/// <returns>{char*} The value, if valid.</returns>
				template<typename T, std::enable_if_t<!std::is_same_v<r_cast_t<T>, char*>, int> = 0>
				T get() const;
			};

		}
	}
}

#include "any.ipp"