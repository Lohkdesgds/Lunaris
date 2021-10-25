#include "file.h"

namespace Lunaris {

	LUNARIS_DECL void __file_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
	}

	LUNARIS_DECL std::string file::transl(const file::open_mode_e& mod)
	{
		switch (mod) {
		case file::open_mode_e::READ_TRY:
			return "rb";
		case file::open_mode_e::WRITE_REPLACE:
			return "wb";
		case file::open_mode_e::READWRITE_REPLACE:
			return "wb+";
		case file::open_mode_e::READWRITE_KEEP:
			return "ab+";
		}
		return "";
	}

	LUNARIS_DECL file::file(file&& oth)
		: fp(std::move(oth.fp)), path(std::move(oth.path))
	{
	}

	LUNARIS_DECL void file::operator=(file&& oth) noexcept
	{
		fp = std::move(oth.fp);
		path = std::move(oth.path);
	}

	LUNARIS_DECL file::~file()
	{
		close();
	}

	LUNARIS_DECL bool file::open(const std::string& pth, const open_mode_e& mode)
	{
		__file_allegro_start();
		close();

		const std::string mode_transl = transl(mode);

		this->path = pth;
		this->fp = std::unique_ptr<ALLEGRO_FILE, bool(*)(ALLEGRO_FILE*)>(al_fopen(pth.c_str(), mode_transl.c_str()), al_fclose);

		seek(0, seek_mode_e::BEGIN);

		return this->fp != nullptr;
	}

	LUNARIS_DECL void file::close()
	{
		if (fp) {
			fp.reset();
			path.clear();
		}
	}
#ifdef LUNARIS_ALPHA_TESTING
	LUNARIS_DECL void file::modify_no_destroy(const bool nodestr)
	{
		ALLEGRO_FILE* tmpfp = this->fp.release();
		if (!nodestr)   this->fp = std::unique_ptr<ALLEGRO_FILE, bool(*)(ALLEGRO_FILE*)>(tmpfp, al_fclose);
		else	        this->fp = std::unique_ptr<ALLEGRO_FILE, bool(*)(ALLEGRO_FILE*)>(tmpfp, [](ALLEGRO_FILE*) {return true; });
	}
#endif
	LUNARIS_DECL const std::string& file::get_path()
	{
		return path;
	}

	LUNARIS_DECL ALLEGRO_FILE* file::get_fp() const
	{
		return fp.get();
	}

	LUNARIS_DECL file::operator ALLEGRO_FILE* () const
	{
		return fp.get();
	}

	LUNARIS_DECL size_t file::read(char* buf, const size_t len)
	{
		return fp ? al_fread(fp.get(), buf, len) : 0;
	}

	LUNARIS_DECL size_t file::write(const char* buf, const size_t len)
	{
		return fp ? al_fwrite(fp.get(), buf, len) : 0;
	}

	LUNARIS_DECL size_t file::tell()
	{
		return fp ? al_ftell(fp.get()) : 0;
	}

	LUNARIS_DECL bool file::seek(const int64_t off, const seek_mode_e skk)
	{
		return fp ? al_fseek(fp.get(), off, static_cast<int>(skk)) : false;
	}

	LUNARIS_DECL bool file::flush()
	{
		return fp ? al_fflush(fp.get()) : false;
	}

	LUNARIS_DECL size_t file::size() const
	{
		return fp ? al_fsize(fp.get()) : 0;
	}

	LUNARIS_DECL tempfile::tempfile(tempfile&& oth) noexcept
		: file(std::move(oth))
	{
	}

	LUNARIS_DECL void tempfile::operator=(tempfile&& oth) noexcept
	{
		close();

		this->file::operator=(std::move(oth));

		oth.fp = nullptr;
	}

	LUNARIS_DECL tempfile::~tempfile()
	{
		close();
	}

	LUNARIS_DECL bool tempfile::open(const std::string& templ)
	{
		__file_allegro_start();
		close();

		ALLEGRO_PATH* var = al_create_path(nullptr);
		ALLEGRO_FILE* tmpfp = nullptr;
		if (!(tmpfp = al_make_temp_file(templ.c_str(), &var))) {
			al_destroy_path(var);
			return false;
		}
		
		this->path = al_path_cstr(var, '/');//ALLEGRO_NATIVE_PATH_SEP

		al_destroy_path(var);
		al_fclose(tmpfp);

		this->fp = std::unique_ptr<ALLEGRO_FILE, bool(*)(ALLEGRO_FILE*)>(al_fopen(path.c_str(), "wb+"), al_fclose);

		seek(0, seek_mode_e::BEGIN);

		return this->fp != nullptr;
	}

	LUNARIS_DECL void tempfile::close()
	{
		if (fp) fp.reset();

		if (!path.empty()) {
			::remove(path.c_str());
			path.clear();
		}
	}

	LUNARIS_DECL memfile::memfile(memfile&& oth) noexcept
		: file(std::move(oth)), mem(std::move(oth.mem))
	{
	}

	LUNARIS_DECL void memfile::operator=(memfile&& oth) noexcept
	{
		close();

		this->file::operator=(std::move(oth));
		this->mem = std::move(oth.mem);

		oth.fp = nullptr;
	}

	LUNARIS_DECL memfile::~memfile()
	{
		close();
	}

	LUNARIS_DECL bool memfile::open(const size_t len)
	{
		__file_allegro_start();
		close();

		this->mem = std::unique_ptr<char[]>(new char[len]);
		this->fp = std::unique_ptr<ALLEGRO_FILE, bool(*)(ALLEGRO_FILE*)>(al_open_memfile(mem.get(), len, "wb+"), al_fclose);

		seek(0, seek_mode_e::BEGIN);

		return this->fp != nullptr;
	}

	LUNARIS_DECL void memfile::close()
	{
		this->file::close();
		if (mem) mem.reset();
	}


#if (_MSC_VER && _WIN32)
	LUNARIS_DECL tempfile __get_executable_resource_as_file(const int id, const WinString type, const std::string& extn)
	{
		HRSRC src = FindResource(NULL, MAKEINTRESOURCE(id), type);
		if (src != NULL) {
			unsigned int myResourceSize = ::SizeofResource(NULL, src);
			HGLOBAL myResourceData = LoadResource(NULL, src);

			if (myResourceData != NULL) {
				void* pMyBinaryData = LockResource(myResourceData);

				tempfile fp;
				if (!fp.open("theblast_temp_XXXX" + extn)) {
					FreeResource(myResourceData);
					return {};
				}
				fp.write((char*)pMyBinaryData, myResourceSize);
				fp.flush();

				FreeResource(myResourceData);

				return fp;
			}
		}
		return {};
	}

	LUNARIS_DECL tempfile get_executable_resource_as_file(const int id, const resource_type_e type, const std::string& extn)
	{
		return __get_executable_resource_as_file(id, (WinString)((ULONG_PTR)((WORD)(type))), extn);
	}

	LUNARIS_DECL memfile __get_executable_resource_as_memfile(const int id, const WinString type)
	{
		HRSRC src = FindResource(NULL, MAKEINTRESOURCE(id), type);
		if (src != NULL) {
			unsigned int myResourceSize = ::SizeofResource(NULL, src);
			HGLOBAL myResourceData = LoadResource(NULL, src);

			if (myResourceData != NULL) {
				void* pMyBinaryData = LockResource(myResourceData);

				memfile fp;
				if (!fp.open(myResourceSize)) {
					FreeResource(myResourceData);
					return {};
				}
				fp.write((char*)pMyBinaryData, myResourceSize);
				fp.flush();

				FreeResource(myResourceData);

				return fp;
			}
		}
		return {};
	}

	LUNARIS_DECL memfile get_executable_resource_as_memfile(const int id, const resource_type_e type)
	{
		return __get_executable_resource_as_memfile(id, (WinString)((ULONG_PTR)((WORD)(type))));
	}
#endif
}