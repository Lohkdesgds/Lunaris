#pragma once

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#endif

#include <string>
#include <vector>
#include <sstream>
#include "..\Initialize\initialize.h"

namespace LSW {
	namespace v5 {
		namespace Handling {

			/*		

	ALLEGRO_RESOURCES_PATH // 'F:\Cloud Storage\OneDrive\Organized\Programming\Projects\LSW_devtest\x64\Debug\'
	ALLEGRO_TEMP_PATH // 'C:\Users\ernes\AppData\Local\Temp\'
	ALLEGRO_USER_DATA_PATH // 'C:\Users\ernes\AppData\Roaming\LSW.exe\'
	ALLEGRO_USER_HOME_PATH // 'C:\Users\ernes\'
	ALLEGRO_USER_SETTINGS_PATH // 'C:\Users\ernes\AppData\Roaming\LSW.exe\'
	ALLEGRO_USER_DOCUMENTS_PATH // 'F:\Cloud Storage\OneDrive\Documentos\'
	ALLEGRO_EXENAME_PATH); // 'F:\Cloud Storage\OneDrive\Organized\Programming\Projects\LSW_devtest\x64\Debug\LSW.exe'
			*/

			namespace path {
				constexpr size_t paths_count = ALLEGRO_LAST_PATH;
				inline const std::string paths_known[paths_count] = { "%resources%", "%temp%", "%appdata%", "%home%", "%settings%", "%documents%", "%exepath%" };
				// ID is from 0 to ALLEGRO_LAST_PATH-1
			}
			/// <summary>
			/// <para>Creates the path automatically (use /)</para>
			/// <para>It will ignore files (filename.extension format). If you want literally as path, do like "C:/folder/other.folder.with.dots/"</para>
			/// </summary>
			/// <param name="{std::string}">The path to be created (can be relative or full. It's recommended to do full or use %paths%)</param>
			void create_path(const std::string&);

			/// <summary>
			/// <para>Gets folder path based on string (one of those path::paths_known or environment).</para>
			/// </summary>
			/// <param name="{std::string}">Sets this if path was found.</param>
			/// <param name="{std::string}">The %path% string like "%appdata%".</param>
			/// <returns>{bool} Success</returns>
			bool get_working_path(std::string&, const std::string& = path::paths_known[0]);

			/// <summary>
			/// <para>Interprets path (ignores stuff like %appdata\%, allows \%appdata% because that can be a path)</para>
			/// <para>+ filters path with \\ (to / instead)</para>
			/// </summary>
			/// <param name="{std::string}">The input and output, like "%appdata%/cool_path_i_made/file.exe"</param>
			void interpret_path(std::string&);

			/// <summary>
			/// <para>Interprets and solves %placeholders% (interpret_path) and guarantees path (create_path)</para>
			/// </summary>
			/// <param name="{std::string}">The path you want to create and interpret (and save interpretation)</param>
			void handle_path(std::string&);

			/// <summary>
			/// <para>Gets the executable path.</para>
			/// </summary>
			/// <returns>{std::string} Full path.</returns>
			std::string get_app_path();

		}
	}
}