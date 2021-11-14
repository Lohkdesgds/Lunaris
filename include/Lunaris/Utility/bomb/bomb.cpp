#include "bomb.h"

namespace Lunaris {

	LUNARIS_DECL bomb::bomb(const std::function<void(void)> f)
		: boom(f)
	{
		if (!boom) throw std::runtime_error("Expected a valid function @ bomb!");
	}

	LUNARIS_DECL bomb::~bomb()
	{
		if (boom) boom();
	}

	LUNARIS_DECL bomb::bomb(bomb&& boo) noexcept
		: boom(std::move(boo.boom))
	{
	}

	LUNARIS_DECL void bomb::operator=(bomb&& boo) noexcept
	{
		if (boom) boom();
		boom = std::move(boo.boom);
	}

	LUNARIS_DECL std::function<void(void)> bomb::defuse()
	{
		return std::move(boom);
	}

	LUNARIS_DECL bool bomb::is_defused() const
	{
		return !boom.operator bool();
	}

}