#include "random.h"

namespace Lunaris {

	LUNARIS_DECL custom_random::custom_random()
	{
		seed = rd() ^ (
			static_cast<std::mt19937::result_type>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()) +
			static_cast<std::mt19937::result_type>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count())
			);
		gen = std::mt19937(seed);
	}

	LUNARIS_DECL unsigned long long custom_random::random()
	{
		return distrib(gen);
	}

	LUNARIS_DECL unsigned long long random()
	{
		return __random_inline_g.random();
	}

}