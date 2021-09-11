#pragma once

#include "../voice.h"
#include "../mixer.h"

namespace Lunaris {
	
	/// <summary>
	/// <para>A sample is a music file or stream itself. You load a sample and then play/control with a Track.</para>
	/// </summary>
	class sample {
		/*
		* Friend class Track: because Track should link to/with a sample. get_instance is just another friend-like.
		*/
		friend class track;

		ALLEGRO_SAMPLE* file_sample = nullptr;

		bool set(ALLEGRO_SAMPLE*);
	public:
		sample();
		~sample();

		sample(const sample&) = delete;
		void operator=(const sample&) = delete;
		sample(sample&&) noexcept;
		void operator=(sample&&) noexcept;

		/// <summary>
		/// <para>Loads a file.</para>
		/// </summary>
		/// <param name="{std::string}">The path to the file.</param>
		/// <returns>{bool} True if loaded successfully.</returns>
		bool load(const std::string&);

		/// <summary>
		/// <para>Destroy this sample, if exists.</para>
		/// </summary>
		void destroy();

		/// <summary>
		/// <para>Comparison operator.</para>
		/// </summary>
		/// <param name="{sample}">Other sample.</param>
		/// <returns>{bool} True if they are the same.</returns>
		bool operator==(const sample&) const;

		/// <summary>
		/// <para>Is this ready and/or loaded?</para>
		/// </summary>
		/// <returns>{bool} True if loaded.</returns>
		bool exists() const;

	};
	
}