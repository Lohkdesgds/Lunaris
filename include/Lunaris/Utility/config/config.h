#pragma once

#include <Lunaris/__macro/macros.h>

#include <allegro5/allegro5.h>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

namespace Lunaris {

	void __config_allegro_start();

	/// <summary>
	/// <para>Config handles both saved configurations and temporary (while running) settings.</para>
	/// <para>The type of data is set by section.</para>
	/// </summary>
	class config {
	public:
		enum class config_section_mode { SAVE, MEMORY_ONLY };
	private:
		struct section {
			config_section_mode mode;
			std::string section_name;
			std::string section_comment;
			std::vector<std::pair<std::string, std::string>> key_and_value;
		};

		std::string path; // save path
		bool autosave = true;
		std::vector<section> conff;

		// section itself, key and value. Won't change if empty (one of them). Adds if not existant
		void _set(section&, const std::string&, const std::string&);
		// by section id. if not found, create section. Mode is used if it has to create a new one.
		section& _get_to_set(const std::string&, const config_section_mode = config_section_mode::SAVE);
		// by section id. if not found, throw
		const section& _get(const std::string&) const;
		// checks existance of a section
		bool _has(const std::string&) const;
	public:
		config() = default;
		config(const config&) = delete;

		/// <summary>
		/// <para>Move constructor.</para>
		/// </summary>
		/// <param name="{Config}">Config to move from.</param>
		config(config&&) noexcept;
		~config();

		/// <summary>
		/// <para>Sets if it should save when destroyed or not (defaults: true).</para>
		/// </summary>
		/// <param name="{bool}">Autosave?</param>
		void auto_save(bool);

		/// <summary>
		/// <para>Loads a config file (automatically will save to this path later).</para>
		/// </summary>
		/// <param name="{std::string}">Path.</param>
		/// <returns>{bool} True if success.</returns>
		bool load(std::string);

		/// <summary>
		/// <para>Sets a path to save at.</para>
		/// </summary>
		/// <param name="{std::string}">Path.</param>
		void save_path(std::string);

		/// <summary>
		/// <para>Flushes the config to file.</para>
		/// </summary>
		/// <returns>{bool} True if success.</returns>
		bool flush();

		/// <summary>
		/// <para>Change a existing section's mode.</para>
		/// </summary>
		/// <param name="{std::string}">Section name.</param>
		/// <param name="{config_section_mode}">The new mode.</param>
		void set(const std::string&, const config_section_mode);

		/// <summary>
		/// <para>Is there a section with this setting set?</para>
		/// </summary>
		/// <param name="{std::string}">Section name.</param>
		/// <param name="{config_section_mode}">Mode it has to be.</param>
		/// <returns>{bool} True if it does.</returns>
		bool has(const std::string&, const config_section_mode) const;

		/// <summary>
		/// <para>Is there a section with this key and setting set?</para>
		/// </summary>
		/// <param name="{std::string}">Section name.</param>
		/// <param name="{std::string}">Key name.</param>
		/// <param name="{config_section_mode}">Mode it has to be.</param>
		/// <returns>{bool} True if it does.</returns>
		bool has(const std::string&, const std::string&, const config_section_mode) const;

		/// <summary>
		/// <para>Is there a section with this setting set?</para>
		/// </summary>
		/// <param name="{std::string}">Section name.</param>
		/// <returns>{bool} True if it does.</returns>
		bool has(const std::string&) const;

		/// <summary>
		/// <para>Is there a section with this key and setting set?</para>
		/// </summary>
		/// <param name="{std::string}">Section name.</param>
		/// <param name="{std::string}">Key name.</param>
		/// <returns>{bool} True if it does.</returns>
		bool has(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Ensures if value is not present, your default value is set.</para>
		/// </summary>
		/// <param name="{std::string}">Section name.</param>
		/// <param name="{std::string}">Key name.</param>
		/// <param name="{T}">A default value.</param>
		/// <param name="{config_section_mode}">Mode it has to be.</param>
		template <typename T>
		void ensure(const std::string&, const std::string&, const T&, const config_section_mode);

		/// <summary>
		/// <para>Ensures if value is not present, your default value is set.</para>
		/// </summary>
		/// <param name="{std::string}">Section name.</param>
		/// <param name="{std::string}">Key name.</param>
		/// <param name="{std::initializer_list}">A default list-like value.</param>
		/// <param name="{config_section_mode}">Mode it has to be.</param>
		template <typename T>
		void ensure(const std::string&, const std::string&, const std::initializer_list<T>&, const config_section_mode);

		/// <summary>
		/// <para>Ensures if value is not present, your default value is set.</para>
		/// </summary>
		/// <param name="{std::string}">Section name.</param>
		/// <param name="{std::string}">Key name.</param>
		/// <param name="{std::vector}">A default list-like value.</param>
		/// <param name="{config_section_mode}">Mode it has to be.</param>
		template <typename T>
		void ensure(const std::string&, const std::string&, const std::vector<T>&, const config_section_mode);

		/// <summary>
		/// <para>Set a config value.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::string}">Value.</param>
		void set(const std::string&, const std::string&, const std::string&);

		/// <summary>
		/// <para>Set a config value.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{bool}">Value.</param>
		void set(const std::string&, const std::string&, const bool&);

		/// <summary>
		/// <para>Set a config value.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{char}">Value.</param>
		void set(const std::string&, const std::string&, const char&);

		/// <summary>
		/// <para>Set a config value.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{int}">Value.</param>
		void set(const std::string&, const std::string&, const int&);

		/// <summary>
		/// <para>Set a config value.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{float}">Value.</param>
		void set(const std::string&, const std::string&, const float&);

		/// <summary>
		/// <para>Set a config value.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{double}">Value.</param>
		void set(const std::string&, const std::string&, const double&);

		/// <summary>
		/// <para>Set a config value.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{unsigned}">Value.</param>
		void set(const std::string&, const std::string&, const unsigned&);

		/// <summary>
		/// <para>Set a config value.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{long}">Value.</param>
		void set(const std::string&, const std::string&, const long&);

		/// <summary>
		/// <para>Set a config value.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{long long}">Value.</param>
		void set(const std::string&, const std::string&, const long long&);

		/// <summary>
		/// <para>Set a config value.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{unsigned long long}">Value.</param>
		void set(const std::string&, const std::string&, const unsigned long long&);


		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::initializer_list}">Values.</param>
		void set(const std::string&, const std::string&, const std::initializer_list<std::string>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::initializer_list}">Values.</param>
		void set(const std::string&, const std::string&, const std::initializer_list<bool>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::initializer_list}">Values.</param>
		void set(const std::string&, const std::string&, const std::initializer_list<char>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::initializer_list}">Values.</param>
		void set(const std::string&, const std::string&, const std::initializer_list<int>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::initializer_list}">Values.</param>
		void set(const std::string&, const std::string&, const std::initializer_list<float>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::initializer_list}">Values.</param>
		void set(const std::string&, const std::string&, const std::initializer_list<double>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::initializer_list}">Values.</param>
		void set(const std::string&, const std::string&, const std::initializer_list<unsigned>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::initializer_list}">Values.</param>
		void set(const std::string&, const std::string&, const std::initializer_list<long>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::initializer_list}">Values.</param>
		void set(const std::string&, const std::string&, const std::initializer_list<long long>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::initializer_list}">Values.</param>
		void set(const std::string&, const std::string&, const std::initializer_list<unsigned long long>&);


		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::vector}">Values.</param>
		void set(const std::string&, const std::string&, const std::vector<std::string>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::vector}">Values.</param>
		void set(const std::string&, const std::string&, const std::vector<bool>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::vector}">Values.</param>
		void set(const std::string&, const std::string&, const std::vector<char>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::vector}">Values.</param>
		void set(const std::string&, const std::string&, const std::vector<int>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::vector}">Values.</param>
		void set(const std::string&, const std::string&, const std::vector<float>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::vector}">Values.</param>
		void set(const std::string&, const std::string&, const std::vector<double>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::vector}">Values.</param>
		void set(const std::string&, const std::string&, const std::vector<unsigned>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::vector}">Values.</param>
		void set(const std::string&, const std::string&, const std::vector<long>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::vector}">Values.</param>
		void set(const std::string&, const std::string&, const std::vector<long long>&);

		/// <summary>
		/// <para>Set values into a config key at once.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <param name="{std::vector}">Values.</param>
		void set(const std::string&, const std::string&, const std::vector<unsigned long long>&);

		/// <summary>
		/// <para>Adds a comment to the config file.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Comment.</param>
		void comment(const std::string&, std::string);

		/// <summary>
		/// <para>Get a value from config.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{std::string} Value.</returns>
		std::string get(const std::string&, const std::string&) const;

		template<typename T> T get_as(const std::string&, const std::string&) const = delete;

		/// <summary>
		/// <para>Get a value from config as a type T.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{bool} Value.</returns>
		template<> bool get_as(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a value from config as a type T.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{char} Value.</returns>
		template<> char get_as(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a value from config as a type T.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{int32_t} Value.</returns>
		template<> int32_t get_as(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a value from config as a type T.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{float} Value.</returns>
		template<> float get_as(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a value from config as a type T.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{double} Value.</returns>
		template<> double get_as(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a value from config as a type T.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{unsigned} Value.</returns>
		template<> unsigned get_as(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a value from config as a type T.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{long} Value.</returns>
		template<> long get_as(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a value from config as a type T.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{long long} Value.</returns>
		template<> long long get_as(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a value from config as a type T.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{unsigned long long} Value.</returns>
		template<> unsigned long long get_as(const std::string&, const std::string&) const;

		// if you know (you probably know) that the config has been set with initializer list, this will break down the list to a std::vector<std::string>
		template<typename T> std::vector<T> get_array(const std::string&, const std::string&) const = delete;

		/// <summary>
		/// <para>Get a vector of values from config.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{std::vector} Vector of values.</returns>
		template<> std::vector<std::string> get_array(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a vector of values from config.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{std::vector} Vector of values.</returns>
		template<> std::vector<bool> get_array(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a vector of values from config.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{std::vector} Vector of values.</returns>
		template<> std::vector<char> get_array(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a vector of values from config.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{std::vector} Vector of values.</returns>
		template<> std::vector<int32_t> get_array(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a vector of values from config.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{std::vector} Vector of values.</returns>
		template<> std::vector<float> get_array(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a vector of values from config.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{std::vector} Vector of values.</returns>
		template<> std::vector<double> get_array(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a vector of values from config.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{std::vector} Vector of values.</returns>
		template<> std::vector<unsigned> get_array(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a vector of values from config.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{std::vector} Vector of values.</returns>
		template<> std::vector<long> get_array(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a vector of values from config.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{std::vector} Vector of values.</returns>
		template<> std::vector<long long> get_array(const std::string&, const std::string&) const;

		/// <summary>
		/// <para>Get a vector of values from config.</para>
		/// </summary>
		/// <param name="{std::string}">Section.</param>
		/// <param name="{std::string}">Key.</param>
		/// <returns>{std::vector} Vector of values.</returns>
		template<> std::vector<unsigned long long> get_array(const std::string&, const std::string&) const;

		void operator=(const config&) = delete;

		/// <summary>
		/// <para>Move operator.</para>
		/// </summary>
		/// <param name="{Config}">Config to move from.</param>
		void operator=(config&&) noexcept;
	};
}

#include "config.ipp"