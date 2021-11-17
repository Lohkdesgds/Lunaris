#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Audio/voice.h>
#include <Lunaris/Audio/mixer.h>
#include <Lunaris/Audio/sample.h>

namespace Lunaris {

	/// <summary>
	/// <para>track is a playable sample commonly attached to a mixer.</para>
	/// </summary>
	class track : public NonCopyable {
		ALLEGRO_SAMPLE_INSTANCE* playing = nullptr;
		unsigned paused_at = 0;

		bool set(ALLEGRO_SAMPLE_INSTANCE*);
	public:
		track();
		~track();

		track(track&&) noexcept;
		void operator=(track&&) noexcept;

		/// <summary>
		/// <para>Set a sample and load it.</para>
		/// </summary>
		/// <param name="{sample}">sample to load from.</param>
		/// <returns>{bool} True if success.</returns>
		bool load(const sample&);

		/// <summary>
		/// <para>Destroy this track, if exists.</para>
		/// </summary>
		void destroy();

		/// <summary>
		/// <para>Attach this to a mixer.</para>
		/// </summary>
		/// <param name="{mixer}">The mixer.</param>
		/// <returns>{bool} True if success.</returns>
		bool attach_to(const mixer&);

		/// <summary>
		/// <para>Is this loaded?</para>
		/// </summary>
		/// <returns>{bool} True if loaded.</returns>
		bool exists() const;

		/// <summary>
		/// <para>Play/Resume.</para>
		/// </summary>
		void play();

		/// <summary>
		/// <para>Pause and save current position.</para>
		/// </summary>
		void pause();

		/// <summary>
		/// <para>Stop and reset.</para>
		/// </summary>
		void stop();

		/// <summary>
		/// <para>Is it playing?</para>
		/// </summary>
		/// <returns>{bool} Playing.</returns>
		bool is_playing() const;

		/// <summary>
		/// <para>Get this track audio configuration.</para>
		/// </summary>
		/// <returns>{ALLEGRO_CHANNEL_CONF} Audio configuration.</returns>
		ALLEGRO_CHANNEL_CONF get_channel_configuration() const;

		/// <summary>
		/// <para>How many channels does this track have?</para>
		/// </summary>
		/// <returns>{size_t} Number of channels.</returns>
		size_t get_channels_num() const;

		/// <summary>
		/// <para>What is the depth/resolution of this track?</para>
		/// </summary>
		/// <returns>{ALLEGRO_AUDIO_DEPTH} The depth.</returns>
		ALLEGRO_AUDIO_DEPTH get_depth() const;

		/// <summary>
		/// <para>What is the depth/resolution of this track?</para>
		/// </summary>
		/// <returns>{int} The depth in bits.</returns>
		int get_depth_bits() const;

		/// <summary>
		/// <para>What frequency is this track?</para>
		/// </summary>
		/// <returns>{unsigned} Frequency, in hertz.</returns>
		unsigned get_frequency() const;

		/// <summary>
		/// <para>Get the track length in samples.</para>
		/// </summary>
		/// <returns>{unsigned} samples.</returns>
		unsigned get_length_samples() const;

		/// <summary>
		/// <para>Set track sample amount.</para>
		/// </summary>
		/// <param name="{unsigned}">sample amount.</param>
		void set_length_samples(const unsigned);

		/// <summary>
		/// <para>track length in milliseconds.</para>
		/// </summary>
		/// <returns>{size_t} Milliseconds.</returns>
		size_t get_length_ms() const;

		/// <summary>
		/// <para>Set track length in milliseconds</para>
		/// </summary>
		/// <param name="{size_t}">Milliseconds.</param>
		void set_length_ms(const size_t);

		/// <summary>
		/// <para>Get the position in samples.</para>
		/// </summary>
		/// <returns>{unsigned} samples since start.</returns>
		unsigned get_position_samples() const;

		/// <summary>
		/// <para>Set the track position in samples.</para>
		/// </summary>
		/// <param name="{unsigned}">Position in samples</param>
		void set_position_samples(const unsigned);

		/// <summary>
		/// <para>Get track position in milliseconds.</para>
		/// </summary>
		/// <returns>{size_t} Position in milliseconds.</returns>
		size_t get_position_ms() const;

		/// <summary>
		/// <para>Set the track position in milliseconds.</para>
		/// </summary>
		/// <param name="{size_t}">Position in milliseconds.</param>
		void set_position_ms(const size_t);

		/// <summary>
		/// <para>Get current track speed.</para>
		/// </summary>
		/// <returns>{float} Speed.</returns>
		float get_speed() const;

		/// <summary>
		/// <para>Set track speed.</para>
		/// </summary>
		/// <param name="{float}">Speed.</param>
		void set_speed(const float);

		/// <summary>
		/// <para>Get current track gain.</para>
		/// </summary>
		/// <returns>{float} Gain.</returns>
		float get_gain() const;

		/// <summary>
		/// <para>Set track gain.</para>
		/// </summary>
		/// <param name="{float}">Gain.</param>
		void set_gain(const float);

		/// <summary>
		/// <para>Get current track pan/balance.</para>
		/// </summary>
		/// <returns>{float} Pan.</returns>
		float get_pan() const;

		/// <summary>
		/// <para>Set track pan/balance.</para>
		/// </summary>
		/// <param name="{float}">Pan.</param>
		void set_pan(const float);

		/// <summary>
		/// <para>Get full track length in seconds.</para>
		/// </summary>
		/// <returns>{float} Precise length in seconds.</returns>
		float get_time_s() const;

		/// <summary>
		/// <para>Get playing mode.</para>
		/// </summary>
		/// <returns>{ALLEGRO_PLAYMODE} Mode.</returns>
		ALLEGRO_PLAYMODE get_play_mode() const;

		/// <summary>
		/// <para>Set playing mode.</para>
		/// </summary>
		/// <param name="{ALLEGRO_PLAYMODE}">New mode.</param>
		void set_play_mode(const ALLEGRO_PLAYMODE);
	};
}
