#pragma once

#include <Lunaris/__macro/macros.h>

// POSSIBLE CUSTOMIZATIONS:
// - LUNARIS_IO_NO_NEW_LINE						: Disables automatic new line after each cout << ... call
// - LUNARIS_IO_DO_NOT_RESET_COLOR_EACH_LINE		: Disables automatic color reset after each cout << ... call

#include <iostream>
#include <mutex>
#include <type_traits>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace Lunaris {

	/// <summary>
	/// <para>This is a multi-thread safe easy to use class cout-like. It does use cout in the end.</para>
	/// </summary>
	class console {
	public:
		/// <summary>
		/// <para>These are the colors supported on console (cmd).</para>
		/// </summary>
#ifdef _WIN32
		enum class color { BLACK, DARK_BLUE, DARK_GREEN, DARK_AQUA, DARK_RED, DARK_PURPLE, GOLD, GRAY, DARK_GRAY, BLUE, GREEN, AQUA, RED, LIGHT_PURPLE, YELLOW, WHITE };
#else
		enum class color { BLACK, DARK_RED, DARK_GREEN, GOLD, DARK_BLUE, DARK_PURPLE, DARK_AQUA, GRAY, DARK_GRAY, RED, GREEN, YELLOW, BLUE, LIGHT_PURPLE, AQUA, WHITE };
#endif
	private:
#ifndef LUNARIS_VERBOSE_BUILD
		std::mutex m_safe;
		using mutex_type = std::mutex;
#else
		using mutex_type = std::recursive_mutex;
#endif

		class _block_control {
			std::unique_lock<mutex_type> m_lock_safe;
		public:
			// newline on death
			~_block_control();

			// Start block with locked argument
			_block_control(std::unique_lock<mutex_type>&&);

			// So it does move stuff around, hm? (VS asked for this lmao)
			_block_control(_block_control&&) noexcept;

			// Set console color
			_block_control& operator<<(const color&);

			// Print or setup something (std::cout)
			template<typename T, typename = std::enable_if_t<!std::is_same_v<std::decay<T>, color>, int>>
			_block_control& operator<<(const T&);
		};
	public:
		/// <summary>
		/// <para>Any type goes internally to a block control. This is the main printing way.</para>
		/// </summary>
		template<typename T>
		_block_control operator<<(const T&);
	};

	inline console cout;
}

#include "console.ipp"