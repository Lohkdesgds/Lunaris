#pragma once

#include <Lunaris/__macro/macros.h>

#include <functional>
#include <stdexcept>

namespace Lunaris {

	class bomb : public NonCopyable {
		std::function<void(void)> boom;
	public:
		/// <summary>
		/// <para>Arm the bomb (when destroyed, this is triggered, unless it's defused!</para>
		/// </summary>
		/// <param name="{function}">A function-bomb.</param>
		bomb(const std::function<void(void)>);

		// detonate
		~bomb();

		/// <summary>
		/// <para>Move the BOMB!</para>
		/// </summary>
		/// <param name="{bomb}">The bomb being moved!</param>
		bomb(bomb&&) noexcept;

		/// <summary>
		/// <para>Move the BOMB!</para>
		/// <para>NOTE! If this bomb here has a bomb inside, it'll be triggered! BE CAREFUL!</para>
		/// </summary>
		/// <param name="{bomb}">The bomb being moved!</param>
		void operator=(bomb&&) noexcept;

		/// <summary>
		/// <para>Defuse the bomb! (The function will not be triggered)</para>
		/// </summary>
		/// <returns>The function set, if you want that.</returns>
		std::function<void(void)> defuse();

		/// <summary>
		/// <para>Is this bomb defused?</para>
		/// </summary>
		/// <returns>{bool} Defused?</returns>
		bool is_defused() const;
	};

}