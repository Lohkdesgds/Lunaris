#include "package.h"
#pragma once

namespace Lunaris {

	inline package::package(const package& pack)
		: buf(pack.buf), read_index_pos(pack.read_index_pos)
	{
	}

	inline package::package(package&& pack) noexcept
		: buf(std::move(pack.buf)), read_index_pos(pack.read_index_pos)
	{
		pack.read_index_pos = 0;
	}

	inline package::package(const std::vector<char>& pack)
		: buf(pack), read_index_pos(0)
	{
	}

	inline package::package(std::vector<char>&& pack) noexcept
		: buf(std::move(pack)), read_index_pos(0)
	{
	}

	inline void package::operator=(const package& pack)
	{
		buf = pack.buf;
		read_index_pos = pack.read_index_pos;
	}

	inline void package::operator=(package&& pack) noexcept
	{
		buf = std::move(pack.buf);
		read_index_pos = pack.read_index_pos;
		pack.read_index_pos = 0;
	}

	inline void package::operator=(const std::vector<char>& pack)
	{
		buf = pack;
		read_index_pos = 0;
	}

	inline void package::operator=(std::vector<char>&& pack) noexcept
	{
		buf = std::move(pack);
		read_index_pos = 0;
	}

	inline bool package::operator==(const package& oth) const 
	{
		return oth.buf == buf;
	}

	inline bool package::operator!=(const package& oth) const 
	{
		return oth.buf != buf;
	}

	template<typename T, std::enable_if_t<std::is_pod_v<T> && !std::is_array_v<T> && !std::is_pointer_v<T>, int>>
	inline package& package::operator>>(T& val)
	{
		if (read_index_pos + sizeof(val) > buf.size()) throw std::runtime_error("Out of range");
		auto pos = buf.begin() + read_index_pos;
		std::copy(pos, pos + sizeof(val), (char*)&val);
		read_index_pos += sizeof(val);
		return *this;
	}

	inline package& package::operator>>(std::string& val)
	{
		measure_siz sizer = 0;
		*this >> sizer;
		if (sizer == 0 || read_index_pos + sizer > buf.size()) throw std::runtime_error("Unexpected invalid value while reading");
		auto pos = buf.begin() + read_index_pos;
		val.resize(sizer);
		std::copy(pos, pos + sizer, val.begin());
		read_index_pos += sizer;
		return *this;
	}

	inline package& package::operator>>(std::vector<char>& val)
	{
		measure_siz sizer = 0;
		*this >> sizer;
		if (sizer == 0 || read_index_pos + sizer > buf.size()) throw std::runtime_error("Unexpected invalid value while reading");
		auto pos = buf.begin() + read_index_pos;
		val.resize(sizer);
		std::copy(pos, pos + sizer, val.begin());
		read_index_pos += sizer;
		return *this;
	}


	template<typename T, std::enable_if_t<std::is_pod_v<T> && !std::is_array_v<T> && !std::is_pointer_v<T>, int>>
	inline package& package::operator<<(const T& val)
	{
		buf.resize(buf.size() + sizeof(val));
		auto pos = buf.end() - sizeof(val);
		std::copy((char*)&val, (char*)&val + sizeof(val), pos);
		reset_internal_iterator();
		return *this;
	}

	inline package& package::operator<<(const std::string& val)
	{
		if (val.empty()) throw std::runtime_error("Invalid data to write: it was null!");
		measure_siz sizer = val.size();
		*this << sizer;
		for (const auto& i : val) buf.push_back(i);
		reset_internal_iterator();
		return *this;
	}

	inline package& package::operator<<(const std::vector<char>& val)
	{
		if (val.empty()) throw std::runtime_error("Invalid data to write: it was null!");
		measure_siz sizer = val.size();
		*this << sizer;
		for (const auto& i : val) buf.push_back(i);
		reset_internal_iterator();
		return *this;
	}

	inline void package::reset_internal_iterator()
	{
		read_index_pos = 0;
	}

	inline const std::vector<char>& package::read_as_array() const
	{
		return buf;
	}

	inline size_t package::read_as_data(char* ptr, const size_t max, const size_t bgg) const
	{
		size_t p = bgg;
		for (; p < max && p < buf.size(); ++p) {
			ptr[p] = buf[p];
		}
		return p;
	}

	inline void package::import_as_array(std::vector<char>&& oth)
	{
		buf = std::move(oth);
	}

	inline void package::import_as_data(const char* ptr, const size_t len)
	{
		for (const char* be = ptr; be < (ptr + len); ++be) buf.push_back(*be);
	}

	inline package::operator const std::vector<char>&() const
	{
		return buf;
	}

	inline size_t package::size() const
	{
		return buf.size();
	}

	inline bool package::valid() const
	{
		return buf.size() != 0;
	}

	inline bool package::empty() const
	{
		return buf.empty();
	}

}