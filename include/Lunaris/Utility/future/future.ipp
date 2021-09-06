#pragma once

namespace Lunaris {

	template<typename T>
	template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
	void future<T>::_self::_int_data::handle_start()
	{
		value = new T();
	}

	template<typename T>
	template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
	void future<T>::_self::_int_data::handle_end()
	{
		if (value) {
			delete value;
			value = nullptr;
		}
	}

	template<typename T>
	template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
	void future<T>::_self::_int_data::handle_start()
	{
	}

	template<typename T>
	template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
	void future<T>::_self::_int_data::handle_end()
	{
	}

	template<typename T>
	future<T>::_self::_int_data::_int_data()
	{
		handle_start();
	}

	template<typename T>
	future<T>::_self::_int_data::~_int_data()
	{
		handle_end();
	}

	template<typename T>
	template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
	T* future<T>::_self::_int_data::get_autowait()
	{
		if (has_result) return value;
		std::unique_lock<std::mutex> safe(triggered_mutex);
		if (!has_result) {
			while (!has_result) triggered_result.wait_for(safe, std::chrono::milliseconds(100), [&] { return has_result; });
		}
		return value;
	}

	template<typename T>
	template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
	T* future<T>::_self::_int_data::get_autowait()
	{
		if (has_result) return nullptr;
		std::unique_lock<std::mutex> safe(triggered_mutex);
		if (!has_result) {
			while (!has_result) triggered_result.wait_for(safe, std::chrono::milliseconds(100), [&] { return has_result; });
		}
		return nullptr;
	}

	template<typename T>
	template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
	void future<T>::_self::post(T t)
	{
		if (_data.has_result) throw std::runtime_error("Value was already set!");
		{
			std::unique_lock<std::mutex> safe(_data.triggered_mutex);
			if (redirect) _data._next(t);
			else *_data.value = t;
			_data.has_result = true;
			//std::cout << " __ post posted val\n";
		}

		_data.triggered_result.notify_one();
	}

	template<typename T>
	template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
	void future<T>::_self::post()
	{
		if (_data.has_result) throw std::runtime_error("Value was already set!");

		{
			std::unique_lock<std::mutex> safe(_data.triggered_mutex);
			if (redirect) _data._next();
			_data.has_result = true;
			//std::cout << " __ post posted val\n";
		}

		_data.triggered_result.notify_one();
	}

	template<typename T>
	future<T>::future(future&& oth)
		: m_data(std::move(oth.m_data))
	{
	}

	template<typename T>
	void future<T>::operator=(future&& oth)
	{
		m_data = std::move(oth.m_data);
	}

	template<typename T>
	template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
	const T* future<T>::get()
	{
		if (m_data->redirect)
			throw std::runtime_error("Fatal error: you should not get() a future set to then()!");

		return m_data->_data.get_autowait();
	}

	template<typename T>
	template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
	void future<T>::get()
	{
		m_data->_data.get_autowait();
		return;
	}

	template<typename T>
	inline void future<T>::wait()
	{
		m_data->_data.get_autowait();
	}

	template<typename T>
	template<typename V, typename Q, std::enable_if_t<!std::is_void_v<Q>, int>, typename Res, std::enable_if_t<!std::is_void_v<Res>, int>> // Res == V() result type, Q = self result type
	inline auto future<T>::then(V handler)
	{
		future<decltype(handler(T()))> next;

		m_data->redirect = true;
		m_data->_data._next = [handler, _next_obj = next.m_data](const T& variable)
		{
			_next_obj->post(handler(variable));
		};

		return next;
	}

	template<typename T>
	template<typename V, typename Q, std::enable_if_t<std::is_void_v<Q>, int>, typename Res, std::enable_if_t<!std::is_void_v<Res>, int>> // Res == V() result type, Q = self result type
	inline auto future<T>::then(V handler)
	{
		future<decltype(handler())> next;

		m_data->redirect = true;
		m_data->_data._next = [handler, _next_obj = next.m_data]()
		{
			_next_obj->post(handler());
		};

		return next;
	}

	template<typename T>
	template<typename V, typename Q, std::enable_if_t<!std::is_void_v<Q>, int>, typename Res, std::enable_if_t<std::is_void_v<Res>, int>> // Res == V() result type, Q = self result type
	inline auto future<T>::then(V handler)
	{
		future<decltype(handler(T()))> next;

		m_data->redirect = true;
		m_data->_data._next = [handler, _next_obj = next.m_data](const T& variable)
		{
			handler(variable);
			_next_obj->post();
		};

		return next;
	}

	template<typename T>
	template<typename V, typename Q, std::enable_if_t<std::is_void_v<Q>, int>, typename Res, std::enable_if_t<std::is_void_v<Res>, int>> // Res == V() result type, Q = self result type
	inline auto future<T>::then(V handler)
	{
		future<decltype(handler())> next;

		m_data->redirect = true;
		m_data->_data._next = [handler, _next_obj = next.m_data]()
		{
			handler();
			_next_obj->post();
		};

		return next;
	}

	template<typename T>
	promise<T>::promise(promise&& oth)
	{
		*this = std::move(oth);
	}

	template<typename T>
	void promise<T>::operator=(promise&& oth)
	{
		_next = std::move(oth._next);
	}

	template<typename T>
	inline future<T> promise<T>::get_future()
	{
		if (_next) throw std::runtime_error("You must not create two future of same promise.");
		future<T> next;
		_next = [_next_obj = next.m_data](const T& variable)
		{
			_next_obj->post(variable);
		};
		return next;
	}

	template<typename T>
	template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
	void promise<T>::set_value(const T& val)
	{
		if (_next) _next(val);
	}

	template<typename T>
	template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
	void promise<T>::set_value()
	{
		if (_next) _next();
	}

}