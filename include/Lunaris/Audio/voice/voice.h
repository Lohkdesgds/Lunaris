#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include <stdexcept>

namespace Lunaris {
#ifdef LUNARIS_AUDIO_SAMPLE_AMOUNT
	constexpr int voice_audio_samples = LUNARIS_AUDIO_SAMPLE_AMOUNT;
#else
	constexpr int voice_audio_samples = 8;
#endif

	void __audio_allegro_start();

	/// <summary>
	/// <para>voice is an audio device. You can attach ONE mixer to this.</para>
	/// </summary>
	class voice {
		/*
		* Friend class mixer: because mixer should link to a voice, and it's better if you just set(voice). get_instance is just another friend-like.
		*/
		friend class mixer;

		ALLEGRO_VOICE* device;

		bool set(ALLEGRO_VOICE*);
	public:
		voice();
		~voice();

		voice(const voice&) = delete;
		void operator=(const voice&) = delete;
		voice(voice&&) noexcept;
		void operator=(voice&&) noexcept;

		/// <summary>
		/// <para>Creates/Load the voice with those params.</para>
		/// </summary>
		/// <param name="{int}">The frequency.</param>
		/// <param name="{ALLEGRO_AUDIO_DEPTH}">The depth (bits).</param>
		/// <param name="{ALLEGRO_CHANNEL_CONF}">Channel configuration (surround, 2.0...).</param>
		/// <returns>{bool} True if success.</returns>
		bool create(const int = 48000, const ALLEGRO_AUDIO_DEPTH = ALLEGRO_AUDIO_DEPTH_INT16, const ALLEGRO_CHANNEL_CONF = ALLEGRO_CHANNEL_CONF_2);

		/// <summary>
		/// <para>Destroy this voice, if exists.</para>
		/// </summary>
		void destroy();

		/// <summary>
		/// <para>Mute sound.</para>
		/// </summary>
		/// <param name="{bool}">If true, mute, else unmute.</param>
		void mute(const bool);

		/// <summary>
		/// <para>Is this voice loaded and ready?</para>
		/// </summary>
		/// <returns>{bool} True if exists.</returns>
		bool exists() const;
	};

}