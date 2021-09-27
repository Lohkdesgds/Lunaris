#pragma once

#include <thread>
#include <functional>
#include <memory>

namespace Lunaris {

	class package {
		using measure_siz = size_t;
		std::vector<char> buf;
		size_t read_index_pos = 0;
	public:
		package() = default;
		
		package(const package&);
		package(package&&) noexcept;
		package(const std::vector<char>&);
		package(std::vector<char>&&) noexcept;

		void operator=(const package&);
		void operator=(package&&) noexcept;
		void operator=(const std::vector<char>&);
		void operator=(std::vector<char>&&) noexcept;

		bool operator==(const package&) const;

		template<typename T, std::enable_if_t<std::is_pod_v<T> && !std::is_array_v<T> && !std::is_pointer_v<T>, int> = 0>
		package& operator>>(T&);
		package& operator>>(std::string&);
		package& operator>>(std::vector<char>&);

		template<typename T, std::enable_if_t<std::is_pod_v<T> && !std::is_array_v<T> && !std::is_pointer_v<T>, int> = 0>
		package& operator<<(const T&);
		package& operator<<(const std::string&);
		package& operator<<(const std::vector<char>&);

		void reset_internal_iterator();

		const std::vector<char>& read_as_array() const;
		void import_as_array(std::vector<char>&&);

		operator const std::vector<char>&() const;
	};
}

#include "package.ipp"