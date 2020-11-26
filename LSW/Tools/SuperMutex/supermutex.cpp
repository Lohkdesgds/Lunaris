#include "supermutex.h"

namespace LSW {
	namespace v5 {
		namespace Tools {

			bool SuperMutex::weird_mtx::try_lock()
			{
				if (locked) return false;
				m.lock();
				locked = true;
				m.unlock();
				return true;
			}

			void SuperMutex::weird_mtx::unlock()
			{
				m.lock();
				locked = false;
				m.unlock();
			}
			
			bool SuperMutex::weird_mtx::is_locked() const
			{
				return locked;
			}

			void SuperMutex::lock()
			{
				std::mutex defu;
				if (!mu.try_lock()) {
					std::unique_lock<std::mutex> ul(defu);
					do {
						cond.wait_for(ul, std::chrono::milliseconds(100));
					} while (!mu.try_lock());
				}
			}
			
			void SuperMutex::unlock()
			{
				if (mu.is_locked()) {
					mu.unlock();
					cond.notify_one();
					std::this_thread::sleep_for(std::chrono::milliseconds(2));
				}
			}

			bool SuperMutex::is_locked() const
			{
				return mu.is_locked();
			}

			AutoLock::AutoLock(SuperMutex& m, const bool autolock)
				: you(m)
			{
				if (autolock) {
					you.lock();
					hasunlocked = false;
				}
				else hasunlocked = true;
			}
			
			AutoLock::~AutoLock()
			{
				if (!hasunlocked) you.unlock();
			}
			
			void AutoLock::unlock()
			{
				if (!hasunlocked) {
					hasunlocked = true;
					you.unlock();
				}
			}
			
			void AutoLock::lock()
			{
				if (hasunlocked) {
					hasunlocked = false;
					you.lock();
				}
			}

			void Waiter::wait_signal(const size_t max_t)
			{
				std::mutex defu;
				std::unique_lock<std::mutex> ul(defu);
				if (max_t == 0) cond.wait(ul);
				else cond.wait_for(ul, std::chrono::milliseconds(max_t));
			}
			
			void Waiter::signal_one()
			{
				cond.notify_one();
			}
			
			void Waiter::signal_all()
			{
				cond.notify_all();
			}

		}
	}
}