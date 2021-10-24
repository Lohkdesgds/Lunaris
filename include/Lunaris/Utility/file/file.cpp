#include "file.h"

namespace Lunaris {

	LUNARIS_DECL void __file_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
	}


	LUNARIS_DECL file::~file()
	{
		if (is_temp && !last_path.empty()) delete_and_close();
		else close();
	}

	LUNARIS_DECL file::file(file&& f) noexcept
		: fp(f.fp), last_path(f.last_path), is_temp(f.is_temp)
	{
		f.fp = nullptr;
		f.last_path.clear();
	}

	LUNARIS_DECL void file::operator=(file&& f) noexcept
	{
		close();
		fp = f.fp;
		last_path = f.last_path;
		is_temp = f.is_temp;
		f.fp = nullptr;
		f.last_path.clear();
		f.is_temp = false;
	}

	LUNARIS_DECL bool file::open(const std::string& path, const std::string& mode)
	{
		__file_allegro_start();
		close();
		is_temp = false;
		last_path = path;
		return (fp = al_fopen(path.c_str(), mode.c_str()));
	}

	LUNARIS_DECL bool file::open_temp(const std::string& templ, const std::string& mode)
	{
		__file_allegro_start();
		close();
		is_temp = true;

		ALLEGRO_PATH* var = al_create_path(nullptr);
		if (!(fp = al_make_temp_file(templ.c_str(), &var))) {
			al_destroy_path(var);
			return false;
		}
		last_path = al_path_cstr(var, '/');//ALLEGRO_NATIVE_PATH_SEP
		al_destroy_path(var);

		// reset and guarantee binary write/read
		al_fclose(fp);

		if (!(fp = al_fopen(last_path.c_str(), mode.c_str()))) {
			::remove(last_path.c_str()); // no junk
			return false;
		}

		return true;
	}

	LUNARIS_DECL const std::string& file::get_current_path() const
	{
		return last_path;
	}

	LUNARIS_DECL void file::close()
	{
		if (fp) {
			al_fclose(fp);
			last_path.clear();
			fp = nullptr;
		}
	}

	LUNARIS_DECL size_t file::read(char* buf, const size_t len)
	{
		return al_fread(fp, buf, len);
	}

	LUNARIS_DECL size_t file::write(const char* buf, const size_t len)
	{
		for (size_t p = 0; p < len;) {
			p += al_fwrite(fp, buf + p, len - p);
		}
		return len;
	}

	LUNARIS_DECL size_t file::tell()
	{
		return static_cast<size_t>(al_ftell(fp));
	}

	LUNARIS_DECL bool file::seek(const int64_t i, const ALLEGRO_SEEK p)
	{
		return al_fseek(fp, i, p);
	}

	LUNARIS_DECL bool file::flush()
	{
		return al_fflush(fp);
	}

	LUNARIS_DECL size_t file::size() const
	{
		return al_fsize(fp);
	}

	LUNARIS_DECL void file::delete_and_close()
	{
		auto cpy = last_path;
		close();
		::remove(cpy.c_str());
	}

	LUNARIS_DECL file::operator const ALLEGRO_FILE* () const
	{
		return fp;
	}

#if (_MSC_VER && _WIN32)
	LUNARIS_DECL file get_executable_resource_as_file(const int id, const WINSTRING type, const std::string& extn)
	{
		HRSRC src = FindResource(NULL, MAKEINTRESOURCE(id), type);
		if (src != NULL) {
			unsigned int myResourceSize = ::SizeofResource(NULL, src);
			HGLOBAL myResourceData = LoadResource(NULL, src);

			if (myResourceData != NULL) {
				void* pMyBinaryData = LockResource(myResourceData);

				file fp;
				if (!fp.open_temp("theblast_temp_XXXX" + extn, "wb+")) {
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
#endif
}