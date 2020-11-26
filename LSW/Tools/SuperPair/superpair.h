#pragma once

/*
= - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - =
SuperPair header:

- A subdivision of SuperMap. It has its own file because it is HUGE.
- SuperPair is each pair with any kind of key tied to a value. It is the std::pair, but with any (1 or more keys) to the same value.
= - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - =
*/


#include <stdio.h>

#include <mutex>
#include <vector>
#include <typeinfo>
#include <typeindex>
#include <any>

#include "../../Handling/Abort/abort.h"
#include "../Common/common.h"


namespace LSW {
	namespace v5 {
		namespace Tools {

			/// <summary>
			/// <para>SuperPair is something like std::pair, but it holds a value to any multiple keys.</para>
			/// </summary>
			template<typename T>
			class SuperPair {
				std::vector<std::any> keys;
				T holding{};

				// SUPER HANDLE //
				template<typename K, typename... Args>
				void handleInput(K&, Args...);
				template<typename K>
				void handleInput(K&);
				// SUPER HANDLE //

			public:
				SuperPair() = default;


				/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

				Import functions (auto copy)

				* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


				/// <summary>
				/// <para>Copy constructor.</para>
				/// </summary>
				/// <param name="{SuperPair}">A SuperPair of the same type to copy.</param>
				SuperPair(const SuperPair&);

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{SuperPair}">A SuperPair of the same type to move.</param>
				SuperPair(SuperPair&&) noexcept;

				/// <summary>
				/// <para>A way to set the value and some keys already.</para>
				/// </summary>
				/// <param name="{T}">The value it will hold.</param>
				/// <param name="{Args...}">Keys to the value.</param>
				template<typename... Args>
				SuperPair(const T&, Args...);

				/// <summary>
				/// <para>A way to set the value already.</para>
				/// </summary>
				/// <param name="{T}">The value it will hold.</param>
				SuperPair(const T&);

				/// <summary>
				/// <para>Copy operator.</para>
				/// </summary>
				/// <param name="{SuperPair}">A SuperPair of the same type to copy.</param>
				void operator=(const SuperPair&);

				/// <summary>
				/// <para>Move operator.</para>
				/// </summary>
				/// <param name="{SuperPair}">A SuperPair of the same type to move.</param>
				void operator=(SuperPair&&) noexcept;



				/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

				Simple functions

				* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


				/// <summary>
				/// <para>Sets the internal value directly.</para>
				/// </summary>
				/// <param name="{T}">The value to be set.</param>
				void set_value(const T&);

				/// <summary>
				/// <para>Gets the value it's holding.</para>
				/// </summary>
				/// <returns>{T} The value's reference.</returns>
				T& get_value();

				/// <summary>
				/// <para>Gets the value it's holding.</para>
				/// </summary>
				/// <returns>{T} The value's reference.</returns>
				const T& get_value() const;


				/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

				SET (non constant)

				* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


				/// <summary>
				/// <para>Sets or updates a key.</para>
				/// </summary>
				/// <param name="{Q}">A key.</param>
				template<typename Q, std::enable_if_t<!std::is_same_v<r_cast_t<Q>, char*>, int> = 0>
				void set(const Q&);

				/// <summary>
				/// <para>Sets or updates a key.</para>
				/// </summary>
				/// <param name="{char*}">A key.</param>
				void set(const char*);

				/// <summary>
				/// <para>Sets or updates a key.</para>
				/// </summary>
				/// <param name="{char*}">A key.</param>
				void set(char*);


				/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

				OPERATORS [], () (non constant)

				* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


				/// <summary>
				/// <para>If match, returns the pointer to value, else returns nullptr.</para>
				/// </summary>
				/// <param name="{Q}">A key.</param>
				/// <returns>{T} The value pointer or nullptr if the key doesn't match or doesn't exist.</returns>
				template<typename Q, std::enable_if_t<!std::is_same_v<r_cast_t<Q>, char*>,int> = 0>
				T* operator[](const Q&);

				/// <summary>
				/// <para>If match, returns the pointer to value, else returns nullptr.</para>
				/// </summary>
				/// <param name="{char*}">A key.</param>
				/// <returns>{T} The value pointer or nullptr if the key doesn't match or doesn't exist.</returns>
				T* operator[](const char*);

				/// <summary>
				/// <para>If match, returns the pointer to value, else returns nullptr.</para>
				/// </summary>
				/// <param name="{char*}">A key.</param>
				/// <returns>{T} The value pointer or nullptr if the key doesn't match or doesn't exist.</returns>
				T* operator[](char*);


				/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

				OPERATORS [], () (CONSTANT)

				* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


				/// <summary>
				/// <para>If match, returns the pointer to value, else returns nullptr.</para>
				/// </summary>
				/// <param name="{Q}">A key.</param>
				/// <returns>{T} The value pointer or nullptr if the key doesn't match or doesn't exist.</returns>
				template<typename Q, std::enable_if_t<!std::is_same_v<r_cast_t<Q>, char*>, int> = 0>
				const T* operator[](const Q&) const;

				/// <summary>
				/// <para>If match, returns the pointer to value, else returns nullptr.</para>
				/// </summary>
				/// <param name="{char*}">A key.</param>
				/// <returns>{T} The value pointer or nullptr if the key doesn't match or doesn't exist.</returns>
				const T* operator[](const char*) const;

				/// <summary>
				/// <para>If match, returns the pointer to value, else returns nullptr.</para>
				/// </summary>
				/// <param name="{char*}">A key.</param>
				/// <returns>{T} The value pointer or nullptr if the key doesn't match or doesn't exist.</returns>
				const T* operator[](char*) const;


				/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

				DEL (non constant)

				* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


				/// <summary>
				/// <para>Removes a specific key by type.</para>
				/// <para># PS: CHAR* and CONST CHAR* are transformed to std::string when they're set. You won't find char* type.</para>
				/// </summary>
				template<typename Q>
				void remove();


				/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

				has_type (constant)

				* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


				/// <summary>
				/// <para>Gets if a key of a type is set or not.</para>
				/// <para># PS: CHAR* and CONST CHAR* are transformed to std::string when they're set. You won't find char* type.</para>
				/// </summary>
				/// <returns>{bool} True if there is one of this type.</returns>
				template<typename Q>
				bool has_type() const;

				/// <summary>
				/// <para>Gets if there's this key there.</para>
				/// </summary>
				/// <returns>{bool} True if match.</returns>
				template<typename Q, std::enable_if_t<!std::is_same_v<r_cast_t<Q>, char*>, int> = 0>
				bool has_type(const Q& key) const;

				/// <summary>
				/// <para>Gets if there's this key there.</para>
				/// </summary>
				/// <returns>{bool} True if match.</returns>
				bool has_type(const char* key) const;

				/// <summary>
				/// <para>Gets if there's this key there.</para>
				/// </summary>
				/// <returns>{bool} True if match.</returns>
				bool has_type(char* key) const;


				/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

				get_type (constant)

				* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

				/// <summary>
				/// <para>Gets the key of type K.</para>
				/// <para># PS: CHAR* and CONST CHAR* are transformed to std::string when they're set. You won't find char* type.</para>
				/// </summary>
				/// <param name="{K}">The variable to be set with the value.</param>
				/// <returns></returns>
				template<typename K>
				bool get_type(K& val) const;
			};
		}
	}
}

#include "superpair.ipp"