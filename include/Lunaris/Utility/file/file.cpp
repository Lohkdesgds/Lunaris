#include "file.h"

namespace Lunaris {

	void __file_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
	}


	file::~file()
	{
		if (is_temp && !last_path.empty()) delete_and_close();
		else close();
	}

	file::file(file&& f) noexcept
		: fp(f.fp), last_path(f.last_path), is_temp(f.is_temp)
	{
		f.fp = nullptr;
		f.last_path.clear();
	}

	void file::operator=(file&& f) noexcept
	{
		close();
		fp = f.fp;
		last_path = f.last_path;
		is_temp = f.is_temp;
		f.fp = nullptr;
		f.last_path.clear();
		f.is_temp = false;
	}

	bool file::open(const std::string& path, const std::string& mode)
	{
		__file_allegro_start();
		close();
		is_temp = false;
		last_path = path;
		return (fp = al_fopen(path.c_str(), mode.c_str()));
	}

	bool file::open_temp(const std::string& templ, const std::string& mode)
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

	const std::string& file::get_current_path() const
	{
		return last_path;
	}

	void file::close()
	{
		if (fp) {
			al_fclose(fp);
			last_path.clear();
			fp = nullptr;
		}
	}

	size_t file::read(char* buf, const size_t len)
	{
		return al_fread(fp, buf, len);
	}

	size_t file::write(const char* buf, const size_t len)
	{
		for (size_t p = 0; p < len;) {
			p += al_fwrite(fp, buf + p, len - p);
		}
		return len;
	}

	size_t file::tell()
	{
		return static_cast<size_t>(al_ftell(fp));
	}

	bool file::seek(const int64_t i, const ALLEGRO_SEEK p)
	{
		return al_fseek(fp, i, p);
	}

	bool file::flush()
	{
		return al_fflush(fp);
	}

	size_t file::size() const
	{
		return al_fsize(fp);
	}

	void file::delete_and_close()
	{
		auto cpy = last_path;
		close();
		::remove(cpy.c_str());
	}

	file::operator const ALLEGRO_FILE* () const
	{
		return fp;
	}

}