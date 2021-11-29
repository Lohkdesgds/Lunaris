#include "safe_data.h"
#pragma once

namespace Lunaris {

	template<typename T>
	inline safe_data<T>::safe_data(const safe_data& oth)
	{
		std::shared_lock<shared_recursive_mutex> luck1(oth.shrmtx);
		std::unique_lock<shared_recursive_mutex> luck2(shrmtx);

		data = oth.data;
	}

	template<typename T>
	inline safe_data<T>::safe_data(safe_data&& oth)
	{
		std::unique_lock<shared_recursive_mutex> luck1(oth.shrmtx, std::defer_lock);
		std::unique_lock<shared_recursive_mutex> luck2(shrmtx, std::defer_lock);
		std::lock(luck1, luck2);

		data = std::move(oth.data);
	}

	template<typename T>
	inline safe_data<T>::safe_data(const T& var)
	{
		set(var); // should lock things automatically
	}

	template<typename T>
	inline safe_data<T>::safe_data(T&& var)
	{
		std::unique_lock<shared_recursive_mutex> luck2(shrmtx);
		data = std::move(var);
	}
	
	template<typename T>
	inline T safe_data<T>::read() const
	{
		std::shared_lock<shared_recursive_mutex> luck(shrmtx);
		return data;
	}

	template<typename T>
	inline void safe_data<T>::csafe(const std::function<void(const T&)> f) const
	{
		if (!f) return;
		std::shared_lock<shared_recursive_mutex> luck(shrmtx);
		f(data);
	}

	template<typename T>
	inline void safe_data<T>::safe(const std::function<void(T&)> f)
	{
		if (!f) return;
		std::unique_lock<shared_recursive_mutex> luck(shrmtx);
		f(data);
	}

	template<typename T>
	inline void safe_data<T>::set(const T& var)
	{
		std::unique_lock<shared_recursive_mutex> luck(shrmtx);
		data = var;
	}

	template<typename T>
	inline void safe_data<T>::set(T&& var)
	{
		std::unique_lock<shared_recursive_mutex> luck(shrmtx);
		data = std::move(var);
	}

	template<typename T>
	inline T safe_data<T>::reset()
	{
		std::unique_lock<shared_recursive_mutex> luck(shrmtx);
		T movv = std::move(data);
		data = {}; // be sure
		return movv;
	}

	template<typename T>
	inline safe_data<T>::operator T() const
	{
		std::shared_lock<shared_recursive_mutex> luck(shrmtx);
		return data;
	}
	template<typename T>
	inline void safe_data<T>::operator=(const safe_data& oth)
	{
		std::shared_lock<shared_recursive_mutex> luck1(oth.shrmtx);
		std::unique_lock<shared_recursive_mutex> luck2(shrmtx);

		data = oth.data;
	}

	template<typename T>
	inline void safe_data<T>::operator=(safe_data&& oth)
	{
		std::unique_lock<shared_recursive_mutex> luck1(oth.shrmtx, std::defer_lock);
		std::unique_lock<shared_recursive_mutex> luck2(shrmtx, std::defer_lock);
		std::lock(luck1, luck2);

		data = std::move(oth.data);
	}

	template<typename T>
	inline void safe_data<T>::operator=(const T& var)
	{
		std::unique_lock<shared_recursive_mutex> luck(shrmtx);
		data = var;
	}

	template<typename T>
	inline void safe_data<T>::operator=(T&& var)
	{
		std::unique_lock<shared_recursive_mutex> luck(shrmtx);
		data = std::move(var);
	}

	template<typename T>
	inline safe_vector<T>::safe_vector(const safe_vector& var)
	{
		std::unique_lock<shared_recursive_mutex> luck1(shrmtx, std::defer_lock);
		std::unique_lock<shared_recursive_mutex> luck2(var.shrmtx, std::defer_lock);
		std::lock(luck1, luck2);

		data = var.data;
	}

	template<typename T>
	inline safe_vector<T>::safe_vector(safe_vector&& var)
	{
		std::unique_lock<shared_recursive_mutex> luck1(shrmtx, std::defer_lock);
		std::unique_lock<shared_recursive_mutex> luck2(var.shrmtx, std::defer_lock);
		std::lock(luck1, luck2);

		data = std::move(var.data);
	}

	template<typename T>
	inline void safe_vector<T>::operator=(const safe_vector& var)
	{
		std::unique_lock<shared_recursive_mutex> luck1(shrmtx, std::defer_lock);
		std::unique_lock<shared_recursive_mutex> luck2(var.shrmtx, std::defer_lock);
		std::lock(luck1, luck2);

		data = var.data;
	}

	template<typename T>
	inline void safe_vector<T>::operator=(safe_vector&& var)
	{
		std::unique_lock<shared_recursive_mutex> luck1(shrmtx, std::defer_lock);
		std::unique_lock<shared_recursive_mutex> luck2(var.shrmtx, std::defer_lock);
		std::lock(luck1, luck2);

		data = std::move(var.data);
	}

	template<typename T>
	inline void safe_vector<T>::push_back(const T& var)
	{
		std::unique_lock<shared_recursive_mutex> luck1(shrmtx);
		data.push_back(var);
	}

	template<typename T>
	inline void safe_vector<T>::push_back(T&& var)
	{
		std::unique_lock<shared_recursive_mutex> luck1(shrmtx);
		data.push_back(std::move(var));
	}

	template<typename T>
	inline T safe_vector<T>::index(const size_t& i) const
	{
		std::shared_lock<shared_recursive_mutex> luck1(shrmtx);
		if (i >= data.size()) throw std::out_of_range("index was beyond save_vector's size");
		return data[i];
	}

	template<typename T>
	inline T safe_vector<T>::operator[](const size_t i) const
	{
		std::shared_lock<shared_recursive_mutex> luck1(shrmtx);
		if (i >= data.size()) throw std::out_of_range("index was beyond save_vector's size");
		return data[i];
	}

	template<typename T>
	inline bool safe_vector<T>::set(const size_t i, const T& var, const bool forc)
	{
		std::unique_lock<shared_recursive_mutex> luck1(shrmtx);
		if (i >= data.size()) {
			if (forc) {
				try {
					data.resize(i + 1); // when i == 0, size is 1, and so on...
				}
				catch (...) { return false; } // failure
				data[i] = var;
				return true;
			}
			return false;
		}
		data[i] = var;
		return true;
	}

	template<typename T>
	inline bool safe_vector<T>::set(const size_t i, T&& var, const bool forc)
	{
		std::unique_lock<shared_recursive_mutex> luck1(shrmtx);
		if (i >= data.size()) {
			if (forc) {
				try {
					data.resize(i + 1); // when i == 0, size is 1, and so on...
				}
				catch (...) { return false; } // failure
				data[i] = std::move(var);
				return true;
			}
			return false;
		}
		data[i] = std::move(var);
		return true;
	}

	template<typename T>
	inline void safe_vector<T>::csafe(const std::function<void(const std::vector<T>&)> f)
	{
		if (!f) return;
		std::shared_lock<shared_recursive_mutex> luck1(shrmtx);
		f(data);
	}

	template<typename T>
	inline void safe_vector<T>::safe(const std::function<void(std::vector<T>&)> f)
	{
		if (!f) return;
		std::unique_lock<shared_recursive_mutex> luck1(shrmtx);
		f(data);
	}

	template<typename T>
	inline void safe_vector<T>::erase(const size_t& var)
	{
		std::unique_lock<shared_recursive_mutex> luck1(shrmtx);
		if (var >= data.size()) return;
		data.erase(data.begin() + var);
	}

	template<typename T>
	inline void safe_vector<T>::erase(const size_t& var, const size_t& var2)
	{
		std::unique_lock<shared_recursive_mutex> luck1(shrmtx);
		if (var >= data.size()) return;
		data.erase(data.begin() + var, (var2 >= data.size()) ? data.end() : (data.begin() + var2));
	}

	template<typename T>
	inline void safe_vector<T>::clear()
	{
		std::unique_lock<shared_recursive_mutex> luck1(shrmtx);
		data.clear();
	}

	template<typename T>
	inline size_t safe_vector<T>::size() const
	{
		return data.size();
	}

}