#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Audio/voice.h>

namespace Lunaris {

	/// <summary>
	/// <para>mixer is the track manager. You can attach this to a voice device and play some tracks in it.</para>
	/// </summary>
	class mixer : public NonCopyable {
		/*
		* Friend class track: because track should link to a mixer, and it's better if you just set(mixer). get_instance is just another friend-like.
		*/
		friend class track;

		ALLEGRO_MIXER* mixing = nullptr;

		bool set(ALLEGRO_MIXER*);
	public:
		mixer();
		~mixer();

		mixer(mixer&&) noexcept;
		void operator=(mixer&&) noexcept;

		/// <summary>
		/// <para>Creates/Load the mixer with those params.</para>
		/// </summary>
		/// <param name="{int}">The frequency.</param>
		/// <param name="{ALLEGRO_AUDIO_DEPTH}">The depth (bits).</param>
		/// <param name="{ALLEGRO_CHANNEL_CONF}">Channel configuration (surround, 2.0...).</param>
		/// <returns>{bool} True if success.</returns>
		bool create(const int = 48000, const ALLEGRO_AUDIO_DEPTH = ALLEGRO_AUDIO_DEPTH_INT16, const ALLEGRO_CHANNEL_CONF = ALLEGRO_CHANNEL_CONF_2);

		/// <summary>
		/// <para>Destroy this mixer, if exists.</para>
		/// </summary>
		void destroy();

		/// <summary>
		/// <para>Mute sound.</para>
		/// </summary>
		/// <param name="{bool}">If true, mute, else unmute.</param>
		void mute(const bool);

		/// <summary>
		/// <para>Attach THIS mixer into a mixer.</para>
		/// <para>This mixer will be BELOW the mixer you attach to.</para>
		/// </summary>
		/// <param name="{mixer}">The other mixer.</param>
		/// <returns>{bool} True if success.</returns>
		bool attach_to(const mixer&);

		/// <summary>
		/// <para>Attach THIS mixer into a voice device.</para>
		/// <para>This mixer will be BELOW the voice you attach to.</para>
		/// </summary>
		/// <param name="{voice}">The voice.</param>
		/// <returns>{bool} True if success.</returns>
		bool attach_to(const voice&);

		/// <summary>
		/// <para>Get mixer's gain.</para>
		/// </summary>
		/// <returns>{float} mixer gain.</returns>
		float get_gain() const;

		/// <summary>
		/// <para>Set mixer gain.</para>
		/// </summary>
		/// <param name="{float}">mixer gain.</param>
		void set_gain(const float);

		/// <summary>
		/// <para>Is this mixer loaded and ready?</para>
		/// </summary>
		/// <returns>{bool} True if exists.</returns>
		bool exists() const;
	};

}