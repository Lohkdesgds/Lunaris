#pragma once

#include <Lunaris/__macro/macros.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_memfile.h>
#include <stdexcept>
#include <string>
#include <memory>
#if (_WIN32)
#include <Windows.h>
#endif

namespace Lunaris {

#ifdef _WIN32
#ifdef UNICODE
	using WinString = LPWSTR;
#else
	using WinString = LPSTR;
#endif
	enum class resource_type_e { // based on https://docs.microsoft.com/en-us/windows/win32/menurc/resource-types
		CURSOR_DEP      = (int)((WORD)((ULONG_PTR)RT_CURSOR)),			// Hardware-dependent cursor resource.
		BITMAP          = (int)((WORD)((ULONG_PTR)RT_BITMAP)),			// Bitmap resource.
		ICON_DEP        = (int)((WORD)((ULONG_PTR)RT_ICON)),			// Hardware-dependent icon resource.
		MENU            = (int)((WORD)((ULONG_PTR)RT_MENU)),			// Menu resource.
		DIALOG          = (int)((WORD)((ULONG_PTR)RT_DIALOG)),			// Dialog box.
		STRING          = (int)((WORD)((ULONG_PTR)RT_STRING)),			// String-table entry.
		FONTDIR         = (int)((WORD)((ULONG_PTR)RT_FONTDIR)),			// Font directory resource.
		FONT            = (int)((WORD)((ULONG_PTR)RT_FONT)),			// Font resource.
		ACCELERATOR     = (int)((WORD)((ULONG_PTR)RT_ACCELERATOR)),		// Accelerator table.
		RCDATA          = (int)((WORD)((ULONG_PTR)RT_RCDATA)),			// Application-defined resource (raw data).
		MESSAGETABLE    = (int)((WORD)((ULONG_PTR)RT_MESSAGETABLE)),	// Message-table entry.
		CURSOR_INDEP    = (int)((WORD)((ULONG_PTR)RT_GROUP_CURSOR)),	// Hardware-independent cursor resource.
		ICON_INDEP      = (int)((WORD)((ULONG_PTR)RT_GROUP_ICON)),		// Hardware-independent icon resource.
		VERSION         = (int)((WORD)((ULONG_PTR)RT_VERSION)),			// Version resource.
		DLGINCLUDE      = (int)((WORD)((ULONG_PTR)RT_DLGINCLUDE)),		// Allows a resource editing tool to associate a string with an .rc file.
		PLUGPLAY        = (int)((WORD)((ULONG_PTR)RT_PLUGPLAY)),		// Plug and play resource.
		VXD             = (int)((WORD)((ULONG_PTR)RT_VXD)),				// VXD.
		ANICURSOR       = (int)((WORD)((ULONG_PTR)RT_ANICURSOR)),		// Animated cursor.
		ANIICON         = (int)((WORD)((ULONG_PTR)RT_ANIICON)),			// Animated icon.
		HTML            = (int)((WORD)((ULONG_PTR)RT_HTML)),			// HTML resource.
		MANIFEST        = (int)((WORD)((ULONG_PTR)RT_MANIFEST))			// Side-by-Side Assembly Manifest.
	};
#endif

	void __file_allegro_start();

	class file {
	protected:
		ALLEGRO_FILE* fp = nullptr;
		std::string last_path;
		bool is_temp = false;
	public:
		file() = default;
		virtual ~file();

		file(const file&) = delete;
		void operator=(const file&) = delete;

		file(file&&) noexcept;
		void operator=(file&&) noexcept;

		// path, mode
		bool open(const std::string&, const std::string&);

		// template, mode
		bool open_temp(const std::string&, const std::string&);

		const std::string& get_current_path() const;

		virtual void close();

		virtual size_t read(char*, const size_t);
		virtual size_t write(const char*, const size_t);
		virtual size_t tell();
		virtual bool seek(const int64_t, const ALLEGRO_SEEK);
		virtual bool flush();
		virtual size_t size() const;

		virtual void delete_and_close();

		operator const ALLEGRO_FILE* () const;
	};

	class memfile : protected file {
		std::unique_ptr<char[]> mem;
	public:
		using file::operator const ALLEGRO_FILE*;
		using file::read;
		using file::write;
		using file::tell;
		using file::seek;
		using file::flush;
		using file::size;

		memfile() = default;
		~memfile();

		memfile(const memfile&) = delete;
		void operator=(const memfile&) = delete;

		memfile(memfile&&) noexcept;
		void operator=(memfile&&) noexcept;

		bool open(const size_t);

		void close();
	};

#ifdef _WIN32 // && _MSC_VER
	// resource.h defined value like IDR_TTF1, id, expected extension (".jpg", ".png", ...)
	file __get_executable_resource_as_file(const int, const WinString, const std::string&);
	// resource.h defined value like IDR_TTF1, id as enum, expected extension (".jpg", ".png", ...)
	file get_executable_resource_as_file(const int, const resource_type_e, const std::string&);
	// resource.h defined value like IDR_TTF1, id
	memfile __get_executable_resource_as_memfile(const int, const WinString);
	// resource.h defined value like IDR_TTF1, id as enum
	memfile get_executable_resource_as_memfile(const int, const resource_type_e);
#endif
}