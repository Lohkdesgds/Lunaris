#pragma once

// C++
#include <string>
#include <functional>
#include <vector>
#include <mutex>

#include "../../Handling/Abort/abort.h"
#include "../SuperMutex/supermutex.h"
#include "../Resource/resource.h"


namespace LSW {
	namespace v5 {
		namespace Tools {

			/// <summary>
			/// <para>SuperResource is a one-per-type global resource manager.</para>
			/// <para></para>
			/// </summary>
			template<typename T>
			class SuperResource {

				template<typename Q>
				struct internal_data {
					Resource<Q> _data;
					std::string _id;
					bool _enabled = true;
				};

				template<typename Q>
				struct internal_resources {
					SuperMutex m;
					std::vector<internal_data<Q>> objs;
					Resource<Q> main_o;
				};

				static internal_resources<T> saves;
			public:
				/// <summary>
				/// <para>Gets a object you've set as main (like an atlas or default font).</para>
				/// </summary>
				/// <returns>{Resource} The shared_ptr of the main object.</returns>
				Resource<T> get_main();

				/// <summary>
				/// <para>Sets a object in the SuperResource already as main. (it will still exist if you clear the SuperResource)</para>
				/// </summary>
				/// <param name="{std::string}">Resource's ID.</param>
				/// <returns>{bool} True if found and set.</returns>
				bool set_main(const std::string);

				/// <summary>
				/// <para>Locks internal mutex so you can use begin() and end() (or a for auto).</para>
				/// </summary>
				void lock();

				/// <summary>
				/// <para>Unlocks internal mutex after your use of begin() and end() (or for auto).</para>
				/// </summary>
				void unlock();

				/// <summary>
				/// <para>Begin of the vector.</para>
				/// </summary>
				/// <returns>{iterator} The begin of the vector.</returns>
				auto begin();

				/// <summary>
				/// <para>End of the vector.</para>
				/// </summary>
				/// <returns>{iterator} The end of the vector.</returns>
				auto end();

				/// <summary>
				/// <para>Renames a object X to something else.</para>
				/// </summary>
				/// <param name="{std::string}">The ID right now.</param>
				/// <param name="{std::string}">The new ID.</param>
				/// <returns>{bool} True if found and set.</returns>
				bool rename(const std::string, const std::string);

				/// <summary>
				/// <para>Renames a object as enabled or disabled (filter).</para>
				/// </summary>
				/// <param name="{std::string}">The ID.</param>
				/// <param name="{bool}">Enabled?</param>
				/// <returns>{bool} True if found and set.</returns>
				bool set_enabled(const std::string, const bool);

				/// <summary>
				/// <para>Renames objects based on a formula to a function that generates strings based on old name.</para>
				/// </summary>
				/// <param name="{std::function}">The function matcher.</param>
				/// <param name="{std::function}">The new ID generator based on old string.</param>
				/// <returns>{size_t} Amount of objects changed.</returns>
				size_t rename(const std::function<bool(const std::string)>, const std::function<std::string(const std::string)>);

				/// <summary>
				/// <para>Sets objects enabled or not based on formulas.</para>
				/// </summary>
				/// <param name="{std::function}">The function matcher.</param>
				/// <param name="{std::function}">The function to set if it's enabled or disabled by ID.</param>
				/// <returns>{size_t} Amount of objects changed.</returns>
				size_t set_enabled(const std::function<bool(const std::string)>, const std::function<bool(const std::string)>);

				/// <summary>
				/// <para>Creates the object with ID internally that can be accessed anywhere.</para>
				/// </summary>
				/// <param name="{std::string}">The new object ID.</param>
				/// <param name="{bool}">Is is enabled?</param>
				/// <returns>{Resource} The object created.</returns>
				Resource<T> create(const std::string, const bool = true);

				/// <summary>
				/// <para>Creates the object with ID internally that can be accessed anywhere. (same as create)</para>
				/// </summary>
				/// <param name="{std::string}">The new object ID.</param>
				/// <param name="{bool}">Is is enabled?</param>
				/// <returns>{Resource} The object created.</returns>
				Resource<T> load(const std::string, const bool = true);

				/// <summary>
				/// <para>Gets a object using ID.</para>
				/// </summary>
				/// <param name="{std::string}">The object ID.</param>
				/// <param name="{shared_ptr}">The shared_ptr reference to hold the object.</param>
				/// <param name="{bool}">Search for disabled objects too?</param>
				/// <returns>{bool} True if found.</returns>
				bool get(const std::string, Resource<T>&, const bool = false);

				/// <summary>
				/// <para>Remove a object by ID.</para>
				/// </summary>
				/// <param name="{std::string}">The object ID.</param>
				/// <returns>{bool} True if found and removed.</returns>
				bool remove(const std::string);

				/// <summary>
				/// <para>Remove a object by its own shared_ptr value.</para>
				/// </summary>
				/// <param name="{Resource}">The object itself.</param>
				/// <returns>{bool} True if found and removed.</returns>
				bool remove(const Resource<T>);

				/// <summary>
				/// <para>Gets a list of objects using a function to filter them.</para>
				/// </summary>
				/// <param name="{std::function}">A function to filter what you want.</param>
				/// <returns>{std::vector} A vector of those objects.</returns>
				std::vector<Resource<T>> get_list(const std::function<bool(const std::string)>);

				/// <summary>
				/// <para>Swaps a object with another at the position this ID is found.</para>
				/// </summary>
				/// <param name="{std::string}">Object ID to replace.</param>
				/// <param name="{Resource}">The one that will be swapped.</param>
				/// <returns>{Resource} The swapped object that is not there anymore. (or itself if not found)</returns>
				Resource<T> swap(const std::string, Resource<T>&);

				/// <summary>
				/// <para>Clears the whole internal vector.</para>
				/// </summary>
				void clear();
			};


			template<typename T> SuperResource<T>::internal_resources<T> SuperResource<T>::saves;
		}
	}
}

#include "superresource.ipp"