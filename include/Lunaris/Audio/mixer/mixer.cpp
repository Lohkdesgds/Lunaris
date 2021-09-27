#include "mixer.h"

namespace Lunaris {

	bool mixer::set(ALLEGRO_MIXER* mx)
	{
		if (mx) {
			destroy();
			mixing = mx;
			return true;
		}
		return false;
	}

	mixer::mixer()
	{
		__audio_allegro_start(); // from voice.h
	}

	mixer::~mixer()
	{
		destroy();
	}

	mixer::mixer(mixer&& mx) noexcept
		: mixing(mx.mixing)
	{
		mx.mixing = nullptr;
	}

	void mixer::operator=(mixer&& mx) noexcept
	{
		destroy();
		mixing = mx.mixing;
		mx.mixing = nullptr;
	}

	bool mixer::create(const int frequency, const ALLEGRO_AUDIO_DEPTH depth, const ALLEGRO_CHANNEL_CONF conf)
	{
		return set(al_create_mixer(frequency, depth, conf));
	}

	void mixer::destroy()
	{
		if (mixing) {
			al_destroy_mixer(mixing);
			mixing = nullptr;
		}
	}

	void mixer::mute(const bool muted)
	{
		al_set_mixer_playing(mixing, !muted);
	}

	bool mixer::attach_to(const mixer& mx)
	{
		return mx.exists() && mixing && al_attach_mixer_to_mixer(mixing, mx.mixing);
	}

	bool mixer::attach_to(const voice& vc)
	{
		return vc.exists() && mixing && al_attach_mixer_to_voice(mixing, vc.device);
	}

	float mixer::get_gain() const
	{
		return al_get_mixer_gain(mixing);
	}

	void mixer::set_gain(const float gain)
	{
		if (mixing) al_set_mixer_gain(mixing, gain);
	}

	bool mixer::exists() const
	{
		return mixing != nullptr;
	}

}