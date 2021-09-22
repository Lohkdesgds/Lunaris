#pragma once

namespace Lunaris {

	template<typename T>
	inline safe_data<T>::safe_data(const safe_data& oth)
	{
		std::shared_lock<std::shared_mutex> luck1(oth.shrmtx);
		std::unique_lock<std::shared_mutex> luck2(shrmtx);

		data = oth.data;
	}

	template<typename T>
	inline safe_data<T>::safe_data(safe_data&& oth)
	{
		std::unique_lock<std::shared_mutex> luck1(oth.shrmtx, std::defer_lock);
		std::unique_lock<std::shared_mutex> luck2(shrmtx, std::defer_lock);
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
		std::unique_lock<std::shared_mutex> luck2(shrmtx);
		data = std::move(var);
	}
	
	template<typename T>
	inline T safe_data<T>::read()
	{
		std::shared_lock<std::shared_mutex> luck(shrmtx);
		return data;
	}

	template<typename T>
	inline void safe_data<T>::set(T&& var)
	{
		std::unique_lock<std::shared_mutex> luck(shrmtx);
		data = std::move(var);
	}

	template<typename T>
	inline void safe_data<T>::set(const T& var)
	{
		std::unique_lock<std::shared_mutex> luck(shrmtx);
		data = var;
	}

	template<typename T>
	inline void safe_data<T>::reset()
	{
		std::unique_lock<std::shared_mutex> luck(shrmtx);
		data = {};
	}

	template<typename T>
	inline safe_data<T>::operator T()
	{
		std::shared_lock<std::shared_mutex> luck(shrmtx);
		return data;
	}
	template<typename T>
	inline void safe_data<T>::operator=(const safe_data& oth)
	{
		std::shared_lock<std::shared_mutex> luck1(oth.shrmtx);
		std::unique_lock<std::shared_mutex> luck2(shrmtx);

		data = oth.data;
	}

	template<typename T>
	inline void safe_data<T>::operator=(safe_data&& oth)
	{
		std::unique_lock<std::shared_mutex> luck1(oth.shrmtx, std::defer_lock);
		std::unique_lock<std::shared_mutex> luck2(shrmtx, std::defer_lock);
		std::lock(luck1, luck2);

		data = std::move(oth.data);
	}

	template<typename T>
	inline void safe_data<T>::operator=(T&& var)
	{
		std::unique_lock<std::shared_mutex> luck(shrmtx);
		data = std::move(var);
	}

	template<typename T>
	inline void safe_data<T>::operator=(const T& var)
	{
		std::unique_lock<std::shared_mutex> luck(shrmtx);
		data = var;
	}

}