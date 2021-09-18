#pragma once

#include <vector>
#include <string>


namespace LSW {
	namespace v5 {
		namespace Tools {

			/// <summary>
			/// <para>Buffer is a std::vector of char aimed for data storage.</para>
			/// </summary>
			class Buffer : public std::vector<char> {
				mutable std::string _temp;
			public:
				Buffer() = default;
				using std::vector<char>::vector;
				using std::vector<char>::operator=;

				/// <summary>
				/// <para>Copy data from string.</para>
				/// </summary>
				/// <param name="{std::string}">Some data to copy.</param>
				Buffer(const std::string&);

				/// <summary>
				/// <para>Copy data from raw string.</para>
				/// </summary>
				/// <param name="{const char*}">Raw string.</param>
				Buffer(const char*);

				/// <summary>
				/// <para>Copy data from raw string (with limit).</para>
				/// </summary>
				/// <param name="{const char*}">Raw string.</param>
				/// <param name="{size_t}">String size.</param>
				Buffer(const char*, const size_t);

				/// <summary>
				/// <para>Copy data from something (with limit).</para>
				/// </summary>
				/// <param name="{const void*}">Raw data.</param>
				/// <param name="{size_t}">Data size.</param>
				Buffer(const void*, const size_t);

				/// <summary>
				/// <para>Automatically cast to string (copy).</para>
				/// </summary>
				operator std::string();

				/// <summary>
				/// <para>Automatically cast to const string (reference, may not update and may break if original object dies).</para>
				/// </summary>
				operator const std::string&() const;

				/// <summary>
				/// <para>Compare two buffers.</para>
				/// </summary>
				/// <param name="{Buffer}">Another buffer.</param>
				/// <returns>{bool} True if equal in size and content.</returns>
				bool operator==(const Buffer&) const;

				/// <summary>
				/// <para>Compare this buffer with string.</para>
				/// </summary>
				/// <param name="{std::string}">A string.</param>
				/// <returns>{bool} True if equal in size and content.</returns>
				bool operator==(const std::string&) const;

				/// <summary>
				/// <para>Compare this buffer with raw string.</para>
				/// </summary>
				/// <param name="{const char*}">A raw string.</param>
				/// <returns>{bool} True if equal in size and content.</returns>
				bool operator==(const char*) const;

				/// <summary>
				/// <para>Compare two Buffers.</para>
				/// </summary>
				/// <param name="{Buffer}">Another Buffer.</param>
				/// <returns>{bool} True if not equal in size and content.</returns>
				bool operator!=(const Buffer&) const;

				/// <summary>
				/// <para>Compare this buffer with string.</para>
				/// </summary>
				/// <param name="{std::string}">A string.</param>
				/// <returns>{bool} True if not equal in size and content.</returns>
				bool operator!=(const std::string&) const;

				/// <summary>
				/// <para>Compare this buffer with raw string.</para>
				/// </summary>
				/// <param name="{const char*}">A raw string.</param>
				/// <returns>{bool} True if not equal in size and content.</returns>
				bool operator!=(const char*) const;

				/// <summary>
				/// <para>Copy a Buffer data to this.</para>
				/// </summary>
				/// <param name="{Buffer}">Another Buffer.</param>
				/// <returns>{Buffer} Self.</returns>
				Buffer& operator=(const Buffer&);

				/// <summary>
				/// <para>Copy a string data to this.</para>
				/// </summary>
				/// <param name="{std::string}">A string.</param>
				/// <returns>{Buffer} Self.</returns>
				Buffer& operator=(const std::string&);

				/// <summary>
				/// <para>Copy a raw string data to this.</para>
				/// </summary>
				/// <param name="{const char*}">A raw string.</param>
				/// <returns>{Buffer} Self.</returns>
				Buffer& operator=(const char*);

				/// <summary>
				/// <para>Push back some data.</para>
				/// </summary>
				/// <param name="{char}">A single byte.</param>
				void operator+=(const char&);

				/// <summary>
				/// <para>Push back some data.</para>
				/// </summary>
				/// <param name="{Buffer}">A Buffer.</param>
				void operator+=(const Buffer&);

				/// <summary>
				/// <para>Push back some data.</para>
				/// </summary>
				/// <param name="{std::string}">A string.</param>
				void operator+=(const std::string&);

				/// <summary>
				/// <para>Copy current data to somewhere else.</para>
				/// </summary>
				/// <param name="{void*}">A pointer to store data.</param>
				/// <param name="{size_t}">Size of the pointer's destination.</param>
				void copy_to(void*, const size_t) const;

				/// <summary>
				/// <para>Cast this as string.</para>
				/// </summary>
				/// <returns>{std::string} A string with the data.</returns>
				const std::string cast_as_string() const;
			};

		}
	}
}