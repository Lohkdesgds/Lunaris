#include "SmartFile.h"


namespace LSW {
	namespace v5 {
		namespace Interface {

			std::string SmartFile::convert(const smartfile::file_modes m)
			{
				switch (m) {
				case smartfile::file_modes::READ:
					return "rb";
				case smartfile::file_modes::WRITE:
					return "wb";
				case smartfile::file_modes::READ_WRITE_KEEP:
					return "rb+";
				case smartfile::file_modes::READ_WRITE_OVERWRITE:
					return "wb+";
				case smartfile::file_modes::APPEND_WRITE:
					return "ab";
				case smartfile::file_modes::APPEND_READ_WRITE:
					return "ab+";
				}
				return "rb+";
			}

			SmartFile::SmartFile()
			{
				Handling::init_basic();
			}
			
			SmartFile::~SmartFile()
			{
				close();
			}

			SmartFile::SmartFile(SmartFile&& u)
			{
				*this = std::move(u); // operator=
			}

			void SmartFile::operator=(SmartFile&& u)
			{
				fp = u.fp;
				was_temp_file = u.was_temp_file;
				eoff = u.eoff;
				last_path = std::move(u.last_path);
				latest_mode = u.latest_mode;

				u.fp = nullptr;
				u.was_temp_file = false;
				u.eoff = false;
				u.last_path.clear(); // just to be sure
				u.latest_mode = {}; // default
			}
			
			bool SmartFile::open(std::string path, const smartfile::file_modes m)
			{
				eoff = false;
				close();
				Handling::interpret_path(path);

				latest_mode = m;
				last_path = path;

				return (fp = al_fopen(path.c_str(), convert(m).c_str()));
			}

			bool SmartFile::open_temp(const smartfile::file_modes m)
			{
				eoff = false;
				close();

				auto pathtemp = std::unique_ptr<ALLEGRO_PATH, std::function<void(ALLEGRO_PATH*)>>(al_create_path(nullptr), [](ALLEGRO_PATH* t) { al_destroy_path(t); });
				ALLEGRO_PATH* var = pathtemp.get();
				if (!(fp = al_make_temp_file("XXXXXXXXXXXXXXX.lsw_temp", &var))) {
					return false;
				}
				last_path = al_path_cstr(var, ALLEGRO_NATIVE_PATH_SEP);

				// reset and guarantee binary write/read
				al_fclose(fp);

				if (!(fp = al_fopen(last_path.c_str(), convert(m).c_str()))) {
					::remove(last_path.c_str()); // no junk
					return false;
				}

				latest_mode = m;
				was_temp_file = true;
				
				return true;
			}

			bool SmartFile::is_open() const
			{
				return fp;
			}

			smartfile::file_modes SmartFile::open_mode() const
			{
				return latest_mode;
			}

			bool SmartFile::is_readable() const
			{
				return fp && (
					latest_mode == smartfile::file_modes::APPEND_READ_WRITE ||
					latest_mode == smartfile::file_modes::READ ||
					latest_mode == smartfile::file_modes::READ_WRITE_KEEP ||
					latest_mode == smartfile::file_modes::READ_WRITE_OVERWRITE);
			}

			bool SmartFile::is_writable() const
			{
				return fp && (
					latest_mode == smartfile::file_modes::APPEND_READ_WRITE ||
					latest_mode == smartfile::file_modes::APPEND_WRITE ||
					latest_mode == smartfile::file_modes::READ_WRITE_KEEP ||
					latest_mode == smartfile::file_modes::READ_WRITE_OVERWRITE ||
					latest_mode == smartfile::file_modes::WRITE);
			}

			std::string SmartFile::path() const
			{
				return is_open() ? last_path : "";
			}

			int64_t SmartFile::total_size() const
			{
				int64_t size_of_this = 0;
				if (fp) {
					auto b44 = al_ftell(fp);
					al_fseek(fp, 0, ALLEGRO_SEEK_END);
					size_of_this = al_ftell(fp);
					al_fseek(fp, b44, ALLEGRO_SEEK_SET);
				}
				return size_of_this;
			}

			void SmartFile::close()
			{
				if (fp) {
					al_fclose(fp);
					eoff = true;
					fp = nullptr;
				}
				if (was_temp_file && !last_path.empty()) {
					::remove(last_path.c_str());
					was_temp_file = false;
					last_path.clear();
				}
				last_path.clear();
			}

			bool SmartFile::eof() const
			{
				return eoff;
			}

			void SmartFile::seek(const int64_t off, const smartfile::file_seek point)
			{
				al_fseek(fp, off, static_cast<ALLEGRO_SEEK>(point));
				eoff = al_feof(fp);
			}

			size_t SmartFile::read(std::string& buf, const size_t siz)
			{
				if (!fp) { return 0; }
				if (al_feof(fp)) { eoff = true; return 0; }

				buf.resize(siz); // set maximum size

				auto readd = al_fread(fp, buf.data(), siz);
				if (readd <= 0) {
					eoff = true;
					buf.clear();
					return 0;
				}
				buf.resize(readd); // set what is there
				buf.shrink_to_fit(); // clean up the rest of it (won't clear after it)
				return readd;
			}

			size_t SmartFile::read_until(std::string& buf, const char stop)
			{
				if (!fp) { return 0; }
				if (al_feof(fp)) { eoff = true; return 0; }

				buf.clear();

				size_t readd = 0;

				for (char ch = stop - 1; ch != stop && !(eoff = al_feof(fp));) {
					int cc = al_fgetc(fp);
					if (cc < 0) {
						eoff = true;
						break;
					}
					ch = static_cast<char>(cc);
					buf += ch;
					readd++;
				}

				return readd;
			}

			size_t SmartFile::write(const std::string& buf, const size_t s)
			{
				if (!fp) return 0;
				return al_fwrite(fp, buf.data(), s ? s : buf.size());
			}

			int64_t quick_get_file_size(std::string p) {
				SmartFile f;
				if (!f.open(p, smartfile::file_modes::READ)) return -1;
				return f.total_size(); // close on destroy
			}

		}
	}
}
