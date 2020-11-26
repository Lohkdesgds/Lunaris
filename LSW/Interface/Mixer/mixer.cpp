#include "mixer.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			bool Mixer::set(ALLEGRO_MIXER* s)
			{
				if (!s) return false;
				mixer = std::shared_ptr<ALLEGRO_MIXER>(s, [](ALLEGRO_MIXER*& b) { if (al_is_system_installed() && b) { al_destroy_mixer(b); b = nullptr; } });
				return mixer.get();
			}

			Mixer::Mixer()
			{
				Handling::init_basic();
				Handling::init_audio();
			}

			bool Mixer::load(const int frequency, const ALLEGRO_AUDIO_DEPTH depth, const ALLEGRO_CHANNEL_CONF conf)
			{
				return set(al_create_mixer(frequency, depth, conf));
			}

			void Mixer::mute(const bool m)
			{
				al_set_mixer_playing(mixer.get(), m);
			}

			bool Mixer::attach_to(const Mixer& s)
			{
				return al_attach_mixer_to_mixer(mixer.get(), s.mixer.get());
			}

			bool Mixer::attach_to(const Voice& s)
			{
				return al_attach_mixer_to_voice(mixer.get(), s.device.get());
			}

			float Mixer::get_gain() const
			{
				return al_get_mixer_gain(mixer.get());
			}

			void Mixer::set_gain(const float f)
			{
				al_set_mixer_gain(mixer.get(), f);
			}

			bool Mixer::exists() const
			{
				return mixer.get();
			}

			// implementation 

			template Tools::Resource<Mixer>;

		}
	}
}