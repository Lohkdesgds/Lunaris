#include "voice.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			bool Voice::set(ALLEGRO_VOICE* s)
			{
				if (!s) return false;
				device = std::shared_ptr<ALLEGRO_VOICE>(s, [](ALLEGRO_VOICE*& b) { if (al_is_system_installed() && b) { al_destroy_voice(b); b = nullptr; } });
				return device.get();
			}

			Voice::Voice()
			{
				Handling::init_basic();
				Handling::init_audio();
			}

			bool Voice::load(const int frequency, const ALLEGRO_AUDIO_DEPTH depth, const ALLEGRO_CHANNEL_CONF conf)
			{
				return set(al_create_voice(frequency, depth, conf));
			}

			void Voice::mute(const bool m)
			{
				al_set_voice_playing(device.get(), m);
			}

			bool Voice::exists() const
			{
				return device.get();
			}

			// implementation 

			template Tools::Resource<Voice>;

		}
	}
}