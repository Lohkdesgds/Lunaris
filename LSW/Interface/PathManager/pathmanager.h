#pragma once

#include <string>
#include <functional>
#include <physfs.h>

// Others
#include "../../Handling/Abort/abort.h"
#include "../../Handling/Initialize/initialize.h"
#include "../../Handling/Path/path.h"
#include "../../Tools/Common/common.h"


namespace LSW {
	namespace v5 {
		namespace Interface {

			struct file_info {
				std::string path;
				int64_t size = 0;
				std::string size_str;
			};

			// Has to use / instead of \\

			/// <summary>
			/// <para>PathManager can change the way internal path works.</para>
			/// <para>You can set the thread's path relative to one or multiple compacted files using this.</para>
			/// <para>The paths unfortunalety are global.</para>
			/// <para>The effects are set per thread while this variable exists or while apply() is valid (before unapply()).</para>
			/// </summary>
			class PathManager {
				static bool physfs_initialized_once;

				std::vector<file_info> read_directory(const std::string& = "") const; // can call itself if folders in
			public:
				PathManager();
				~PathManager();
				
				/// <summary>
				/// <para>Add a new file (like a zip) or directory to relative path.</para>
				/// </summary>
				/// <param name="{std::string}">Path/File.</param>
				void add_path(std::string);
				
				/// <summary>
				/// <para>Remove once added path or file.</para>
				/// </summary>
				/// <param name="{std::string}">Path/File.</param>
				void del_path(const std::string);

				/// <summary>
				/// <para>Applies this settings to current thread.</para>
				/// </summary>
				void apply() const;
				
				/// <summary>
				/// <para>Cancel and go back to default way.</para>
				/// </summary>
				void unapply() const;

				/// <summary>
				/// <para>All paths set and ready.</para>
				/// </summary>
				/// <returns>{std::string} All paths set.</returns>
				std::vector<std::string> paths_set() const;
				
				/// <summary>
				/// <para>Shows all files in paths.</para>
				/// </summary>
				/// <returns></returns>
				std::vector<file_info> files_in_paths() const;
			};

		}
	}
}