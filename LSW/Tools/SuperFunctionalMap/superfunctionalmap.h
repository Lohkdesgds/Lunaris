#pragma once

#include <string>
#include <functional>
#include <type_traits>

#include "../FastFunction/fastfunction.h"
#include "../SuperMap/supermap.h"
#include "../Common/common.h"

namespace LSW {
	namespace v5 {
		namespace Tools {

			namespace base {
				enum class e_base { BASE, BASE_FUNC }; // virtual use to know who it is
			}

			/// <summary>
			/// <para>SuperFunctionalMap is a SuperMap, but aimed to handle with functions instead of values themselves. It can be transformed to a SuperMap and back to SuperFunctionalMap (if you're using a shared_ptr of SuperMap).</para>
			/// <para>This exists so you can directly get the value directly or function in a easier way. You can set a value as a function so the value changes as time goes by.</para>
			/// <para>———————————————————————————————————————————————————————</para>
			/// <para># NOTES:</para>
			/// <para>1. Internally this is a SuperMap of a FastFunction (function/var class). It has a shared_ptr of that. You can get back and set a new SuperMap of function as you please.</para>
			/// <para>2. get_direct&lt;T&gt; returns the value (not function, the value itself), while get_direct&lt;T*&gt; returns a pointer to the function (not value, the REAL function!)</para>
			/// </summary>
			template<typename BaseType>
			class SuperFunctionalMap {
			protected:
				std::shared_ptr<SuperMap<FastFunction<BaseType>>> map = std::make_shared<SuperMap<FastFunction<BaseType>>>();
			public:
				SuperFunctionalMap() = default;

				/// <summary>
				/// <para>Copy constructor, but from a similar SuperMap one.</para>
				/// </summary>
				/// <param name="{SuperMap}">A SuperMap of function.</param>
				/// <returns></returns>
				SuperFunctionalMap(const SuperMap<FastFunction<BaseType>>&);

				/// <summary>
				/// <para>Gets the shared_ptr of the SuperMap inside this object.</para>
				/// </summary>
				/// <returns>{SuperMap} Shared_ptr of SuperMap.</returns>
				template<typename Key, typename Compare = r_cast_t<Key>, std::enable_if_t<std::is_same<Compare, BaseType>::value, int> = 0>
				std::shared_ptr<SuperMap<FastFunction<BaseType>>> get();

				/// <summary>
				/// <para>Gets the shared_ptr of the SuperMap inside this object.</para>
				/// </summary>
				/// <returns>{SuperMap} Shared_ptr of SuperMap.</returns>
				template<typename Key, typename Compare = r_cast_t<Key>, std::enable_if_t<std::is_same<Compare, BaseType>::value, int> = 0>
				std::shared_ptr<SuperMap<FastFunction<BaseType>>> get() const;

				/// <summary>
				/// <para>Adds to the SuperMap internally.</para>
				/// </summary>
				/// <param name="{SuperMap}">Another SuperMap with other SuperPairs.</param>
				template<typename Key, typename Compare = r_cast_t<Key>, std::enable_if_t<std::is_same<Compare, BaseType>::value, int> = 0>
				void set(const SuperMap<FastFunction<BaseType>>&);

				/// <summary>
				/// <para>Moves to the SuperMap internally.</para>
				/// </summary>
				/// <param name="{SuperMap}">Another SuperMap with other SuperPairs.</param>
				template<typename Key, typename Compare = r_cast_t<Key>, std::enable_if_t<std::is_same<Compare, BaseType>::value, int> = 0>
				void set(SuperMap<FastFunction<BaseType>>&&);

				/// <summary>
				/// <para>Sets the internal shared_ptr to another SuperMap.</para>
				/// </summary>
				/// <param name="{shared_ptr}">The other SuperMap this should be referencing to.</param>
				template<typename Key, typename Compare = r_cast_t<Key>, std::enable_if_t<std::is_same<Compare, BaseType>::value, int> = 0>
				void set(const std::shared_ptr<SuperMap<FastFunction<BaseType>>>&);

				/// <summary>
				/// <para>Move the internal shared_ptr from another SuperMap.</para>
				/// </summary>
				/// <param name="{shared_ptr}">The other SuperMap this should be cutting from.</param>
				template<typename Key, typename Compare = r_cast_t<Key>, std::enable_if_t<std::is_same<Compare, BaseType>::value, int> = 0>
				void set(std::shared_ptr<SuperMap<FastFunction<BaseType>>>&&);

				/// <summary>
				/// <para>Adds a Key to return the Value (automatically creates a function that returns this value).</para>
				/// </summary>
				/// <param name="{Arg1}">A key.</param>
				/// <param name="{Arg2}">The value this key would return.</param>
				template<typename Key, typename Arg1, typename Compare = r_cast_t<Key>, std::enable_if_t<std::is_same<Compare, BaseType>::value, int> = 0>
				void set(const Arg1&, const Key&);

				/// <summary>
				/// <para>Adds a Key to return the Function.</para>
				/// </summary>
				/// <param name="{Arg1}">A key.</param>
				/// <param name="{Arg2}">The function this key would return.</param>
				template<typename Key, typename Arg1, typename Compare = r_cast_t<Key>,	std::enable_if_t<std::is_same<Compare, BaseType>::value, int> = 0>
				void set(const Arg1&, std::function<Key(void)>);

				/// <summary>
				/// <para>Gets a Value from Key. (if the function is complex, the value may not be always the same, of course)</para>
				/// </summary>
				/// <param name="{Arg1}">A key.</param>
				/// <param name="{Arg2}">The value.</param>
				/// <returns>{bool} True if success.</returns>
				template<typename Key, typename Arg1, typename Compare = r_cast_t<Key>,	std::enable_if_t<std::is_same<Compare, BaseType>::value, int> = 0>
				bool get(const Arg1&, Key&) const;

				/// <summary>
				/// <para>Gets a Function from Key.</para>
				/// </summary>
				/// <param name="{Arg1}">A key.</param>
				/// <param name="{Arg2}">The function.</param>
				/// <returns>{bool} True if success.</returns>
				template<typename Key, typename Arg1, typename Compare = r_cast_t<Key>,	std::enable_if_t<std::is_same<Compare, BaseType>::value, int> = 0>
				bool get(const Arg1&, std::function<Key(void)>&) const;

				/// <summary>
				/// <para>Gets directly value as return from Key. (if the function is complex, the value may not be always the same, of course)</para>
				/// </summary>
				/// <param name="{Arg1}">A key.</param>
				/// <returns>{Ret} A valid value if worked, else default value (like 0).</returns>
				template<typename Ret, typename Arg1, typename Compare = r_cast_t<Ret>, std::enable_if_t<(!std::is_pointer<Ret>::value&& std::is_same<Compare, BaseType>::value), int> = 0>
				Ret get_direct(const Arg1&) const;

				/// <summary>
				/// <para>Gets directly the function from Key.</para>
				/// </summary>
				/// <param name="{Arg1}">A key.</param>
				/// <returns>{std::function*} Not nullptr if the function was found.</returns>
				template<typename Ret, typename Arg1, typename Compare = r_cast_t<std::remove_pointer_t<Ret>>, std::enable_if_t<(std::is_pointer<Ret>::value && std::is_same<Compare, BaseType>::value), int> = 0>
				std::function<Compare(void)>* get_direct(const Arg1&);

				/// <summary>
				/// <para>Gets the exact holding value reference from Key.</para>
				/// </summary>
				/// <param name="{Arg1}">A key.</param>
				/// <returns>{std::function*} Not nullptr if the function was found.</returns>
				template<typename Ret, typename Arg1, typename Compare = r_cast_t<std::remove_pointer_t<Ret>>, std::enable_if_t<(std::is_reference<Ret>::value && std::is_same<Compare, BaseType>::value), int> = 0>
				FastFunction<Compare>* get_direct(const Arg1&);
			};

		}
	}
}

#include "superfunctionalmap.ipp"