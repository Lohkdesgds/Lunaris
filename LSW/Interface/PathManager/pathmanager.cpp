#include "pathmanager.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			bool PathManager::physfs_initialized_once = false;

			std::vector<file_info> PathManager::read_directory(const std::string& path) const
			{
				std::vector<file_info> files;
				char** rc = PHYSFS_enumerateFiles(path.empty() ? "" : path.c_str());
				for (auto j = rc; *j != NULL; j++) {
					PHYSFS_Stat fprop;
					std::string cpath = (path.empty() ? "" : (std::string(path) + "/")) + *j;
					PHYSFS_stat(cpath.c_str(), &fprop);

					switch (fprop.filetype) {
					case PHYSFS_FILETYPE_REGULAR: // file
						files.push_back({cpath, fprop.filesize, (Tools::byte_auto_string(fprop.filesize, 1, true) + "B")});
						//files += cpath + " [" + Tools::byte_auto_string(fprop.filesize, 1, true) + "B], ";
						break;
					case PHYSFS_FILETYPE_DIRECTORY: // path
						//files += read_directory(cpath.c_str());// +", ";
						for (auto& i : read_directory(cpath)) {
							files.push_back(i);
						}
						break;
					}
				}
				PHYSFS_freeList(rc);
				//if (files.size() > 1) for (short u = 0; u < 2; u++) files.pop_back();

				return files;
			}

			PathManager::PathManager()
			{
				Handling::init_basic();

				if (!physfs_initialized_once) {
					auto path = Handling::get_app_path();

					if (!PHYSFS_init(path.c_str())) throw Handling::Abort(__FUNCSIG__, "Failed to setup physfs!");
					al_set_physfs_file_interface();

					physfs_initialized_once = true;
				}
			}
			
			PathManager::~PathManager()
			{
				unapply();
			}
			
			void PathManager::add_path(std::string s)
			{
				Handling::interpret_path(s);
				if (PHYSFS_mount(s.c_str(), nullptr, 1) == 0) throw Handling::Abort(__FUNCSIG__, "Couldn't add " + s + " to search paths.", Handling::abort::abort_level::GIVEUP);
				//if (prunt) prunt(std::string("Added path '") + s.c_str() + "' successfully.");
			}

			void PathManager::del_path(const std::string s)
			{
				if (PHYSFS_unmount(s.c_str()) == 0) throw Handling::Abort(__FUNCSIG__, "Couldn't remove " + s + " from search paths.", Handling::abort::abort_level::GIVEUP);
				//if (prunt) prunt("Removed path '" + s + "' successfully.");
			}

			void PathManager::apply() const
			{
				al_set_physfs_file_interface();
				//if (prunt) prunt("Applied PathManager on current thread #" + std::to_string(Tools::get_thread_id()));
			}

			void PathManager::unapply() const
			{
				al_set_standard_file_interface();
				//if (prunt) prunt("Removed PathManager on current thread #" + std::to_string(Tools::get_thread_id()));
			}

			std::vector<std::string> PathManager::paths_set() const
			{
				//std::string raw_paths;
				std::vector<std::string> all_p;

				auto listpath = PHYSFS_getSearchPath();
				for (auto i = listpath; *i != NULL; i++) {
					//raw_paths += std::string(*i) + ", ";
					all_p.push_back(std::string(*i));
				}
				//for (size_t p = 0; p < 2 && all_p.size() > 0; p++) all_p.pop_back();
				//if (raw_paths.length() > 1) for (short u = 0; u < 2; u++) raw_paths.pop_back();
				PHYSFS_freeList(listpath);

				return all_p;
			}

			std::vector<file_info> PathManager::files_in_paths() const
			{
				return read_directory();
			}

		}
	}
}