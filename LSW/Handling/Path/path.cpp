#include "path.h"


namespace LSW {
	namespace v5 {
		namespace Handling {

			void create_path(const std::string& s)
			{
				if (s.find('/') == std::string::npos) return; // raw filename

				std::string str;
				std::stringstream ss(s);
				std::string token;
				std::vector<std::string> paths;

#ifdef _WIN32
				const char slash = '\\';
#else
				const char slash = '/';
#endif

				// Windows likes \\ to paths, / to URLs, so the whole stuff is /, but Windows calls will be \\.


				while (std::getline(ss, token, '/'))
				{
					str += token;
					paths.push_back(str);
					str += slash;
				}

				std::string u = paths.back();
				if (size_t h = u.rfind(slash); h != std::string::npos) {
					if (u.substr(h).rfind(".") != std::string::npos && s.back() != slash) paths.pop_back();
				}

				for (auto& i : paths)
				{
#ifndef _WIN32
					mkdir(i.c_str(), 0777);
#else
					CreateDirectoryA(i.c_str(), NULL);
#endif
				}
			}
			bool get_working_path(std::string& res, const std::string& path)
			{
				for (int u = 0; u < path::paths_count; u++) {
					if (path == path::paths_known[u]) {

						init_basic();
						ALLEGRO_PATH* path = al_get_standard_path(u);
						ALLEGRO_PATH* pathexe = al_get_standard_path(ALLEGRO_EXENAME_PATH);

						res = (path) ? al_path_cstr(path, '/') : "";
						al_destroy_path(path);

						std::string filename = al_get_path_filename(pathexe);
						al_destroy_path(pathexe);

						if (res.rfind(filename + '/') == (res.size() - filename.size() - 1)) 
							res.erase(res.end() - filename.size() - 1, res.end());

						if (res.rfind(filename) == (res.size() - filename.size())) 
							res.erase(res.end() - filename.size(), res.end());

						return !res.empty();
					}
				}
				return false;
			}


			void interpret_path(std::string& cpy)
			{
				if (cpy.find('%') == std::string::npos) return; // no % found, just skip

				for (auto& i : cpy) {
					//if (i == '/') i = '\\';
					if (i == '\\') i = '/';
				}

				std::string endresult;

				size_t min_pos = 0;
				while (1) {
					size_t pos_0 = 0;
					size_t pos_1 = 0;

					std::string wrk = cpy.substr(min_pos);

					pos_0 = wrk.find('%');
					if (pos_0 >= cpy.length()) break; // no more found (invalid pos)
					pos_0 += min_pos;

					pos_1 = cpy.substr(pos_0 + 1).find('%');
					if (pos_1 >= cpy.length()) break; // there is only one (invalid pos)
					pos_1 += pos_0 + 1;

					// if "\%", ignore this, insert \ to the % before that, continue.
					if (pos_1 > 0 && cpy[pos_1 - 1] == '\\') {
						//if (pos_0 > 0 && cpy[pos_0 - 1] != '\\') cpy.insert(cpy.begin() + pos_0, '\\'); // if enabled, 2 below
						endresult += cpy.substr(min_pos, pos_1 - min_pos + 1); // 1 because of ^
						min_pos = pos_1 + 1; // 1 because of ^^
						continue;
					}

					// so they are valid.

					endresult += cpy.substr(min_pos, pos_0 - min_pos);
					//printf_s("SS > %s\n", cpy.substr(min_pos, pos_0 - min_pos).c_str());

					std::string srch = cpy.substr(pos_0, pos_1 - pos_0 + 1); // this is %tag, not %tag%, so later it's easier to \%tag\% if needed

					std::string path_res;
					if (get_working_path(path_res, srch.c_str())) { // as said before
						endresult += path_res;
						//printf_s("YY > %s\n", path_res.c_str());
					}
					else {
						endresult += srch;
						//printf_s("NF > %s\n", srch.c_str());
					}

					min_pos = pos_1 + 1;
				}

				if (min_pos < cpy.length()) {
					std::string lil = cpy.substr(min_pos);
					endresult += lil;
				}
				//if (min_pos < cpy.length()) printf_s("PL > %s\n", cpy.substr(min_pos).c_str());

				cpy = std::move(endresult);
			}

			void handle_path(std::string& path)
			{
				interpret_path(path);
				create_path(path);
			}

			std::string get_app_path()
			{
				init_basic();
				ALLEGRO_PATH* path = al_get_standard_path(ALLEGRO_EXENAME_PATH);
				std::string res = (path) ? al_path_cstr(path, '/') : "";
				al_destroy_path(path);
				return res;
			}

		}
	}
}