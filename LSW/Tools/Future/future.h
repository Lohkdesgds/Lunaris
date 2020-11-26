#pragma once

// easier way to use anywhere else
//#define USE_STD_FULLY

#ifndef USE_STD_FULLY
#include "../../Handling/Abort/abort.h"
#endif

#include <future>
#include <functional>
#include <any>


#define TIMEOUT_MS 20000 // timeout if get() gets stuck for too long

namespace LSW {
	namespace v5 {
		namespace Tools {

			template<typename T>
			class then_block {
				std::function<void(T)> f{};
				bool promise_ran = false, promise_passed = false, f_ran = false;
				std::mutex m;
			public:
				/// <summary>
				/// <para>Sets the function (then function).</para>
				/// </summary>
				/// <param name="{std::function}">The then() function itself.</param>
				void set(std::function<void(T)> nf) {
					std::lock_guard<std::mutex> luck(m);
					f = nf;
				}

				/// <summary>
				/// <para>Gets if it has task to do (based on what have been done so far).</para>
				/// </summary>
				/// <returns>{bool} True if has task.</returns>
				bool has_future_task() {
					std::lock_guard<std::mutex> luck(m);
					return (promise_passed && !promise_ran) && f && !f_ran;
				}

				/// <summary>
				/// <para>Do the task as future/promise if needed.</para>
				/// </summary>
				/// <param name="{Q}">The value to be set.</param>
				/// <param name="{bool}">Is this promise?</param>
				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
				void run_if_not_yet(Q v, const bool is_this_promise) {
					std::lock_guard<std::mutex> luck(m);
					if (f && !f_ran) {
						f(v);
						f_ran = true;
						promise_ran |= is_this_promise;
					}
					promise_passed |= is_this_promise;
				}

				/// <summary>
				/// <para>Do the task as future/promise if needed. (void edition)</para>
				/// </summary>
				/// <param name="{bool}">Is this promise?</param>
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
				void run_if_not_yet(const bool is_this_promisse) {
					std::lock_guard<std::mutex> luck(m);
					if (f && !f_ran) {
						f();
						f_ran = true;
						promise_ran |= is_this_promisse;
					}
					promise_passed |= is_this_promisse;
				}
			};

			template<typename T>
			struct fake {
				T* var = nullptr;
				std::function<void(void)> del;
				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
				fake() { var = new T(); del = [&] { if (var) { delete var; var = nullptr; }}; }
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
				fake() {}

				~fake() { if (del) del(); }
			};

			/// <summary>
			/// <para>The Future holds or is about to hold a value in the future.</para>
			/// <para>You can use this to let a thread set the value assynchronously while doing other stuff.</para>
			/// </summary>
			template<typename T = void>
			class Future : std::future<T> {
				std::shared_ptr<then_block<T>> next = std::make_shared<then_block<T>>(); // internally a Promise<any> (thistype)
				fake<T> later_value;
				bool got_value_already = false;

				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
				void _end();
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
				void _end();
			public:

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{std::future}">The future to move.</param>
				Future(std::future<T>&& f) : std::future<T>(std::move(f)) { }

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{Future}">The Future to move.</param>
				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
				Future(Future&&) noexcept;

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{Future}">The Future to move.</param>
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
				Future(Future&&) noexcept;

				~Future();


				/// <summary>
				/// <para>Assign copy.</para>
				/// </summary>
				/// <param name="{std::future}">The future to move.</param>
				void operator=(std::future<T>&&);

				/// <summary>
				/// <para>Assign copy.</para>
				/// </summary>
				/// <param name="{Future}">The Future to move.</param>
				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
				void operator=(Future&&);

				/// <summary>
				/// <para>Assign copy.</para>
				/// </summary>
				/// <param name="{Future}">The Future to move.</param>
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
				void operator=(Future&&);

				/// <summary>
				/// <para>Gets the value (if it is set) or waits (with timeout) for it to be set.</para>
				/// <para>* if you want no timeout, wait() before get().</para>
				/// </summary>
				/// <returns>{T} The value.</returns>
				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
				T get();

				/// <summary>
				/// <para>Gets if has set or waits for signal (with timeout).</para>
				/// <para>* if you want no timeout, wait() before get().</para>
				/// </summary>
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
				void get();

				/// <summary>
				/// <para>Waits for the signal to get the value.</para>
				/// </summary>
				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
				void wait();

				/// <summary>
				/// <para>Waits for the signal.</para>
				/// </summary>
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
				void wait();

				/// <summary>
				/// <para>Gets if the variable is set.</para>
				/// </summary>
				/// <param name="{unsigned}">Max time to wait for status in milliseconds.</param>
				/// <returns>{bool} True if available to get().</returns>
				bool get_ready(unsigned = 0);


				/// <summary>
				/// <para>Sets a function to be called after value is set.</para>
				/// <para>The type the function returns is the Future type this function returns.</para>
				/// </summary>
				/// <param name="{std::function}">A function that has T as argument and returns a type Q.</param>
				/// <returns>{Future} A Future of type Q.</returns>
				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q> && !std::is_void_v<T>, int> = 0>
				Future<Q> then(std::function<Q(T)>);

				/// <summary>
				/// <para>Sets a function to be called after value is set.</para>
				/// <para>The type the function returns is the Future type this function returns.</para>
				/// </summary>
				/// <param name="{std::function}">A function that has T as argument and returns a type VOID.</param>
				/// <returns>{Future} A Future of type VOID.</returns>
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q> && !std::is_void_v<T>, int> = 0>
				Future<Q> then(std::function<void(T)>);

				/// <summary>
				/// <para>Sets a function to be called after value is set.</para>
				/// <para>The type the function returns is the Future type this function returns.</para>
				/// </summary>
				/// <param name="{std::function}">A function that has NO arguments and returns a type Q.</param>
				/// <returns>{Future} A Future of type Q.</returns>
				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q> && std::is_void_v<T>, int> = 0>
				Future<Q> then(std::function<Q(T)>);

				/// <summary>
				/// <para>Sets a function to be called after value is set.</para>
				/// <para>The type the function returns is the Future type this function returns.</para>
				/// </summary>
				/// <param name="{std::function}">A function that has NO arguments and returns a type VOID.</param>
				/// <returns>{Future} A Future of type VOID.</returns>
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q> && std::is_void_v<T>, int> = 0>
				Future<Q> then(std::function<void(T)>);

				/// <summary>
				/// <para>Sets a function to be called after value is set.</para>
				/// <para>The type the function in this case is forced to be the VOID VOID.</para>
				/// </summary>
				/// <param name="{std::function}">A function that has NO arguments and returns a type VOID.</param>
				/// <returns>{Future} A Future of type VOID.</returns>
				template<typename Q = T, typename Void = void, std::enable_if_t<!std::is_void_v<Q> && std::is_void_v<Void>, int> = 0>
				Future<void> then(std::function<void(Void)>);


				/// <summary>
				/// <para>Internal method because of templates not being friends of others from the same class, but not same type.</para>
				/// </summary>
				/// <param name="{std::function}">The function it is linked to. (forward setting)</param>
				/// <param name="{std::shared_ptr}">Copy of back then_block.</param>
				template<typename R, std::enable_if_t<!std::is_void_v<R>, int> = 0>
				void _set_next(std::function<void(T)>, std::shared_ptr<then_block<R>>);

				/// <summary>
				/// <para>Internal method because of templates not being friends of others from the same class, but not same type.</para>
				/// </summary>
				/// <param name="{std::function}">The function it is linked to. (forward setting)</param>
				/// <param name="{std::shared_ptr}">Copy of back then_block.</param>
				template<typename R, std::enable_if_t<std::is_void_v<R>, int> = 0>
				void _set_next(std::function<void(T)>, std::shared_ptr<then_block<R>>);

				/// <summary>
				/// <para>Internal method to get internal then (because different types are not friends).</para>
				/// </summary>
				/// <returns>{std::shared_ptr} The then_block block.</returns>
				std::shared_ptr<then_block<T>> _get_then();
			};



			/// <summary>
			/// <para>Promise is the one that can generate a Future and set its value later.</para>
			/// </summary>
			template<typename T = void>
			class Promise : std::promise<T> {
				bool got_future = false;
				std::function<T(void)> task_to_do;
				std::shared_ptr<then_block<T>> then_if;
				bool got_future_once = false;
				bool set_already_skip = false;
			public:
				Promise(const Promise&) = delete;

				/// <summary>
				/// <para>Default constructor.</para>
				/// </summary>
				Promise() : std::promise<T>() {}

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{Promise}">Promise to move.</param>
				Promise(Promise&&);

				/// <summary>
				/// <para>Move.</para>
				/// </summary>
				/// <param name="{Promise}">Promise to move.</param>
				void operator=(Promise&&) noexcept;

				/// <summary>
				/// <para>Constructor that sets up internal work function directly.</para>
				/// </summary>
				/// <param name="{std::function}">The function it will run when work() is called.</param>
				Promise(std::function<T(void)>);

				/// <summary>
				/// <para>Gets the Future associated to this Promise.</para>
				/// <para>Can only be called once per object.</para>
				/// </summary>
				/// <returns>{Future} The Future.</returns>
				Future<T> get_future();

				/// <summary>
				/// <para>Works (runs) the function set and sets as tasked.</para>
				/// <para>The value is set to the Future.</para>
				/// </summary>
				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
				void work();

				/// <summary>
				/// <para>Works (runs) the function set and sets as tasked.</para>
				/// <para>The Future will know the task has been done.</para>
				/// </summary>
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
				void work();

				/// <summary>
				/// <para>Has the value been set already?</para>
				/// </summary>
				/// <returns>{bool} True if has worked and/or the value has been set.</returns>
				bool has_set();

				/// <summary>
				/// <para>Sets as done and notifies Future.</para>
				/// </summary>
				/// <returns>{bool} True if it was set (not set before this call).</returns>
				template<typename Q = T>
				typename std::enable_if_t<std::is_void_v<Q>, bool> set_value();

				/// <summary>
				/// <para>Sets value and notifies Future.</para>
				/// </summary>
				/// <param name="{T}">The value to be set.</param>
				/// <returns>{bool} True if it was set (not set before this call).</returns>
				template<typename Q = T>
				typename std::enable_if_t<!std::is_void_v<Q>, bool> set_value(const Q&);
			};

			/// <summary>
			/// <para>In some weird cases you might need this, but it is not recommended, because this is not multithread.</para>
			/// <para>This sets a Future like a defined value already if you need to "cast" a value to a Future directly.</para>
			/// </summary>
			/// <param name="{T}">The value the Future will hold already.</param>
			/// <returns>{Future} The future of type T with the value you've set.</returns>
			template<typename T>
			Future<T> fake_future(const T&);
			
		}
	}
}

#include "future.ipp"