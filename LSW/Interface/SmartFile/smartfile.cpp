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
			
			bool SmartFile::open(std::string path, const smartfile::file_modes m)
			{
				eoff = false;
				close();
				Handling::interpret_path(path);

				latest_mode = m;

				return (fp = al_fopen(path.c_str(), convert(m).c_str()));
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
