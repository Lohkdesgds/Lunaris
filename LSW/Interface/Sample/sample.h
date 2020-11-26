#pragma once

// C++
#include <string>
#include <functional>

// Others
#include "../../Handling/Initialize/initialize.h"
#include "../../Tools/Resource/resource.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			/// <summary>
			/// <para>A Sample is a music file or stream itself. You load a Sample and then play/control with a Track.</para>
			/// </summary>
			class Sample {
				/*
				* Friend class Track: because Track should link to/with a Sample. get_instance is just another friend-like.
				*/
				friend class Track;

				std::shared_ptr<ALLEGRO_SAMPLE> file_sample;

				bool set(ALLEGRO_SAMPLE*);
			public:
				Sample();

				/// <summary>
				/// <para>Loads a file.</para>
				/// </summary>
				/// <param name="{std::string}">The path to the file.</param>
				/// <returns>{bool} True if loaded successfully.</returns>
				bool load(const std::string);

				/// <summary>
				/// <para>Operator that returns true if exists.</para>
				/// </summary>
				operator bool() const;
				
				/// <summary>
				/// <para>Operator that returns true if doesn't exist.</para>
				/// </summary>
				/// <returns>{bool} True if not loaded.</returns>
				bool operator!() const;

				/// <summary>
				/// <para>Comparison operator.</para>
				/// </summary>
				/// <param name="{Sample}">Other Sample.</param>
				/// <returns>{bool} True if they are the same.</returns>
				bool operator==(const Sample&) const;

				/// <summary>
				/// <para>Is this ready and/or loaded?</para>
				/// </summary>
				/// <returns>{bool} True if loaded.</returns>
				bool exists() const;

			};
		}
	}
}