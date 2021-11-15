#pragma once

#include <Lunaris/__macro/macros.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_memfile.h>
#include <stdexcept>
#include <string>
#include <memory>
#include <functional>
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

	class file : private NonCopyable {
	public:
		enum class open_mode_e {
			READ_TRY,				// rb
			WRITE_REPLACE,			// wb
			READWRITE_REPLACE,		// wb+
			READWRITE_KEEP			// ab+
		};
		enum class seek_mode_e {
			BEGIN	= ALLEGRO_SEEK_SET,
			END		= ALLEGRO_SEEK_END,
			CURRENT	= ALLEGRO_SEEK_CUR,
		};
	protected:
		std::unique_ptr<ALLEGRO_FILE, bool(*)(ALLEGRO_FILE*)> fp{ nullptr, nullptr };
		std::string path;

		std::string transl(const open_mode_e&);

		friend class font; // font has to be able to "own" the file.
		/// <summary>
		/// <para>in very very specific cases sometimes you don't want the ALLEGRO_FILE to be destroyed. This unset/set current rule. Default is to destroy.</para>
		/// <para>Example: loading font from file.</para>
		/// <para>TRUE results in NO DESTRUCTION!</para>
		/// </summary>
		/// <param name=""></param>
		void modify_no_destroy(const bool);
	public:
		file() = default;
		file(file&&);
		virtual void operator=(file&&) noexcept;
		virtual ~file();

		virtual bool open(const std::string&, const open_mode_e&);
		virtual void close();

		const std::string& get_path();
		ALLEGRO_FILE* get_fp() const;
		operator ALLEGRO_FILE*() const;

		size_t read(char*, const size_t);
		size_t write(const char*, const size_t);
		size_t tell();
		bool seek(const int64_t, const seek_mode_e);
		bool flush();
		size_t size() const;
		bool is_open() const;
	};

	class tempfile : public file {
	public:
		tempfile() = default;
		tempfile(tempfile&&) noexcept;
		void operator=(tempfile&&) noexcept;
		~tempfile();

		bool open(const std::string&);
		void close();

		using file::get_path;
		using file::get_fp;
		using file::operator ALLEGRO_FILE*;
		using file::read;
		using file::write;
		using file::tell;
		using file::seek;
		using file::flush;
		using file::size;
		using file::is_open;
	};

	class memfile : public file {
		std::unique_ptr<char[]> mem;
	public:
		memfile() = default;
		memfile(memfile&&) noexcept;
		void operator=(memfile&&) noexcept;
		~memfile();

		bool open(const size_t);
		void close();

		using file::get_path;
		using file::get_fp;
		using file::operator ALLEGRO_FILE*;
		using file::read;
		using file::write;
		using file::tell;
		using file::seek;
		using file::flush;
		using file::size;
		using file::is_open;
	};

#ifdef _WIN32 // && _MSC_VER
	// resource.h defined value like IDR_TTF1, id, expected extension (".jpg", ".png", ...)
	tempfile get_executable_resource_as_file(const int, const WinString&, const std::string&);
	// resource.h defined value like IDR_TTF1, id as enum, expected extension (".jpg", ".png", ...)
	tempfile get_executable_resource_as_file(const int, const resource_type_e, const std::string&);
	// resource.h defined value like IDR_TTF1, id
	memfile get_executable_resource_as_memfile(const int, const WinString&);
	// resource.h defined value like IDR_TTF1, id as enum
	memfile get_executable_resource_as_memfile(const int, const resource_type_e);
#endif
}