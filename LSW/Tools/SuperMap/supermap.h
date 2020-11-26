#pragma once

/*
= - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - =
SuperMap header:

- SuperMap is an unlimited amount of SuperPairs combined into a giant 'std::map like' mapping configuration
- You can have multiple types (defined by each pair) leading to undefined values (no one really knows what's going on here lmao)
= - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - =
*/


// C
#include <stdio.h>
// C++
#include <mutex>
#include <vector>
#include <typeinfo>
#include <typeindex>

// import
#include "../SuperPair/superpair.h"

namespace LSW {
	namespace v5 {
		namespace Tools {

			/// <summary>
			/// <para>SuperMap is a std::map like class, but one that uses SuperPair as pairs.</para>
			/// <para>SuperPairs allow multiple different keys to result in a same value, and each value to have different key combinations.</para>
			/// </summary>
			template<typename T>
			class SuperMap {
				std::vector<SuperPair<T>> sps;
			public:
				SuperMap() = default;

				/// <summary>
				/// <para>Clears the map.</para>
				/// </summary>
				void clear();


				/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

				Import functions (auto copy)

				* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


				/// <summary>
				/// <para>Sets a list of multiple SuperPairs.</para>
				/// </summary>
				/// <param name="{initializer_list}">A initializer list of SuperPairs.</param>
				SuperMap(std::initializer_list<SuperPair<T>>);

				/// <summary>
				/// <para>Copy constructor.</para>
				/// </summary>
				/// <param name="{SuperMap}">The other SuperMap to copy.</param>
				SuperMap(const SuperMap&);

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{SuperMap}">The other SuperMap to move.</param>
				SuperMap(SuperMap&&) noexcept;

				/// <summary>
				/// <para>Copy operator.</para>
				/// </summary>
				/// <param name="{SuperMap}">The other SuperMap to copy.</param>
				void operator=(const SuperMap&);

				/// <summary>
				/// <para>Move operator.</para>
				/// </summary>
				/// <param name="{SuperMap}">The other SuperMap to move.</param>
				void operator=(SuperMap&&) noexcept;


				/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

				Simple functions

				* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


				/// <summary>
				/// <para>Adds a SuperMap into this SuperMap.</para>
				/// </summary>
				/// <param name="{SuperMap}">The SuperMap being added.</param>
				/// <returns></returns>
				void add(const SuperMap&);

				/// <summary>
				/// <para>Move a SuperMap into this SuperMap.</para>
				/// </summary>
				/// <param name="{SuperMap}">The SuperMap being moved.</param>
				/// <returns></returns>
				void add(SuperMap&&);

				/// <summary>
				/// <para>Adds a initializer list of multiple SuperPairs.</para>
				/// </summary>
				/// <param name="{initializer_list}">A initializer list of SuperPairs.</param>
				void add(std::initializer_list<SuperPair<T>>);

				/// <summary>
				/// <para>Adds a single SuperPair.</para>
				/// </summary>
				/// <param name="{SuperPair}">A SuperPair to add.</param>
				void add(const SuperPair<T>&);

				/// <summary>
				/// <para>Moves a single SuperPair.</para>
				/// </summary>
				/// <param name="{SuperPair}">A SuperPair to move.</param>
				void add(SuperPair<T>&&);

				/// <summary>
				/// <para>Begin of the vector of SuperPairs.</para>
				/// </summary>
				/// <returns>{iterator} Begin iterator.</returns>
				auto begin();

				/// <summary>
				/// <para>End of the vector of SuperPairs.</para>
				/// </summary>
				/// <returns>{iterator} End iterator.</returns>
				auto end();

				/// <summary>
				/// <para>Begin of the vector of SuperPairs.</para>
				/// </summary>
				/// <returns>{iterator} Begin iterator.</returns>
				auto begin() const;

				/// <summary>
				/// <para>End of the vector of SuperPairs.</para>
				/// </summary>
				/// <returns>{iterator} End iterator.</returns>
				auto end() const;

				/// <summary>
				/// <para>The size of the internal vector of SuperPairs.</para>
				/// </summary>
				/// <returns>{size_t} Vector size.</returns>
				size_t size() const;

				/// <summary>
				/// <para>Erases a specific position of the internal vector of SuperPairs.</para>
				/// </summary>
				/// <param name="{size_t}">The position to be deleted.</param>
				void erase(const size_t);

				/// <summary>
				/// <para>Erases an amount of SuperPairs of the internal vector.</para>
				/// </summary>
				/// <param name="{size_t}">The first position to be deleted.</param>
				/// <param name="{size_t}">The last position to be deleted.</param>
				void erase(const size_t, const size_t);


				/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

				GetPair (non constant)

				* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


				/// <summary>
				/// <para>Gets a SuperPair based on key if matches.</para>
				/// </summary>
				/// <param name="{K}">A key.</param>
				/// <returns>{SuperPair} The SuperPair with that key.</returns>
				template<typename K>
				SuperPair<T>* get_pair(const K&);


				/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

				OPERATORS [], () (non constant)

				* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


				/// <summary>
				/// <para>Get the value with that key.</para>
				/// </summary>
				/// <param name="{K}">The key.</param>
				/// <returns>{T} The value, if match, else nullptr.</returns>
				template<typename K>
				T* operator[](const K&);


				/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

				OPERATORS [], () (CONSTANT)

				* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


				/// <summary>
				/// <para>Get the value with that key.</para>
				/// </summary>
				/// <param name="{K}">The key.</param>
				/// <returns>{T} The value, if match, else nullptr.</returns>
				template<typename K>
				const T* operator[](const K&) const;
			};

		}
	}
}

#include "supermap.ipp"