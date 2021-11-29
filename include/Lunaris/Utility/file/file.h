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

	/// <summary>
	/// <para>file is a file interface using the Allegro's functions in a secure C++ way.</para>
	/// <para>You can create, load, edit and save files with this easily.</para>
	/// </summary>
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
		/// <param name="{bool}">Disable "destruction"?</param>
		void modify_no_destroy(const bool);
	public:
		file() = default;

		/// <summary>
		/// <para>Move a file handler to this.</para>
		/// </summary>
		/// <param name="{file}">The file being moved.</param>
		file(file&&) noexcept;

		/// <summary>
		/// <para>Move a file handler to this.</para>
		/// <para>If this had file, it is closed automatically.</para>
		/// </summary>
		/// <param name="{file}">The file being moved.</param>
		virtual void operator=(file&&) noexcept;

		/// <summary>
		/// <para>Closes the file handle.</para>
		/// </summary>
		virtual ~file();

		/// <summary>
		/// <para>Open a file.</para>
		/// <para>If this had file, it is closed automatically.</para>
		/// </summary>
		/// <param name="{std::string}">File path.</param>
		/// <param name="{open_mode_e}">What mode to open the file.</param>
		/// <returns>{bool} True if success.</returns>
		virtual bool open(const std::string&, const open_mode_e&);

		/// <summary>
		/// <para>Closes any file handle it may have.</para>
		/// </summary>
		virtual void close();

		/// <summary>
		/// <para>Get the path of the latest file it's opened.</para>
		/// </summary>
		/// <returns>{std::string} The internal string containing the file path.</returns>
		const std::string& get_path() const;

		/// <summary>
		/// <para>Get the file handler directly. DO NOT CLOSE THE FILE USING THIS!</para>
		/// </summary>
		/// <returns>{ALLEGRO_FILE*} The file handle.</returns>
		ALLEGRO_FILE* get_fp() const;

		/// <summary>
		/// <para>Gets the internal file handler via cast.</para>
		/// </summary>
		operator ALLEGRO_FILE*() const;

		/// <summary>
		/// <para>Read some bytes of the file (if the file was opened in read mode or equivalent).</para>
		/// </summary>
		/// <param name="{char*}">Buffer to write the data read.</param>
		/// <param name="{size_t}">The buffer size or the amount of bytes you want to read (max).</param>
		/// <returns>{size_t} How many bytes were really read.</returns>
		size_t read(char*, const size_t);

		/// <summary>
		/// <para>Write some bytes into the file (if the file was opened in write mode or equivalent).</para>
		/// </summary>
		/// <param name="{char*}">The buffer to write.</param>
		/// <param name="{size_t}">Buffer size.</param>
		/// <returns>{size_t} How many bytes were really written.</returns>
		size_t write(const char*, const size_t);

		/// <summary>
		/// <para>Where am I? Relative position compared to the beginning of the file, in bytes.</para>
		/// </summary>
		/// <returns>{size_t} Offset from start of the file</returns>
		size_t tell();

		/// <summary>
		/// <para>Go to a specific point relative or absolute in position.</para>
		/// </summary>
		/// <param name="{int64_t}">Where to go?</param>
		/// <param name="{seek_mode_e}">Relative to what?</param>
		/// <returns>{bool} True if success.</returns>
		bool seek(const int64_t, const seek_mode_e);

		/// <summary>
		/// <para>Flush cached stuff to the disk.</para>
		/// </summary>
		/// <returns>{bool} True if flushed successfully.</returns>
		bool flush();

		/// <summary>
		/// <para>Get the file size.</para>
		/// </summary>
		/// <returns>{size_t} File size, in bytes.</returns>
		size_t size() const;

		/// <summary>
		/// <para>Has any file open?</para>
		/// </summary>
		/// <returns>{bool} True if there's a file handle opened.</returns>
		bool is_open() const;

		/// <summary>
		/// <para>It is considered valid if any file handle is open.</para>
		/// </summary>
		/// <returns>{bool} True if handleable (file open)</returns>
		bool valid() const;

		/// <summary>
		/// <para>Same as "not open".</para>
		/// </summary>
		/// <returns>{bool} True if not opened</returns>
		bool empty() const;
	};

	/// <summary>
	/// <para>Behaves like a file, but the file is created in a temporary place.</para>
	/// <para>The file is deleted after destroyed/closed.</para>
	/// </summary>
	class tempfile : public file {
	public:
		tempfile() = default;

		/// <summary>
		/// <para>Move a tempfile handler to this.</para>
		/// </summary>
		/// <param name="{tempfile}">The tempfile being moved.</param>
		tempfile(tempfile&&) noexcept;

		/// <summary>
		/// <para>Move a tempfile handler to this.</para>
		/// <para>If this had file, it is closed and erased automatically.</para>
		/// </summary>
		/// <param name="{tempfile}">The tempfile being moved.</param>
		void operator=(tempfile&&) noexcept;

		/// <summary>
		/// <para>Closes the tempfile handle.</para>
		/// </summary>
		~tempfile();

		/// <summary>
		/// <para>Open a temporary file in the temporary path.</para>
		/// <para>Use 'X' for random character format, like 'lala_XXXXX.tmp' may generate 'lala_AbC1j.tmp'.</para>
		/// </summary>
		/// <param name="{std::string}">File name format.</param>
		/// <returns>{bool} True if good.</returns>
		bool open(const std::string&);

		/// <summary>
		/// <para>Closes the temporary file and deletes it. It is temporary, right?</para>
		/// </summary>
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
		using file::valid;
		using file::empty;
	};

	/// <summary>
	/// <para>Like a file, but the file itself is in memory.</para>
	/// <para>The downside is that you have to use fixed file size. It won't grow bigger than initial size.</para>
	/// <para>The size is defined by you.</para>
	/// </summary>
	class memfile : public file {
		std::unique_ptr<char[]> mem;
	public:
		memfile() = default;

		/// <summary>
		/// <para>Move a memfile handler to this.</para>
		/// </summary>
		/// <param name="{memfile}">The memfile being moved.</param>
		memfile(memfile&&) noexcept;

		/// <summary>
		/// <para>Move a memfile handler to this.</para>
		/// <para>If this had file, it is closed and freed automatically.</para>
		/// </summary>
		/// <param name="{memfile}">The memfile being moved.</param>
		void operator=(memfile&&) noexcept;

		/// <summary>
		/// <para>Closes the memfile handle.</para>
		/// </summary>
		~memfile();

		/// <summary>
		/// <para>Opens a memory file of that size (it's fixed in size).</para>
		/// </summary>
		/// <param name="{size_t}">The memory buffer size.</param>
		/// <returns>{bool} True if success.</returns>
		bool open(const size_t);

		/// <summary>
		/// <para>Close and free memory.</para>
		/// </summary>
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
		using file::valid;
		using file::empty;
	};

#ifdef _WIN32 // && _MSC_VER

	/// <summary>
	/// <para>Get a Visual Studio resource as a tempfile.</para>
	/// </summary>
	/// <param name="{int}">The ID of the resource, found in a header like 'resource.h' like IDR_TTF1.</param>
	/// <param name="{WinString}">A file type in string (if you're on unicode, this is WCHAR_T*, else probably CHAR*, and it may be "PNG", "TTF" or something like that).</param>
	/// <param name="{std::string}">Custom format, also known as "what extension should the file have?". Examples: ".jpg", ".png", ".tmp", ...</param>
	/// <returns>{tempfile} A tempfile with or without the content. Please check size != 0.</returns>
	tempfile get_executable_resource_as_file(const int, const WinString&, const std::string&);

	/// <summary>
	/// <para>Get a Visual Studio resource as a tempfile.</para>
	/// </summary>
	/// <param name="{int}">The ID of the resource, found in a header like 'resource.h' like IDR_TTF1.</param>
	/// <param name="{resource_type_e}">Some types are defined as specific values instead of string. You should check the output anyways.</param>
	/// <param name="{std::string}">Custom format, also known as "what extension should the file have?". Examples: ".jpg", ".png", ".tmp", ...</param>
	/// <returns>{tempfile} A tempfile with or without the content. Please check size != 0.</returns>
	tempfile get_executable_resource_as_file(const int, const resource_type_e, const std::string&);

	/// <summary>
	/// <para>Get a Visual Studio resource as a memfile.</para>
	/// </summary>
	/// <param name="{int}">The ID of the resource, found in a header like 'resource.h' like IDR_TTF1.</param>
	/// <param name="{WinString}">A file type in string (if you're on unicode, this is WCHAR_T*, else probably CHAR*, and it may be "PNG", "TTF" or something like that).</param>
	/// <returns>{memfile} A memfile with or without the content. Please check size != 0.</returns>
	memfile get_executable_resource_as_memfile(const int, const WinString&);

	/// <summary>
	/// <para>Get a Visual Studio resource as a memfile.</para>
	/// </summary>
	/// <param name="{int}">The ID of the resource, found in a header like 'resource.h' like IDR_TTF1.</param>
	/// <param name="{resource_type_e}">Some types are defined as specific values instead of string. You should check the output anyways.</param>
	/// <returns>{memfile} A memfile with or without the content. Please check size != 0.</returns>
	memfile get_executable_resource_as_memfile(const int, const resource_type_e);

#endif
}