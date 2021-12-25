#pragma once

namespace Lunaris {

	template<typename T>
	inline tie<T>::tie(const bool newd)
	{
		if (newd) _ptr = std::make_shared<T>();
	}

	template<typename T>
	inline tie<T>::tie(const tie& oth)
		: _ptr(oth._ptr)
	{
	}

	template<typename T>
	inline tie<T>::tie(tie&& oth)
		: _ptr(std::move(oth._ptr))
	{
	}

	template<typename T>
	inline tie<T>::tie(const T& var)
		: _ptr(new T(var))
	{
	}

	template<typename T>
	inline tie<T>::tie(T&& var)
		: _ptr(new T(std::move(var)))
	{
	}

	template<typename T>
	inline const tie<T>& tie<T>::operator=(const tie& oth)
	{
		_ptr = oth._ptr;
		return *this;
	}

	template<typename T>
	inline void tie<T>::operator=(tie&& oth)
	{
		_ptr = std::move(oth._ptr);
	}

	template<typename T>
	inline const T& tie<T>::operator=(const T& var)
	{
		if (!_ptr) _ptr = std::shared_ptr<T>(new T(var));
		else *_ptr = var;
	}

	template<typename T>
	inline void tie<T>::operator=(T&& var)
	{
		if (!_ptr) _ptr = std::shared_ptr<T>(new T(std::move(var)));
		else *_ptr = std::move(var);
	}

	template<typename T>
	inline const T* tie<T>::operator->() const
	{
		return _ptr.get();
	}

	template<typename T>
	inline T* tie<T>::operator->()
	{
		return _ptr.get();
	}

	template<typename T>
	inline const T& tie<T>::operator*() const
	{
		return *_ptr;
	}

	template<typename T>
	inline T& tie<T>::operator*()
	{
		return *_ptr;
	}

	template<typename T>
	inline bool tie<T>::operator==(const T& v) const
	{
		return _ptr ? *_ptr == v : false;
	}

	template<typename T>
	inline bool tie<T>::operator==(const tie& oth) const
	{
		return _ptr.get() == oth._ptr.get();
	}

	template<typename T>
	inline bool tie<T>::operator!=(const T& v) const
	{
		return _ptr ? *_ptr != v : true;
	}

	template<typename T>
	inline bool tie<T>::operator!=(const tie& oth) const
	{
		return _ptr.get() != oth._ptr.get();
	}

	template<typename T>
	inline std::optional<T> tie<T>::unref()
	{
		if (_ptr) {
			T mov_onc = std::move(*_ptr.get());
			_ptr = std::make_shared<T>();
			return mov_onc;
		}
		return {};
	}

	template<typename T>
	inline std::optional<T> tie<T>::ref(T& ref)
	{
		if (_ptr) {
			T mov_onc = std::move(*_ptr.get());
			_ptr = std::shared_ptr<T>((T*)&ref, [](T*) {}); // shared_ptr with no destructor, ofc
			return mov_onc;
		}
		else {
			_ptr = std::shared_ptr<T>((T*)&ref, [](T*) {}); // shared_ptr with no destructor, ofc
			return {};
		}
	}

	template<typename T>
	inline void tie<T>::swap(tie&& oth)
	{
		auto _ths = std::move(_ptr);
		_ptr = std::move(oth._ptr);
		oth._ptr = std::move(_ths);
	}

	template<typename T>
	inline bool tie<T>::valid() const
	{
		return _ptr.get() != nullptr;
	}

	template<typename T>
	inline bool tie<T>::empty() const
	{
		return _ptr.get() == nullptr;
	}

}