#pragma once

#include <random>
#include <chrono>

namespace Lunaris {

	class custom_random {
		std::random_device rd;
		std::mt19937::result_type seed;
		std::mt19937 gen;
		std::uniform_int_distribution<unsigned long long> distrib;
	public:
		custom_random();

		unsigned long long random();
	};

	inline custom_random __random_inline_g;

	/// <summary>
	/// <para>Get a random number, like rand(), but better.</para>
	/// </summary>
	/// <returns>{unsigned long long} A random number.</returns>
	unsigned long long random();
}