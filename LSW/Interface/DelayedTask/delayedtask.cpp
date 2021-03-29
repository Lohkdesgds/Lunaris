#include "delayedtask.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			DelayedTask::_task::_task(const std::function<void(void)>& f)
				: what(f)
			{
			}

			void DelayedTask::add_abort(const Handling::Abort& ab)
			{
				Tools::AutoLock l(abort_mu);
				if (aborts.size() < delayedtask::maximum_aborts_saved) aborts.push_back(ab);
			}

			void DelayedTask::run_solo_autoabort(const std::function<void(void)>& f)
			{
				try {
					f();
				}
				catch (const Handling::Abort& e)
				{
					add_abort(e);
				}
				catch (const std::exception& e)
				{
					Handling::Abort a("unknown", e.what(), Handling::abort::abort_level::GIVEUP);
					add_abort(a);
				}
				catch (...)
				{
					Handling::Abort a(__FUNCSIG__, "Unhandled exception! Couldn't get it as Handling::Abort.", Handling::abort::abort_level::GIVEUP);
					add_abort(a);
				}
			}

			void DelayedTask::queue_task(Tools::boolThreadF run)
			{
				while (run())
				{
					Tools::AutoLock l(mu, false);
					if (l.try_lock())
					{
						// cleanup solo threads
						for (size_t p = 0; p < solo_delayed.size(); p++) {
							const auto& i = solo_delayed[p];
							if (!i.running()) {
								solo_delayed.erase(solo_delayed.begin() + p--);
							}
						}

						// queued tasks
						const auto now = std::chrono::system_clock::now();

						for (size_t p = 0; p < single_queue_queue.size(); p++)
						{
							auto& i = single_queue_queue[p];
							if (now >= i.when) {
								if (i.what.has_set()) run_solo_autoabort([&] { i.what.work(); });
								single_queue_queue.erase(single_queue_queue.begin() + p--);
							}
						}
						
					}
				}
			}

			DelayedTask::DelayedTask()
			{
				single_queue.set([&](Tools::boolThreadF f) {queue_task(f); });
				single_queue.start();
			}

			DelayedTask::~DelayedTask()
			{
				single_queue.stop();
				Tools::AutoLock l(mu);
				for (auto& i : single_queue_queue) if (!i.what.has_set()) run_solo_autoabort([&] { i.what.work(); });
				for (auto& i : solo_delayed) i.stop();
			}

			Tools::Future<void> DelayedTask::push_back(const std::function<void(void)> f, const std::chrono::milliseconds dt, const delayedtask::delayed_type delaytype)
			{
				if (!f) throw Handling::Abort(__FUNCSIG__, "Invalid function!", Handling::abort::abort_level::GIVEUP);

				Tools::AutoLock l(mu);
				const auto timee = std::chrono::system_clock::now() + dt;

				switch (delaytype) {
				case delayedtask::delayed_type::QUEUE:
				{
					_task task(f);
					task.when = timee;
					Tools::Future<void> fut = task.what.get_future();
					single_queue_queue.push_back(std::move(task));
					return std::move(fut);
				}
				case delayedtask::delayed_type::SPAWN_SOLO:
				{
					solo_delayed.push_back({});
					Tools::SuperThread<>& thr = solo_delayed.back();
					thr.set([&, func = f, timee](Tools::boolThreadF){ std::this_thread::sleep_until(timee); run_solo_autoabort(func); });
					Tools::Future<void> fut = thr.start();
					return std::move(fut);
				}
				}
				return Tools::fake_future<void>();
			}

			bool DelayedTask::had_abort() const
			{
				return aborts.size();
			}

			Handling::Abort DelayedTask::pop_last_abort()
			{
				Tools::AutoLock l(abort_mu);
				if (aborts.size() == 0) return Handling::Abort("", "", Handling::abort::abort_level::OTHER);
				Handling::Abort ab = aborts.front();
				aborts.erase(aborts.begin());
				return std::move(ab);
			}

			size_t DelayedTask::size() const
			{
				return single_queue_queue.size() + solo_delayed.size();
			}

			size_t DelayedTask::threads_queued() const
			{
				return solo_delayed.size();
			}

		}
	}
}