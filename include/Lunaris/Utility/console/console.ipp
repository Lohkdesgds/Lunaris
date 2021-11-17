#include "console.h"
#pragma once

namespace Lunaris {

	inline console::_block_control::_block_control(std::unique_lock<mutex_type>&& lock)
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
#ifdef _WIN32
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, static_cast<int>(color));
#else
		std::cout << "\033[" << (static_cast<uint8_t>(color) >= 8 ? (static_cast<uint8_t>(color) + 90 - 8) : (static_cast<uint8_t>(color) + 30)) << "m";
#endif
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
#ifdef LUNARIS_VERBOSE_BUILD
		std::unique_lock<mutex_type> lock(__g_verbose_lock); // global mutex for print
		PRINT_DEBUG(nullptr);
#else
		std::unique_lock<mutex_type> lock(m_safe);
#endif
		console::_block_control _block(std::move(lock));
		_block << var;
		return _block;
	}

}