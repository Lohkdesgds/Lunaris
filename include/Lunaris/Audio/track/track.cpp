#include "track.h"

namespace Lunaris {

	LUNARIS_DECL bool track::set(ALLEGRO_SAMPLE_INSTANCE* si)
	{
		if (si) {
#ifdef LUNARIS_VERBOSE_BUILD
			if (playing) PRINT_DEBUG("Moved track %p <- %p", playing, si);
			else PRINT_DEBUG("New track %p", si);
#endif
			destroy();
			playing = si;
			return true;
		}
		return false;
	}

	LUNARIS_DECL track::track()
	{
		__audio_allegro_start(); // from voice.h
	}

	LUNARIS_DECL track::~track()
	{
		destroy();
	}

	LUNARIS_DECL track::track(track&& tk) noexcept
		: playing(tk.playing), paused_at(tk.paused_at)
	{
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Moved track new <- %p", tk.playing);
#endif
		tk.playing = nullptr;
		tk.paused_at = 0;
	}

	LUNARIS_DECL void track::operator=(track&& tk) noexcept
	{
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Moved track %p <- %p", playing, tk.playing);
#endif
		destroy();
		playing = tk.playing;
		paused_at = tk.paused_at;
		tk.playing = nullptr;
		tk.paused_at = 0;
	}

	LUNARIS_DECL bool track::load(const sample& sp)
	{
		paused_at = 0;

		if (!playing) {
			if (!set(al_create_sample_instance(nullptr))) return false;
			al_set_sample_instance_playing(playing, false);
		}

		if (sp.exists()) {
			return al_set_sample(playing, sp.file_sample);
		}

		return false;
	}

	LUNARIS_DECL void track::destroy()
	{
		if (playing) {
#ifdef LUNARIS_VERBOSE_BUILD
			PRINT_DEBUG("Del track %p", playing);
#endif
			al_destroy_sample_instance(playing);
			playing = nullptr;
		}
	}

	LUNARIS_DECL bool track::attach_to(const mixer& mx)
	{
		return playing && mx.exists() && al_attach_sample_instance_to_mixer(playing, mx.mixing);
	}

	LUNARIS_DECL bool track::exists() const
	{
		return playing != nullptr;
	}

	LUNARIS_DECL bool track::empty() const
	{
		return playing == nullptr;
	}

	LUNARIS_DECL bool track::valid() const
	{
		return playing != nullptr;
	}

	LUNARIS_DECL void track::play()
	{
		if (!exists()) return;
		if (is_playing()) return;
		al_play_sample_instance(playing);
		if (paused_at) set_position_samples(paused_at);
	}

	LUNARIS_DECL void track::pause()
	{
		if (!exists()) return;
		if (!is_playing()) return;
		paused_at = get_position_samples();
		al_set_sample_instance_playing(playing, false);
	}

	LUNARIS_DECL void track::stop()
	{
		if (!exists()) return;
		paused_at = 0;
		al_stop_sample_instance(playing);
	}

	LUNARIS_DECL bool track::is_playing() const
	{
		if (!exists()) return false;
		return al_get_sample_instance_playing(playing);
	}

	LUNARIS_DECL ALLEGRO_CHANNEL_CONF track::get_channel_configuration() const
	{
		if (!exists()) return {};
		return al_get_sample_instance_channels(playing);
	}

	LUNARIS_DECL size_t track::get_channels_num() const
	{
		if (!exists()) return 0;
		return al_get_channel_count(get_channel_configuration());
	}

	LUNARIS_DECL ALLEGRO_AUDIO_DEPTH track::get_depth() const
	{
		if (!exists()) return {};
		return al_get_sample_instance_depth(playing);
	}

	LUNARIS_DECL int track::get_depth_bits() const
	{
		if (!exists()) return {};
		switch (al_get_sample_instance_depth(playing)) {
		case ALLEGRO_AUDIO_DEPTH_INT8:
		case ALLEGRO_AUDIO_DEPTH_UINT8:
			return 8;
		case ALLEGRO_AUDIO_DEPTH_INT16:
		case ALLEGRO_AUDIO_DEPTH_UINT16:
			return 16;
		case ALLEGRO_AUDIO_DEPTH_INT24:
		case ALLEGRO_AUDIO_DEPTH_UINT24:
			return 24;
		case ALLEGRO_AUDIO_DEPTH_FLOAT32:
			return 32;
		default:
			return 0;
		}
	}

	LUNARIS_DECL unsigned track::get_frequency() const
	{
		if (!exists()) return 0;
		return al_get_sample_instance_frequency(playing);
	}

	LUNARIS_DECL unsigned track::get_length_samples() const
	{
		if (!exists()) return 0;
		return al_get_sample_instance_length(playing);
	}

	LUNARIS_DECL void track::set_length_samples(const unsigned len)
	{
		if (!exists()) return;
		al_set_sample_instance_length(playing, len);
	}

	LUNARIS_DECL size_t track::get_length_ms() const
	{
		if (!exists()) return 0;
		return static_cast<size_t>((1000.0 * get_length_samples()/* / get_channels_num()*/) / get_frequency());
	}

	LUNARIS_DECL void track::set_length_ms(const size_t ms)
	{
		if (!exists()) return;
		set_length_samples((ms / 1000.0) * get_frequency()/* * get_channels_num()*/);
	}

	LUNARIS_DECL unsigned track::get_position_samples() const
	{
		if (!exists()) return 0;
		return paused_at ? paused_at : al_get_sample_instance_position(playing);
	}

	LUNARIS_DECL void track::set_position_samples(const unsigned pos)
	{
		if (!exists()) return;
		paused_at = 0;
		al_set_sample_instance_position(playing, pos);
	}

	LUNARIS_DECL size_t track::get_position_ms() const
	{
		if (!exists()) return 0;
		return static_cast<size_t>((1000.0 * get_position_samples()/* / get_channels_num()*/) / get_frequency());
	}

	LUNARIS_DECL void track::set_position_ms(const size_t ms)
	{
		if (!exists()) return;
		set_position_samples((ms / 1000.0) * get_frequency()/* * get_channels_num()*/);
	}

	LUNARIS_DECL float track::get_speed() const
	{
		if (!exists()) return 0.0f;
		return al_get_sample_instance_speed(playing);
	}

	LUNARIS_DECL void track::set_speed(const float speed)
	{
		if (!exists()) return;
		al_set_sample_instance_speed(playing, speed);
	}

	LUNARIS_DECL float track::get_gain() const
	{
		if (!exists()) return 0.0f;
		return al_get_sample_instance_gain(playing);
	}

	LUNARIS_DECL void track::set_gain(const float gain)
	{
		if (!exists()) return;
		al_set_sample_instance_gain(playing, gain);
	}

	LUNARIS_DECL float track::get_pan() const
	{
		if (!exists()) return 0.0f;
		return al_get_sample_instance_pan(playing);
	}

	LUNARIS_DECL void track::set_pan(const float pan)
	{
		if (!exists()) return;
		al_set_sample_instance_pan(playing, pan);
	}

	LUNARIS_DECL float track::get_time_s() const
	{
		if (!exists()) return 0.0f;
		return al_get_sample_instance_time(playing);
	}

	LUNARIS_DECL ALLEGRO_PLAYMODE track::get_play_mode() const
	{
		if (!exists()) return {};
		return al_get_sample_instance_playmode(playing);
	}

	LUNARIS_DECL void track::set_play_mode(const ALLEGRO_PLAYMODE mode)
	{
		if (!exists()) return;
		al_set_sample_instance_playmode(playing, mode);
	}

}
