#include "voice.h"

namespace Lunaris {

	LUNARIS_DECL void __audio_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if ((!al_is_audio_installed() && !al_install_audio()) || (!al_is_acodec_addon_initialized() && !al_init_acodec_addon()) || !al_reserve_samples(voice_audio_samples))
			throw std::runtime_error("Can't start Audio or Audio Codec!");
	}

	LUNARIS_DECL bool voice::set(ALLEGRO_VOICE* nc)
	{
		if (nc) {
#ifdef LUNARIS_VERBOSE_BUILD
			if (device) PRINT_DEBUG("Moved voice %p <- %p", device, nc);
			else PRINT_DEBUG("New voice %p", nc);
#endif
			destroy();
			device = nc;
			return true;
		}
		return false;
	}

	LUNARIS_DECL voice::voice()
	{
		__audio_allegro_start();
	}

	LUNARIS_DECL voice::~voice()
	{
		destroy();
	}

	LUNARIS_DECL voice::voice(voice&& vc) noexcept
		: device(vc.device)
	{
		PRINT_DEBUG("Moved voice new <- %p", vc.device);
		vc.device = nullptr;
	}

	LUNARIS_DECL void voice::operator=(voice&& vc) noexcept
	{
		PRINT_DEBUG("Moved voice %p <- %p", device, vc.device);
		destroy();
		device = vc.device;
		vc.device = nullptr;
	}

	LUNARIS_DECL bool voice::create(const int frequency, const ALLEGRO_AUDIO_DEPTH depth, const ALLEGRO_CHANNEL_CONF conf)
	{
		return set(al_create_voice(frequency, depth, conf));
	}

	LUNARIS_DECL void voice::destroy()
	{
		if (device) {
			PRINT_DEBUG("Del voice %p", device);
			al_destroy_voice(device);
			device = nullptr;
		}
	}

	LUNARIS_DECL bool voice::mute(const bool muted)
	{
		if (device) return al_set_voice_playing(device, !muted);
		return false;
	}

	LUNARIS_DECL bool voice::exists() const
	{
		return device != nullptr;
	}

}