#include "mixer.h"

namespace Lunaris {

	LUNARIS_DECL bool mixer::set(ALLEGRO_MIXER* mx)
	{
		if (mx) {
#ifdef LUNARIS_VERBOSE_BUILD
			if (mixing) PRINT_DEBUG("Moved mixer %p <- %p", mixing, mx);
			else PRINT_DEBUG("New mixer %p", mx);
#endif
			destroy();
			mixing = mx;
			return true;
		}
		return false;
	}

	LUNARIS_DECL mixer::mixer()
	{
		__audio_allegro_start(); // from voice.h
	}

	LUNARIS_DECL mixer::~mixer()
	{
		destroy();
	}

	LUNARIS_DECL mixer::mixer(mixer&& mx) noexcept
		: mixing(mx.mixing)
	{
		PRINT_DEBUG("Moved mixer new <- %p", mx.mixing);
		mx.mixing = nullptr;
	}

	LUNARIS_DECL void mixer::operator=(mixer&& mx) noexcept
	{
		PRINT_DEBUG("Moved mixer %p <- %p", mixing, mx.mixing);
		destroy();
		mixing = mx.mixing;
		mx.mixing = nullptr;
	}

	LUNARIS_DECL bool mixer::create(const int frequency, const ALLEGRO_AUDIO_DEPTH depth, const ALLEGRO_CHANNEL_CONF conf)
	{
		return set(al_create_mixer(frequency, depth, conf));
	}

	LUNARIS_DECL void mixer::destroy()
	{
		if (mixing) {
			PRINT_DEBUG("Del mixer %p", mixing);
			al_destroy_mixer(mixing);
			mixing = nullptr;
		}
	}

	LUNARIS_DECL void mixer::mute(const bool muted)
	{
		al_set_mixer_playing(mixing, !muted);
	}

	LUNARIS_DECL bool mixer::attach_to(const mixer& mx)
	{
		return mx.exists() && mixing && al_attach_mixer_to_mixer(mixing, mx.mixing);
	}

	LUNARIS_DECL bool mixer::attach_to(const voice& vc)
	{
		return vc.exists() && mixing && al_attach_mixer_to_voice(mixing, vc.device);
	}

	LUNARIS_DECL float mixer::get_gain() const
	{
		return al_get_mixer_gain(mixing);
	}

	LUNARIS_DECL void mixer::set_gain(const float gain)
	{
		if (mixing) al_set_mixer_gain(mixing, gain);
	}

	LUNARIS_DECL bool mixer::exists() const
	{
		return mixing != nullptr;
	}

}