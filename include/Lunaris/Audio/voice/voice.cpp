#include "voice.h"

namespace Lunaris {

	void __audio_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if ((!al_is_audio_installed() && !al_install_audio()) || (!al_is_acodec_addon_initialized() && !al_init_acodec_addon()) || !al_reserve_samples(voice_audio_samples))
			throw std::runtime_error("Can't start Audio or Audio Codec!");
	}

	bool voice::set(ALLEGRO_VOICE* nc)
	{
		if (nc) {
			destroy();
			device = nc;
			return true;
		}
		return false;
	}

	voice::voice()
	{
		__audio_allegro_start();
	}

	voice::~voice()
	{
		destroy();
	}

	voice::voice(voice&& vc) noexcept
		: device(vc.device)
	{
		vc.device = nullptr;
	}

	void voice::operator=(voice&& vc) noexcept
	{
		destroy();
		device = vc.device;
		vc.device = nullptr;
	}

	bool voice::create(const int frequency, const ALLEGRO_AUDIO_DEPTH depth, const ALLEGRO_CHANNEL_CONF conf)
	{
		return set(al_create_voice(frequency, depth, conf));
	}

	void voice::destroy()
	{
		if (device) {
			al_destroy_voice(device);
			device = nullptr;
		}
	}

	bool voice::mute(const bool muted)
	{
		if (device) return al_set_voice_playing(device, !muted);
		return false;
	}

	bool voice::exists() const
	{
		return device != nullptr;
	}

}