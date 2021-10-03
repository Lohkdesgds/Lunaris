#pragma once

#include <Lunaris/__macro/macros.h>

#include <windows.h>
#include <wininet.h>
#include <ShlObj.h>
#include <string>
#include <thread>
#include <functional>

#pragma comment (lib, "wininet.lib")

namespace Lunaris {
	class downloader {
		std::vector<char> buf;
		size_t TotalBytesRead = 0;

		bool run_get_with_func(const std::string&, std::function<void(const char*, const size_t)>);
	public:
		/// <summary>
		/// <para>Download content from this URL and stores in a internal Buffer.</para>
		/// </summary>
		/// <param name="{std::string}">The URL.</param>
		/// <returns>{bool} True if succeess.</returns>
		bool get(const std::string&);

		/// <summary>
		/// <para>Download content from this URL and stores directly using the function.</para>
		/// </summary>
		/// <param name="{std::string}">The URL.</param>
		/// <param name="{std::function}">The function receiving the download feed (in chunks).</param>
		/// <returns>{bool} True if succeess.</returns>
		bool get_store(const std::string&, std::function<void(const char*, const size_t)>);

		/// <summary>
		/// <para>Total bytes in memory right now (might not be 100% accurate).</para>
		/// </summary>
		/// <returns>{size_t} Total bytes in memory.</returns>
		size_t bytes_read() const;

		/// <summary>
		/// <para>Reads data from Buffer.</para>
		/// </summary>
		/// <returns>{Buffer} Constant reference to Buffer.</returns>
		const std::vector<char>& read() const;

		/// <summary>
		/// <para>Clears internal buffer.</para>
		/// </summary>
		void clear_buf();
	};

}