#include "fastfunction.h"
#pragma once

namespace LSW {
	namespace v5 {
		namespace Tools {

			template<typename T>
			inline FastFunction<T>::FastFunction(const FastFunction& o)
			{
				*this = o;
			}

			template<typename T>
			inline FastFunction<T>::FastFunction(FastFunction&& o) noexcept
			{
				*this = std::move(o);
			}

			template<typename T>
			inline FastFunction<T>::FastFunction(const T& val)
			{
				set(val);
			}

			template<typename T>
			inline FastFunction<T>::FastFunction(const std::function<T(void)>& ff)
			{
				set(ff);
			}

			template<typename T>
			inline void FastFunction<T>::operator=(const FastFunction& o)
			{
				std::lock_guard<std::mutex> l1(gen_use);
				std::lock_guard<std::mutex> l2(o.gen_use);
				gen = o.gen;
				ref = o.ref;
				using_gen = o.using_gen;
			}

			template<typename T>
			inline void FastFunction<T>::operator=(FastFunction&& o) noexcept
			{
				std::lock_guard<std::mutex> l1(gen_use);
				std::lock_guard<std::mutex> l2(o.gen_use);
				gen = std::move(o.gen);
				ref = o.ref;
				using_gen = o.using_gen;
				o.using_gen = false;
			}

			template<typename T>
			inline void FastFunction<T>::set(const T& t)
			{
				using_gen = false;
				ref = t;
			}

			template<typename T>
			inline void FastFunction<T>::set(const std::function<T(void)>& f)
			{
				using_gen = false;
				{
					std::lock_guard<std::mutex> l(gen_use);
					gen = f;
				}
				using_gen = true;
			}

			template<typename T>
			inline T FastFunction<T>::operator()()
			{
				return get();
			}

			template<typename T>
			inline T FastFunction<T>::get()
			{
				if (using_gen) {
					std::lock_guard<std::mutex> l(gen_use);
					ref = gen();
					return ref;
				}
				return ref;
			}

			template<typename T>
			inline T& FastFunction<T>::get_ref()
			{
				if (using_gen) {
					std::lock_guard<std::mutex> l(gen_use);
					ref = gen();
					return ref;
				}
				return ref;
			}

			template<typename T>
			inline bool FastFunction<T>::is_function() const
			{
				return using_gen;
			}

			template<typename T>
			inline std::function<T(void)> FastFunction<T>::get_f() const
			{
				std::lock_guard<std::mutex> l(gen_use);
				return gen;
			}

		}
	}
}