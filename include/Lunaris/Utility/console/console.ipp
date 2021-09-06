#include "console.h"
#pragma once

namespace Lunaris {

	inline console::_block_control::_block_control(std::unique_lock<std::mutex>&& lock)
		: m_lock_safe(std::move(lock))
	{
	}
	
	inline console::_block_control::_block_control(_block_control&& oth) noexcept
		: m_lock_safe(std::move(oth.m_lock_safe))
	{
	}

	inline console::_block_control::~_block_control()
	{
		if (!m_lock_safe.owns_lock()) return; // if moved, it won't own anything, so it should not reset stuff!
#ifndef LUNARIS_IO_NO_NEW_LINE
		std::cout << '\n';
#endif
#ifndef LUNARIS_IO_DO_NOT_RESET_COLOR_EACH_LINE
		*this << color::GRAY;
#endif
	}

	inline console::_block_control& console::_block_control::operator<<(const color& color)
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, static_cast<int>(color));
		return *this;
	}

	template<typename T, typename>
	inline console::_block_control& console::_block_control::operator<<(const T& var)
	{
		std::cout << var;
		return *this;
	}

	template<typename T>
	inline console::_block_control console::operator<<(const T& var)
	{
		std::unique_lock<std::mutex> lock(m_safe);
		console::_block_control _block(std::move(lock));
		_block << var;
		return _block;
	}

}