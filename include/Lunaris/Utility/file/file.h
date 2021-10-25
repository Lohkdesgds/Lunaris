#pragma once

#include <Lunaris/__macro/macros.h>

#include <allegro5/allegro5.h>
#include <stdexcept>
#include <string>
#if (_WIN32)
#include <Windows.h>
#endif

namespace Lunaris {

#ifdef _WIN32
#ifdef UNICODE
	using WinString = std::wstring;
	using WinStringAlt = std::string;
#else
	using WinString = std::string;
	using WinStringAlt = std::wstring;
#endif
#endif

	void __file_allegro_start();

	class file {
		ALLEGRO_FILE* fp = nullptr;
		std::string last_path;
		bool is_temp = false;
	public:
		file() = default;
		~file();

		file(const file&) = delete;
		void operator=(const file&) = delete;

		file(file&&) noexcept;
		void operator=(file&&) noexcept;

		// path, mode
		bool open(const std::string&, const std::string&);

		// template, mode
		bool open_temp(const std::string&, const std::string&);

		const std::string& get_current_path() const;

		void close();

		size_t read(char*, const size_t);
		size_t write(const char*, const size_t);
		size_t tell();
		bool seek(const int64_t, const ALLEGRO_SEEK);
		bool flush();
		size_t size() const;

		void delete_and_close();

		operator const ALLEGRO_FILE* () const;
	};

#ifdef _WIN32 // && _MSC_VER
	// resource.h defined value like IDR_TTF1, its name as string, expected extension (".jpg", ".png", ...)
	file get_executable_resource_as_file(const int, const WinString&, const std::string&);
	file get_executable_resource_as_file(const int, const WinStringAlt&, const std::string&);
#endif
}