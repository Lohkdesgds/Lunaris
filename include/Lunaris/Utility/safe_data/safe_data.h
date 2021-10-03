#pragma once

#include <Lunaris/__macro/macros.h>

#include <shared_mutex>

namespace Lunaris {

	template<typename T>
	class safe_data {
		mutable std::shared_mutex shrmtx;
		T data;
	public:
		safe_data() = default;
		safe_data(const safe_data&);
		safe_data(safe_data&&);
		safe_data(const T&);
		safe_data(T&&);

		T read();
		void set(T&&);
		void set(const T&);
		void reset();

		operator T();
		void operator=(const safe_data&);
		void operator=(safe_data&&);
		void operator=(T&&);
		void operator=(const T&);
	};
}

#include "safe_data.ipp"