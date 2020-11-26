#pragma once

#include <chrono>
#include <vector>
#include <string>

#include "../Common/common.h"

namespace LSW {
	namespace v5 {
		namespace Tools {

			/// <summary>
			/// <para>Debugging sometimes needs some timings, especially when the problem is low performance. Use this to register some points in time!</para>
			/// </summary>
			class Stopwatch {
				std::vector<std::chrono::steady_clock::time_point> stops;
				std::vector<double> max_diffs;
				size_t point = 0, reserved = 0;
			public:
				/// <summary>
				/// <para>Resize the internal vector to this size, so you can call call + this - 1 (call counts).</para>
				/// </summary>
				/// <param name="{size_t}">Amount of points in time.</param>
				void prepare(const size_t);

				/// <summary>
				/// <para>Reset to 0 the max differences.</para>
				/// </summary>
				void reset_max();

				/// <summary>
				/// <para>Reset to 0 and click once already.</para>
				/// </summary>
				void start();

				/// <summary>
				/// <para>Register next point.</para>
				/// </summary>
				void click_one();

				/// <summary>
				/// <para>Difference in time between two points (first, last).</para>
				/// </summary>
				/// <param name="{size_t}">Call index, starting in 0 when start().</param>
				/// <param name="{size_t}">Call index, starting in 0 when start().</param>
				/// <returns>{std::chrono::nanoseconds} Difference in time.</returns>
				std::chrono::nanoseconds get_nanosec_between(const size_t, const size_t);

				/// <summary>
				/// <para>Difference in time between two points (first, last).</para>
				/// </summary>
				/// <param name="{size_t}">Call index, starting in 0 when start().</param>
				/// <param name="{size_t}">Call index, starting in 0 when start().</param>
				/// <returns>{std::chrono::microseconds} Difference in time.</returns>
				std::chrono::microseconds get_microsec_between(const size_t, const size_t);

				/// <summary>
				/// <para>Difference in time between two points (first, last).</para>
				/// </summary>
				/// <param name="{size_t}">Call index, starting in 0 when start().</param>
				/// <param name="{size_t}">Call index, starting in 0 when start().</param>
				/// <returns>{std::chrono::milliseconds} Difference in time.</returns>
				std::chrono::milliseconds get_millisec_between(const size_t, const size_t);

				/// <summary>
				/// <para>Difference in time between two points (first, last).</para>
				/// </summary>
				/// <param name="{size_t}">Call index, starting in 0 when start().</param>
				/// <param name="{size_t}">Call index, starting in 0 when start().</param>
				/// <returns>{std::chrono::seconds} Difference in time.</returns>
				std::chrono::seconds get_seconds_between(const size_t, const size_t);

				/// <summary>
				/// <para>Maximum difference in time between two points (first, first + 1).</para>
				/// </summary>
				/// <param name="{size_t}">Call index, starting in 0 when start().</param>
				/// <returns>{double} The difference, in seconds.</returns>
				double get_max_between_p(const size_t);

				/// <summary>
				/// <para>Difference in time between two points (first, last) automatic range to string.</para>
				/// </summary>
				/// <param name="{size_t}">Call index, starting in 0 when start().</param>
				/// <param name="{size_t}">Call index, starting in 0 when start().</param>
				/// <returns>{std::string} Difference in time.</returns>
				std::string get_string_between(const size_t, const size_t);

				/// <summary>
				/// <para>Maximum difference in time between two points (first, first + 1) automatic range to string.</para>
				/// </summary>
				/// <param name="{size_t}">Call index, starting in 0 when start().</param>
				/// <returns>{std::string} Max difference in time.</returns>
				std::string get_string_between_p(const size_t);

				/// <summary>
				/// <para>If you prepare(5), this will return 4, because [4] == 5th point, got it?</para>
				/// </summary>
				/// <returns>{size_t} Last point (1 less than total size).</returns>
				size_t last_point_valid();

				/// <summary>
				/// <para>Generates table onw by one with automatic scale, one per line.</para>
				/// </summary>
				/// <returns>{std::string} Table.</returns>
				std::string generate_table_statistics();
			};

		}
	}
}