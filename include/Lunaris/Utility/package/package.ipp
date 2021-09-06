#pragma once

namespace Lunaris {

	template<typename T, std::enable_if_t<std::is_pod_v<T> && !std::is_array_v<T> && !std::is_pointer_v<T>, int>>
	package& package::operator>>(T& val)
	{
		if (read_index_pos + sizeof(val) > buf.size()) throw std::runtime_error("Out of range");
		auto pos = buf.begin() + read_index_pos;
		std::copy(pos, pos + sizeof(val), (char*)&val);
		read_index_pos += sizeof(val);
		return *this;
	}

	package& package::operator>>(std::string& val)
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

	package& package::operator>>(std::vector<char>& val)
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
	package& package::operator<<(const T& val)
	{
		buf.resize(buf.size() + sizeof(val));
		auto pos = buf.end() - sizeof(val);
		std::copy((char*)&val, (char*)&val + sizeof(val), pos);
		reset_internal_iterator();
		return *this;
	}

	package& package::operator<<(const std::string& val)
	{
		if (val.empty()) throw std::runtime_error("Invalid data to write: it was null!");
		measure_siz sizer = val.size();
		*this << sizer;
		for (const auto& i : val) buf.push_back(i);
		reset_internal_iterator();
		return *this;
	}

	package& package::operator<<(const std::vector<char>& val)
	{
		if (val.empty()) throw std::runtime_error("Invalid data to write: it was null!");
		measure_siz sizer = val.size();
		*this << sizer;
		for (const auto& i : val) buf.push_back(i);
		reset_internal_iterator();
		return *this;
	}

	void package::reset_internal_iterator()
	{
		read_index_pos = 0;
	}

	std::vector<char> package::export_as_array() const
	{
		return buf;
	}

	void package::import_as_array(std::vector<char>&& oth)
	{
		buf = std::move(oth);
	}

	inline package::operator std::vector<char>() const
	{
		return buf;
	}

}