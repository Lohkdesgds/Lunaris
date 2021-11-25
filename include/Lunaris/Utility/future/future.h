#pragma once

#include <Lunaris/__macro/macros.h>

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>

namespace Lunaris {

	template<typename T> class future;
	template<typename T> class promise;

	/// <summary>
	/// <para>This holds a not yet set variable. You can get the value itself or link a function to run when the variable is ready.</para>
	/// </summary>
	template<typename T>
	class future : public NonCopyable {
		// internal use
		struct _self {

			struct _int_data {
				T* value = nullptr;
				std::function<void(T)> _next;
				std::condition_variable triggered_result;
				std::mutex triggered_mutex;
				bool has_result = false;

				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0> void handle_start();
				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0> void handle_end();
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0> void handle_start();
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0> void handle_end();

				_int_data();
				~_int_data();

				_int_data(const _int_data&) = delete;
				_int_data(_int_data&&) = delete;
				void operator=(const _int_data&) = delete;
				void operator=(_int_data&&) = delete;

				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0> T* get_autowait();
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0> T* get_autowait();
			};

			_int_data _data;
			bool redirect = false;

			template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
			void post(T);
			template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
			void post();
		};

		// data //
		std::shared_ptr<_self> m_data = std::make_shared<_self>();

		// functions //
		future() = default;

		// friends //
		template<typename V> friend class promise;
		template<typename V> friend class future; // friend of any of this
	public:

		future(future&&) noexcept;
		void operator=(future&&) noexcept;

		/// <summary>
		/// <para>Get the value directly (if ready, else wait).</para>
		/// </summary>
		/// <returns>{T*} The value pointer (do not delete this).</returns>
		template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
		const T* get();

		/// <summary>
		/// <para>Get if signal was set. Returns if yes, else wait (like wait()).</para>
		/// </summary>
		template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
		void get();

		/// <summary>
		/// <para>Wait for the variable to be set.</para>
		/// </summary>
		void wait();

		/// <summary>
		/// <para>Set a function to handle variable when set (you should NOT get() after this).</para>
		/// </summary>
		/// <param name="{V}">V is a function or lambda that handles the required type and returns something or void.</param>
		/// <returns>{future&lt;V&gt;} Future of variable set by this function.</returns>
		template<typename V, typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0, typename Res = std::result_of_t<V(Q)>, std::enable_if_t<!std::is_void_v<Res>, int> = 0>
		auto then(V);

		/// <summary>
		/// <para>Set a function to handle variable when set (you should NOT get() after this).</para>
		/// </summary>
		/// <param name="{V}">V is a function or lambda that handles the required type and returns something or void.</param>
		/// <returns>{future&lt;V&gt;} Future of variable set by this function.</returns>
		template<typename V, typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0, typename Res = std::result_of_t<V()>, std::enable_if_t<!std::is_void_v<Res>, int> = 0>
		auto then(V);

		/// <summary>
		/// <para>Set a function to handle variable when set (you should NOT get() after this).</para>
		/// </summary>
		/// <param name="{V}">V is a function or lambda that handles the required type and returns something or void.</param>
		/// <returns>{future&lt;V&gt;} Future of variable set by this function.</returns>
		template<typename V, typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0, typename Res = std::result_of_t<V(Q)>, std::enable_if_t<std::is_void_v<Res>, int> = 0>
		auto then(V);

		/// <summary>
		/// <para>Set a function to handle variable when set (you should NOT get() after this).</para>
		/// </summary>
		/// <param name="{V}">V is a function or lambda that handles the required type and returns something or void.</param>
		/// <returns>{future&lt;V&gt;} Future of variable set by this function.</returns>
		template<typename V, typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0, typename Res = std::result_of_t<V()>, std::enable_if_t<std::is_void_v<Res>, int> = 0>
		auto then(V);
	};

	/// <summary>
	/// <para>You promise you'll have the value later, but not now!</para>
	/// <para>Create a future from this and set the value in the future, somewhere else.</para>
	/// </summary>
	template<typename T>
	class promise : public NonCopyable {
		std::function<void(T)> _next;
	public:
		promise() = default;

		promise(promise&&);
		void operator=(promise&&);

		/// <summary>
		/// <para>Get future so next value is saved somewhere.</para>
		/// </summary>
		/// <returns>{future&lt;T&gt;} Future ready linked to this promise.</returns>
		future<T> get_future();

		/// <summary>
		/// <para>Set value and send to future (if any).</para>
		/// </summary>
		/// <param name="{T}">Value to be set.</param>
		template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
		void set_value(const T&);

		/// <summary>
		/// <para>Set value and send to future (if any).</para>
		/// </summary>
		/// <param name="{T}">Value to be set.</param>
		template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
		void set_value();
	};
	
	/// <summary>
	/// <para>If you were about to do something, but it didn't work even before knowing it, you can promise with a response already set (future with value set already).</para>
	/// </summary>
	/// <param name="{T}">The pre-determined value for the future.</param>
	/// <returns>{future} The future with value already set.</returns>
	template<typename T, std::enable_if_t<!std::is_void_v<T>, int> = 0>
	future<T> make_empty_future(const T&);

	/// <summary>
	/// <para>If you were about to do something, but it didn't work even before knowing it, you can promise with a response already set (future with value set already).</para>
	/// </summary>
	/// <returns>{future} The future with value already set.</returns>
	template<typename T, std::enable_if_t<std::is_void_v<T>, int> = 0>
	future<T> make_empty_future();

}

#include "future.ipp"