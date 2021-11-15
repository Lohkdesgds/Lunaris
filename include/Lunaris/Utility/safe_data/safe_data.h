#pragma once

#include <Lunaris/__macro/macros.h>

#include <Lunaris/Utility/mutex.h>

#include <shared_mutex>
#include <functional>

namespace Lunaris {

	template<typename T>
	class safe_data {
		mutable shared_recursive_mutex shrmtx;
		T data;
	public:
		safe_data() = default;
		safe_data(const safe_data&);
		safe_data(safe_data&&);
		safe_data(const T&);
		safe_data(T&&);

		T read();

		void csafe(const std::function<void(const T&)>);
		void safe(const std::function<void(T&)>);

		void set(T&&);
		void set(const T&);
		void reset();

		operator T();
		void operator=(const safe_data&);
		void operator=(safe_data&&);
		void operator=(T&&);
		void operator=(const T&);
	};

	template<typename T>
	class safe_vector {
		mutable shared_recursive_mutex shrmtx;
		std::vector<T> data;
	public:
		safe_vector() = default;
		safe_vector(const safe_vector&);
		safe_vector(safe_vector&&);
		void operator=(const safe_vector&);
		void operator=(safe_vector&&);

		void push_back(T&&);
		const T& index(const size_t&) const;
		T& index(const size_t&);

		void csafe(const std::function<void(const std::vector<T>&)>);
		void safe(const std::function<void(std::vector<T>&)>);

		void erase(const size_t&);
		void erase(const size_t&, const size_t&);
		void clear();

		size_t size() const;
	};
}

#include "safe_data.ipp"