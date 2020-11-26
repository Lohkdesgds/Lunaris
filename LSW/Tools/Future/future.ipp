#pragma once

namespace LSW {
	namespace v5 {
		namespace Tools {

			/* * * * * * * * * * * * * * * * * * > FUTURE<T> < * * * * * * * * * * * * * * * * * */

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
			inline void Future<T>::_end()
			{
				if (next->has_future_task()) next->run_if_not_yet(get(), false);
			}

			template<typename T>
			template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
			inline void Future<T>::_end()
			{
				if (next->has_future_task()) next->run_if_not_yet(false);
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
			inline Future<T>::Future(Future<T>&& f) noexcept : std::future<T>(std::move(f))
			{
				next = f.next;
				*later_value.var = *f.later_value.var;
				got_value_already = f.got_value_already;
			}

			template<typename T>
			template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
			inline Future<T>::Future(Future<T>&& f) noexcept : std::future<T>(std::move(f))
			{
				next = f.next;
				got_value_already = f.got_value_already;
			}

			template<typename T>
			inline Future<T>::~Future()
			{
				_end<T>();
			}

			template<typename T>
			inline void Future<T>::operator=(std::future<T>&& f)
			{
				std::future<T>::operator=(std::move(f));
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
			inline void Future<T>::operator=(Future<T>&& f)
			{
				std::future<T>::operator=(std::move(f));
				next = f.next; 
				*later_value.var = *f.later_value.var; 
				got_value_already = f.got_value_already;
			}

			template<typename T>
			template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
			inline void Future<T>::operator=(Future<T>&& f)
			{
				std::future<T>::operator=(std::move(f));
				next = f.next;
				got_value_already = f.got_value_already;
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
			inline T Future<T>::get()
			{
				if (std::future<T>::valid())
				{
#ifndef USE_STD_FULLY
					if (!get_ready(TIMEOUT_MS)) throw Handling::Abort(__FUNCSIG__, "Blocked for too long! Variable can't be set!", Handling::abort::abort_level::GIVEUP); // if invalid after 10 sec, cancel?
#else
					if (!get_ready(TIMEOUT_MS)) throw std::exception("Blocked for too long! Variable can't be set!"); // if invalid after 10 sec, cancel?
#endif
					* later_value.var = std::future<T>::get();
					if (next->has_future_task()) next->run_if_not_yet(*later_value.var, false);
					got_value_already = true;
				}
				return *later_value.var;
			}

			template<typename T>
			template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
			inline void Future<T>::get()
			{
				if (std::future<T>::valid()) {
#ifndef USE_STD_FULLY
					if (!get_ready(TIMEOUT_MS)) throw Handling::Abort(__FUNCSIG__, "Blocked for too long! Variable can't be set!", Handling::abort::abort_level::GIVEUP); // if invalid after 10 sec, cancel?
#else
					if (!get_ready(TIMEOUT_MS)) throw std::exception("Blocked for too long! Variable can't be set!"); // if invalid after 10 sec, cancel?
#endif
					std::future<T>::get();
					if (next->has_future_task()) next->run_if_not_yet(false);
					got_value_already = true;
				}
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
			inline void Future<T>::wait()
			{
				if (std::future<T>::valid()) {
					*later_value.var = std::future<T>::get(); // just wait in get
					if (next->has_future_task()) next->run_if_not_yet(*later_value.var, false);
					got_value_already = true;
				}
			}

			template<typename T>
			template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
			inline void Future<T>::wait()
			{
				if (std::future<T>::valid()) {
					std::future<T>::get(); // just wait in get
					if (next->has_future_task()) next->run_if_not_yet(false);
					got_value_already = true;
				}
			}

			template<typename T>
			inline bool Future<T>::get_ready(unsigned ms)
			{
				return got_value_already ? true : (std::future<T>::valid() ? std::future<T>::wait_for(std::chrono::milliseconds(ms)) == std::future_status::ready : false);
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_void_v<Q> && !std::is_void_v<T>, int>>
			inline Future<Q> Future<T>::then(std::function<Q(T)> f)
			{
				std::shared_ptr<Promise<Q>> nuxt = std::make_shared<Promise<Q>>();
				Future<Q> yee = nuxt->get_future();

				_set_next([ff = std::move(f), nuu = std::move(nuxt)](T res){
					nuu->set_value(ff(res));
				}, next);
				return yee;
			}

			template<typename T>
			template<typename Q, std::enable_if_t<std::is_void_v<Q> && !std::is_void_v<T>, int>>
			inline Future<Q> Future<T>::then(std::function<void(T)> f)
			{
				std::shared_ptr<Promise<void>> nuxt = std::make_shared<Promise<void>>();
				Future<void> yee = nuxt->get_future();

				_set_next([ff = std::move(f), nuu = std::move(nuxt)](T res){
					ff(res);
					nuu->set_value();
				}, next);
				return yee;
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_void_v<Q>&& std::is_void_v<T>, int>>
			inline Future<Q> Future<T>::then(std::function<Q(T)> f)
			{
				std::shared_ptr<Promise<Q>> nuxt = std::make_shared<Promise<Q>>();
				Future<Q> yee = nuxt->get_future();

				_set_next([ff = std::move(f), nuu = std::move(nuxt)](){
					nuu->set_value(ff());
				}, next);
				return yee;
			}

			template<typename T>
			template<typename Q, std::enable_if_t<std::is_void_v<Q>&& std::is_void_v<T>, int>>
			inline Future<Q> Future<T>::then(std::function<void(T)> f)
			{
				std::shared_ptr<Promise<void>> nuxt = std::make_shared<Promise<void>>();
				Future<void> yee = nuxt->get_future();

				_set_next([ff = std::move(f), nuu = std::move(nuxt)](){
					ff();
					nuu->set_value();
				}, next);
				return yee;
			}

			template<typename T>
			template<typename Q, typename Void, std::enable_if_t<!std::is_void_v<Q>&& std::is_void_v<Void>, int>>
			inline Future<void> Future<T>::then(std::function<void(Void)> f)
			{
				std::shared_ptr<Promise<void>> nuxt = std::make_shared<Promise<void>>();
				Future<void> yee = nuxt->get_future();

				_set_next([ff = std::move(f), nuu = std::move(nuxt)](){
					ff();
					nuu->set_value();
				}, next);
				return yee;
			}

			template<typename T>
			template<typename R, std::enable_if_t<!std::is_void_v<R>, int>>
			inline void Future<T>::_set_next(std::function<void(T)> f, std::shared_ptr<then_block<R>> bfor)
			{
				if (bfor->has_future_task()) bfor->run_if_not_yet(*later_value.var, false);
				next->set(f);
			}

			template<typename T>
			template<typename R, std::enable_if_t<std::is_void_v<R>, int>>
			inline void Future<T>::_set_next(std::function<void(T)> f, std::shared_ptr<then_block<R>> bfor)
			{
				if (bfor->has_future_task()) bfor->run_if_not_yet(false);
				next->set(f);
			}

			template<typename T>
			inline std::shared_ptr<then_block<T>> Future<T>::_get_then()
			{
				return next;
			}

			/* * * * * * * * * * * * * * * * * * > PROMISE<T> < * * * * * * * * * * * * * * * * * */

			template<typename T>
			inline Promise<T>::Promise(Promise&& p)
			{
				got_future = std::move(p.got_future);
				task_to_do = std::move(p.task_to_do);
				then_if = std::move(p.then_if);
				got_future_once = std::move(p.got_future_once);
				set_already_skip = std::move(p.set_already_skip);
				std::promise<T>::operator=(std::move(p));
			}

			template<typename T>
			inline void Promise<T>::operator=(Promise&& p) noexcept
			{
				got_future = std::move(p.got_future);
				task_to_do = std::move(p.task_to_do);
				then_if = std::move(p.then_if);
				got_future_once = std::move(p.got_future_once);
				set_already_skip = std::move(p.set_already_skip);
				std::promise<T>::operator=(std::move(p));
			}

			template<typename T>
			inline Promise<T>::Promise(std::function<T(void)> nf) : std::promise<T>()
			{
				task_to_do = nf;
			}

			template<typename T>
			inline Future<T> Promise<T>::get_future()
			{
				if (!got_future) got_future = true;
#ifndef USE_STD_FULLY
				else throw Handling::Abort(__FUNCSIG__, "For feature reasons, don't create multiple Futures from a single Promise", Handling::abort::abort_level::GIVEUP);
#else
				else throw std::exception("For feature reasons, don't create multiple Futures from a single Promise");
#endif
				Future<T> fut = std::future<T>(std::promise<T>::get_future());
				then_if = fut._get_then();
				return fut;
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
			inline void Promise<T>::work()
			{
				if (task_to_do) {
					try {
						set_value(task_to_do());
					}
					catch (std::exception e) {
						std::promise<T>::set_exception(std::current_exception());
					}
					task_to_do = std::function<T(void)>();
				}
			}

			template<typename T>
			template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
			inline void Promise<T>::work()
			{
				if (task_to_do)
				{
					try {
						task_to_do();
						set_value();
					}
					catch (std::exception e) {
						std::promise<T>::set_exception(std::current_exception());
					}
					task_to_do = std::function<T(void)>();
				}
			}

			template<typename T>
			inline bool Promise<T>::has_set()
			{
				return !set_already_skip;
			}

			template<typename T>
			template<typename Q>
			inline typename std::enable_if_t<std::is_void_v<Q>, bool> Promise<T>::set_value()
			{
				if (!set_already_skip) {
					std::promise<T>::set_value();
					if (then_if) {
						then_if->run_if_not_yet(true);
						then_if.reset();
					}
					set_already_skip = true;
					return true;
				}
				return false;
			}

			template<typename T>
			template<typename Q>
			inline typename std::enable_if_t<!std::is_void_v<Q>, bool> Promise<T>::set_value(const Q& v)
			{
				if (!set_already_skip) {
					std::promise<T>::set_value(v);
					if (then_if) {
						then_if->run_if_not_yet(v, true);
						then_if.reset();
					}
					set_already_skip = true;
					return true;
				}
				return false;
			}

			template<typename T>
			Future<T> fake_future(const T& set)
			{
				Promise<T> pr;
				Future<T> fut = pr.get_future();
				pr.set_value(set);
				return std::move(fut);
			}

		}
	}
}