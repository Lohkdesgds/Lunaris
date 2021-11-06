#pragma once

#include <Lunaris/__macro/macros.h>

#include <allegro5/allegro5.h>
#include <string>
#include <filesystem>
#include <fstream>

namespace Lunaris {

	enum class path_sep_e {NATIVE, FORWARD, BACKWARD};

	void __path_allegro_start();

	/// <summary>
	/// <para>Translate this to char.</para>
	/// </summary>
	/// <param name="{path_sepr_e}">The enum value</param>
	/// <returns>{char} Translated enum to char</returns>
	char sep_path_to_char(const path_sep_e);

	/// <summary>
	/// <para>Used in paths, setting app name will automatically generate names based on this on get_standard_path.</para>
	/// </summary>
	/// <param name="{std::string}">App name.</param>
	void set_app_name(const std::string&);

	/// <summary>
	/// <para>Get app name you've set or the one Allegro did for you.</para>
	/// </summary>
	/// <returns>{std::string} App name.</returns>
	std::string get_app_name();

	/// <summary>
	/// <para>If you just want to type like get_standard_path() + "blah/whatever\\I want like that/lol" and make it work, use this.</para>
	/// </summary>
	/// <param name="{std::string}">Your weird maybe malformed path.</param>
	/// <param name="{path_sep_e}">Final format you want.</param>
	/// <returns>{std::string} Fixed format</returns>
	std::string fix_path_to(std::string, const path_sep_e = path_sep_e::NATIVE);
	
	/// <summary>
	/// <para>Get standard path using Allegro's path system.</para>
	/// </summary>
	/// <param name="{int}">One of ALLEGRO_**_PATH (defaults to ALLEGRO_USER_DATA_PATH).</param>
	/// <param name="{path_sep_e}">What slash style? Native = default to system.</param>
	/// <returns>{std::string} Final path if success, else empty.</returns>
	std::string get_standard_path(int = ALLEGRO_USER_DATA_PATH, const path_sep_e = path_sep_e::NATIVE);

	/// <summary>
	/// <para>Tries to make this path valid.</para>
	/// <para>Expects true path to end with / or \\ like 'C:/path/'. Paths without that will consider the end as file name.</para>
	/// </summary>
	/// <param name="{std::string}">Path.</param>
	/// <param name="{path_sep_e}">The path mode you are using.</param>
	/// <param name="{bool}">Call fix_path_to just to be sure things will happen as expected?</param>
	/// <returns>{bool} True if success</returns>
	bool make_path(std::string, const path_sep_e = path_sep_e::NATIVE, const bool = true);

	/// <summary>
	/// <para>Removes this path (completely) or file.</para>
	/// </summary>
	/// <param name="{std::string}">Path itself.</param>
	/// <param name="{bool}">Call fix_path_to just to be sure things will happen as expected?</param>
	/// <returns>{size_t} How many items were deleted, or 0 if none, or static_cast&lt;size_t&gt;(-1) if error.</returns>
	size_t remove_all(std::string, const bool = true);
}