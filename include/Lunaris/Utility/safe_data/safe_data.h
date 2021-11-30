#pragma once

#include <Lunaris/__macro/macros.h>

#include <Lunaris/Utility/mutex.h>

#include <shared_mutex>
#include <functional>

namespace Lunaris {

	/// <summary>
	/// <para>safe_data is a safe way to store a variable. All changes are 100% safe.</para>
	/// <para>Shared_mutex is used alongside it, and some read-only stuff can be parallel!</para>
	/// <para>You should be able to lock itself back many times in the same thread, because the shared_muted is also recursive.</para>
	/// </summary>
	template<typename T>
	class safe_data {
		mutable shared_recursive_mutex shrmtx;
		T data;
	public:
		safe_data() = default;

		/// <summary>
		/// <para>Copy a safe_data into this.</para>
		/// </summary>
		/// <param name="{safe_data}">A safe_data.</param>
		safe_data(const safe_data&);

		/// <summary>
		/// <para>Move a safe_data into this.</para>
		/// </summary>
		/// <param name="{safe_data}">A safe_data.</param>
		safe_data(safe_data&&);

		/// <summary>
		/// <para>Copy a value into this safely.</para>
		/// </summary>
		/// <param name="{T}">The variable to copy.</param>
		safe_data(const T&);

		/// <summary>
		/// <para>Move a value into this safely.</para>
		/// </summary>
		/// <param name="{T}">The variable to move.</param>
		safe_data(T&&);

		/// <summary>
		/// <para>Read the value right now (copy)</para>
		/// </summary>
		/// <returns>{T} A copy of the current value set.</returns>
		T read() const;

		/// <summary>
		/// <para>Read the value in constant mode (shared reading mode).</para>
		/// </summary>
		/// <param name="{function}">A function that reads the variable.</param>
		void csafe(const std::function<void(const T&)>) const;

		/// <summary>
		/// <para>Read the value in read/write mode (exclusive mode).</para>
		/// </summary>
		/// <param name="{function}">A function that does something with the variable.</param>
		void safe(const std::function<void(T&)>);

		/// <summary>
		/// <para>Set the value directly.</para>
		/// </summary>
		/// <param name="{T}">The value to copy and set.</param>
		void set(const T&);

		/// <summary>
		/// <para>Set the value directly (moving).</para>
		/// </summary>
		/// <param name="{T&amp;&amp;}">The value to move and set.</param>
		void set(T&&);

		/// <summary>
		/// <para>Reset the internal value to "zero".</para>
		/// </summary>
		/// <returns>{T} The value that was there exactly before this.</returns>
		T reset();

		/// <summary>
		/// <para>Cast self to T (copy).</para>
		/// </summary>
		operator T() const;

		/// <summary>
		/// <para>Copy a safe_data into this.</para>
		/// </summary>
		/// <param name="{safe_data}">A safe_data.</param>
		void operator=(const safe_data&);

		/// <summary>
		/// <para>Move a safe_data into this.</para>
		/// </summary>
		/// <param name="{safe_data}">A safe_data.</param>
		void operator=(safe_data&&);

		/// <summary>
		/// <para>Copy a value into this safely.</para>
		/// </summary>
		/// <param name="{T}">The variable to copy.</param>
		void operator=(const T&);

		/// <summary>
		/// <para>Move a value into this safely.</para>
		/// </summary>
		/// <param name="{T}">The variable to move.</param>
		void operator=(T&&);
	};

	/// <summary>
	/// <para>save_vector is a safe way to store a vector of values. All changes are 100% safe.</para>
	/// <para>Shared_mutex is used alongside it, and some read-only stuff can be parallel!</para>
	/// <para>You should be able to lock itself back many times in the same thread, because the shared_muted is also recursive.</para>
	/// </summary>
	template<typename T>
	class safe_vector {
		mutable shared_recursive_mutex shrmtx;
		std::vector<T> data;
	public:
		safe_vector() = default;

		/// <summary>
		/// <para>Copy a vector into this.</para>
		/// </summary>
		/// <param name="{safe_vector}">Another vector to copy from.</param>
		safe_vector(const safe_vector&);

		/// <summary>
		/// <para>Move a vector into this.</para>
		/// </summary>
		/// <param name="{safe_vector}">Another vector to copy from.</param>
		safe_vector(safe_vector&&);

		/// <summary>
		/// <para>Copy a vector into this.</para>
		/// </summary>
		/// <param name="{safe_vector}">Another vector to copy from.</param>
		void operator=(const safe_vector&);

		/// <summary>
		/// <para>Move a vector into this.</para>
		/// </summary>
		/// <param name="{safe_vector}">Another vector to copy from.</param>
		void operator=(safe_vector&&);

		/// <summary>
		/// <para>Add new value into the vector safely.</para>
		/// </summary>
		/// <param name="{T}">Value to copy.</param>
		void push_back(const T&);

		/// <summary>
		/// <para>Move new value into the vector safely.</para>
		/// </summary>
		/// <param name="{T}">Value to move.</param>
		void push_back(T&&);

		/// <summary>
		/// <para>Copy value at this index value.</para>
		/// </summary>
		/// <param name="{size_t}">Index offset.</param>
		/// <returns>{T} A copy of the value at that position.</returns>
		T index(const size_t&) const;

		/// <summary>
		/// <para>Copy value at this index value.</para>
		/// </summary>
		/// <param name="{size_t}">Index offset.</param>
		/// <returns>{T} A copy of the value at that position.</returns>
		T operator[](const size_t) const;

		/// <summary>
		/// <para>Set the value at this point in the vector.</para>
		/// </summary>
		/// <param name="{size_t}">Offset.</param>
		/// <param name="{T}">Value to copy.</param>
		/// <param name="{bool}">If out of range, try to resize to that? (Filling empty spaces with "zero")</param>
		/// <returns>{bool} True if in range or successful (if force resize).</returns>
		bool set(const size_t, const T&, const bool = false);

		/// <summary>
		/// <para>Set the value at this point in the vector (moving).</para>
		/// </summary>
		/// <param name="{size_t}">Offset.</param>
		/// <param name="{T}">Value to move.</param>
		/// <param name="{bool}">If out of range, try to resize to that? (Filling empty spaces with "zero")</param>
		/// <returns>{bool} True if in range or successful (if force resize).</returns>
		bool set(const size_t, T&&, const bool = false);

		/// <summary>
		/// <para>Read the vector in constant mode (shared reading mode).</para>
		/// </summary>
		/// <param name="{function}">A function that reads the vector.</param>
		void csafe(const std::function<void(const std::vector<T>&)>);

		/// <summary>
		/// <para>Read the value in read/write mode (exclusive mode).</para>
		/// </summary>
		/// <param name="{function}">A function that does something with the variable.</param>
		void safe(const std::function<void(std::vector<T>&)>);

		/// <summary>
		/// <para>Erase an exact index point in the vector.</para>
		/// </summary>
		/// <param name="{size_t}">Offset.</param>
		void erase(const size_t);
		
		/// <summary>
		/// <para>Erase from point A to point B.</para>
		/// <para>If A is greater than vector size, it does nothing.</para>
		/// <para>If B is greater than vector size, it'll erase till the end().</para>
		/// </summary>
		/// <param name="{size_t}">Offset A.</param>
		/// <param name="{size_t}">Offset B.</param>
		void erase(const size_t, const size_t);

		/// <summary>
		/// <para>Clears the entire vector.</para>
		/// </summary>
		void clear();

		/// <summary>
		/// <para>Get vector size.</para>
		/// </summary>
		/// <returns>{size_t} Vector size.</returns>
		size_t size() const;
	};
}

#include "safe_data.ipp"