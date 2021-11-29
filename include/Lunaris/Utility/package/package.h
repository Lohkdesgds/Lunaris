#pragma once

#include <Lunaris/__macro/macros.h>

#include <thread>
#include <functional>
#include <memory>

namespace Lunaris {

	/// <summary>
	/// <para>package is a useful tool to combine stuff in a pretty array of char and then get back easily in order.</para>
	/// <para>The operators &lt;&lt; and &gt;&gt; works as you'd expect: write and read easy!</para>
	/// <para>You can also read the entire thing as array or export to a buffer, or write as an array or buffer as well.</para>
	/// </summary>
	class package {
		using measure_siz = size_t;
		std::vector<char> buf;
		size_t read_index_pos = 0;
	public:
		package() = default;
		
		/// <summary>
		/// <para>Copy a package buffer.</para>
		/// </summary>
		/// <param name="{package}">Another package.</param>
		package(const package&);
		
		/// <summary>
		/// <para>Move a package buffer.</para>
		/// </summary>
		/// <param name="{package}">Another package.</param>
		package(package&&) noexcept;

		/// <summary>
		/// <para>Build the package from a vector of char.</para>
		/// </summary>
		/// <param name="{vector&lt;char&gt;}">Array of char.</param>
		package(const std::vector<char>&);

		/// <summary>
		/// <para>Build the package from a vector of char (moving it).</para>
		/// </summary>
		/// <param name="{vector&lt;char&gt;}">Array of char.</param>
		package(std::vector<char>&&) noexcept;

		/// <summary>
		/// <para>Reset and copy package buffer.</para>
		/// </summary>
		/// <param name="{package}">Another package.</param>
		void operator=(const package&);

		/// <summary>
		/// <para>Reset and move package buffer.</para>
		/// </summary>
		/// <param name="{package}">Another package.</param>
		void operator=(package&&) noexcept;

		/// <summary>
		/// <para>Reset and build the package from a vector of char.</para>
		/// </summary>
		/// <param name="{vector&lt;char&gt;}">Array of char.</param>
		void operator=(const std::vector<char>&);

		/// <summary>
		/// <para>Reset and build the package from a vector of char (moving it).</para>
		/// </summary>
		/// <param name="{vector&lt;char&gt;}">Array of char.</param>
		void operator=(std::vector<char>&&) noexcept;

		/// <summary>
		/// <para>Compare this to another package (byte comparison).</para>
		/// </summary>
		/// <param name="{package}">Another package to compare</param>
		/// <returns>{bool} True if equal</returns>
		bool operator==(const package&) const;

		/// <summary>
		/// <para>Compare this to another package (byte comparison).</para>
		/// </summary>
		/// <param name="{package}">Another package to compare</param>
		/// <returns>{bool} True if different</returns>
		bool operator!=(const package&) const;

		/// <summary>
		/// <para>Write next buffer to a variable.</para>
		/// <para>The offset is increased by the variable's size in bytes.</para>
		/// </summary>
		/// <param name="{T&amp;}">Variable getting the data.</param>
		/// <returns>{package&amp;} Itself.</returns>
		template<typename T, std::enable_if_t<std::is_pod_v<T> && !std::is_array_v<T> && !std::is_pointer_v<T>, int> = 0>
		package& operator>>(T&);

		/// <summary>
		/// <para>Write next buffer to a variable.</para>
		/// <para>The offset in this case is pre-determined by the write function. It is automatic. The string size should have the size you've set before.</para>
		/// </summary>
		/// <param name="{string}">Variable getting the data.</param>
		/// <returns>{package&amp;} Itself.</returns>
		package& operator>>(std::string&);

		/// <summary>
		/// <para>Write next buffer to a variable.</para>
		/// <para>The offset in this case is pre-determined by the write function. It is automatic. The string size should have the size you've set before.</para>
		/// </summary>
		/// <param name="{vector&lt;char&gt;}">Variable getting the data.</param>
		/// <returns>{package&amp;} Itself.</returns>
		package& operator>>(std::vector<char>&);

		/// <summary>
		/// <para>Write a variable into the buffer.</para>
		/// <para>The offset (size) is increased automatically.</para>
		/// </summary>
		/// <param name="{T}">A variable to copy.</param>
		/// <returns>{package&amp;} Itself.</returns>
		template<typename T, std::enable_if_t<std::is_pod_v<T> && !std::is_array_v<T> && !std::is_pointer_v<T>, int> = 0>
		package& operator<<(const T&);

		/// <summary>
		/// <para>Write a variable into the buffer.</para>
		/// <para>The length is stored alongside the array data itself.</para>
		/// </summary>
		/// <param name="{string}">The string to copy.</param>
		/// <returns>{package&amp;} Itself.</returns>
		package& operator<<(const std::string&);

		/// <summary>
		/// <para>Write a variable into the buffer.</para>
		/// <para>The length is stored alongside the array data itself.</para>
		/// </summary>
		/// <param name="{vector&lt;char&gt;}">Variable with the data to copy.</param>
		/// <returns>{package&amp;} Itself.</returns>
		package& operator<<(const std::vector<char>&);

		/// <summary>
		/// <para>As you use &gt;&gt; or &lt;&lt; operators, an internal offset is set so you can combo things.</para>
		/// <para>This function resets the offset back to zero.</para>
		/// </summary>
		void reset_internal_iterator();

		/// <summary>
		/// <para>Read the buffer as is.</para>
		/// </summary>
		/// <returns>{const vector&lt;char&gt;&amp;} Internal buffer reference.</returns>
		const std::vector<char>& read_as_array() const;

		/// <summary>
		/// <para>Read the buffer writing into your buffer directly.</para>
		/// </summary>
		/// <param name="{char*}">Where to write.</param>
		/// <param name="{size_t}">The size of your buffer.</param>
		/// <param name="{size_t}">Offset (first byte offset).</param>
		/// <returns>{size_t} The total bytes copied.</returns>
		size_t read_as_data(char*, const size_t, const size_t = 0) const;

		/// <summary>
		/// <para>Alternative way to reset and move data into this.</para>
		/// </summary>
		/// <param name="{vector&lt;char&gt;}">The source (move).</param>
		void import_as_array(std::vector<char>&&);

		/// <summary>
		/// <para>Alternative way to reset and move data into this.</para>
		/// </summary>
		/// <param name="{char*}">Buffer.</param>
		/// <param name="{size_t}">Buffer size.</param>
		void import_as_data(const char*, const size_t);

		/// <summary>
		/// <para>Cast self as vector of char easily.</para>
		/// </summary>
		operator const std::vector<char>&() const;

		/// <summary>
		/// <para>Internal data size.</para>
		/// </summary>
		/// <returns>{size_t} Size, in bytes.</returns>
		size_t size() const;

		/// <summary>
		/// <para>Whether there's data in it or not.</para>
		/// </summary>
		/// <returns>{bool} True if has data.</returns>
		bool valid() const;

		/// <summary>
		/// <para>Whether there's data in it or not.</para>
		/// </summary>
		/// <returns>{bool} True if has NO data stored.</returns>
		bool empty() const;
	};
}

#include "package.ipp"