#include "memory.h"
#pragma once

namespace Lunaris {

	template<typename T>
	inline hybrid_memory<T>::hybrid_memory(const hybrid_memory& oth)
		: m_ptr(oth.m_ptr)
	{
	}

	template<typename T>
	inline hybrid_memory<T>::hybrid_memory(hybrid_memory&& oth) noexcept
		: m_ptr(std::move(oth.m_ptr))
	{
	}

	template<typename T>
	inline void hybrid_memory<T>::operator=(const hybrid_memory& oth)
	{
		m_ptr = oth.m_ptr;
	}

	template<typename T>
	inline void hybrid_memory<T>::operator=(hybrid_memory&& oth) noexcept
	{
		m_ptr = std::move(oth.m_ptr);
	}

	template<typename T>
	inline bool hybrid_memory<T>::operator==(const hybrid_memory& oth) const
	{
		return oth.m_ptr == m_ptr;
	}

	template<typename T>
	inline bool hybrid_memory<T>::operator!=(const hybrid_memory& oth) const
	{
		return oth.m_ptr != m_ptr;
	}

	template<typename T>
	inline const T* hybrid_memory<T>::get() const
	{
		return m_ptr->get();
	}

	template<typename T>
	inline T* hybrid_memory<T>::get()
	{
		return m_ptr->get();
	}

	template<typename T>
	inline const T* hybrid_memory<T>::operator->() const
	{
		return m_ptr->get();
	}

	template<typename T>
	inline T* hybrid_memory<T>::operator->()
	{
		return m_ptr->get();
	}

	template<typename T>
	inline const T& hybrid_memory<T>::operator*() const
	{
		return *get();
	}

	template<typename T>
	inline T& hybrid_memory<T>::operator*()
	{
		return *get();
	}

	template<typename T>
	inline bool hybrid_memory<T>::valid() const
	{
		return m_ptr && get() != nullptr;
	}

	template<typename T>
	inline bool hybrid_memory<T>::empty() const
	{
		return !m_ptr || get() == nullptr;
	}

	template<typename T>
	inline void hybrid_memory<T>::reset_this()
	{
		m_ptr.reset();
	}

	template<typename T>
	inline std::unique_ptr<T> hybrid_memory<T>::reset_shared()
	{
		if (!m_ptr) throw std::runtime_error("Reset all can't be performed on null hybrid!");
		return std::move(*m_ptr);
	}

	template<typename T>
	inline void hybrid_memory<T>::replace_this(T&& oth)
	{
		m_ptr = std::make_shared<std::unique_ptr<T>>(std::make_unique<T>(std::move(oth)));
	}

	template<typename T>
	inline void hybrid_memory<T>::replace_shared(T&& oth)
	{
		if (!m_ptr) m_ptr = std::make_shared<std::unique_ptr<T>>(std::make_unique<T>(std::move(oth)));
		else	    *m_ptr = std::make_unique<T>(std::move(oth));
	}

	template<typename T>
	inline void hybrid_memory<T>::replace_this(std::unique_ptr<T>&& oth)
	{
		m_ptr = std::make_shared<std::unique_ptr<T>>(std::move(oth));
	}

	template<typename T>
	inline void hybrid_memory<T>::replace_shared(std::unique_ptr<T>&& oth)
	{
		if (!m_ptr) m_ptr = std::make_shared<std::unique_ptr<T>>(std::move(oth));
		else	    *m_ptr = std::move(oth);
	}

	template<typename T>
	inline void hybrid_memory<T>::replace_this(const hybrid_memory& oth)
	{
		m_ptr = oth.m_ptr;
	}

	template<typename T>
	inline size_t hybrid_memory<T>::use_count() const
	{
		return m_ptr.use_count();
	}

	template<typename T>
	inline hybrid_memory<T> make_hybrid()
	{
		hybrid_memory<T> hybrid;
		hybrid.replace_this(std::make_unique<T>());
		return hybrid;
	}

	template<typename T, typename K>
	hybrid_memory<T> make_hybrid_derived()
	{
		hybrid_memory<T> hybrid;
		hybrid.replace_this(std::make_unique<K>());
		return hybrid;
	}

	template<typename T, typename... Args>
	inline hybrid_memory<T> make_hybrid(Args&&... args)
	{
		hybrid_memory<T> hybrid;
		hybrid.replace_this(std::make_unique<T>(std::move(args...)));
		return hybrid;
	}

	template<typename T, typename K, typename ...Args>
	hybrid_memory<T> make_hybrid_derived(Args&&... args)
	{
		hybrid_memory<T> hybrid;
		hybrid.replace_this(std::make_unique<K>(std::move(args...)));
		return hybrid;
	}

}