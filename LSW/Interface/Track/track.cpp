#include "track.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			bool Track::set(ALLEGRO_SAMPLE_INSTANCE* s)
			{
				if (!s) return false;
				playing = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>(s, [](ALLEGRO_SAMPLE_INSTANCE*& b) { if (al_is_system_installed() && b) { al_destroy_sample_instance(b); b = nullptr; } });
				return playing.get();
			}
			
			Track::Track()
			{
				Handling::init_basic();
				Handling::init_audio();
			}

			bool Track::load(const Sample& sample)
			{
				paused_at = 0;
				if (!playing) {
					if (!set(al_create_sample_instance(nullptr))) return false;

					al_set_sample_instance_playing(playing.get(), false);
				}

				if (sample) {
					return al_set_sample(playing.get(), sample.file_sample.get());
				}
				return false;
			}

			bool Track::attach_to(const Mixer& s)
			{
				return al_attach_sample_instance_to_mixer(playing.get(), s.mixer.get());
			}

			bool Track::exists() const
			{
				return playing.get();
			}

			void Track::play()
			{
				if (is_playing()) return;
				al_play_sample_instance(playing.get());
				if (paused_at) set_position_samples(paused_at);
			}

			void Track::pause()
			{
				if (!is_playing()) return;
				paused_at = get_position_samples();
				al_set_sample_instance_playing(playing.get(), false);
			}

			void Track::stop()
			{
				paused_at = 0;
				al_stop_sample_instance(playing.get());
			}

			bool Track::is_playing() const
			{
				return al_get_sample_instance_playing(playing.get());
			}

			ALLEGRO_CHANNEL_CONF Track::get_channel_configuration() const
			{
				return al_get_sample_instance_channels(playing.get());
			}

			size_t Track::get_channels_num() const
			{
				return al_get_channel_count(get_channel_configuration());
			}

			ALLEGRO_AUDIO_DEPTH Track::get_depth() const
			{
				return al_get_sample_instance_depth(playing.get());
			}

			unsigned Track::get_frequency() const
			{
				return al_get_sample_instance_frequency(playing.get());
			}

			unsigned Track::get_length_samples() const
			{
				return al_get_sample_instance_length(playing.get());
			}

			void Track::set_length_samples(const unsigned len)
			{
				al_set_sample_instance_length(playing.get(), len);
			}

			size_t Track::get_length_ms() const
			{
				return static_cast<size_t>((1000.0 * get_length_samples()/* / get_channels_num()*/) / get_frequency());
			}

			void Track::set_length_ms(const size_t ms)
			{
				set_length_samples((ms / 1000.0) * get_frequency()/* * get_channels_num()*/);
			}

			unsigned Track::get_position_samples() const
			{
				return paused_at ? paused_at : al_get_sample_instance_position(playing.get());
			}

			void Track::set_position_samples(const unsigned pos)
			{
				paused_at = 0;
				al_set_sample_instance_position(playing.get(), pos);
			}

			size_t Track::get_position_ms() const
			{
				return static_cast<size_t>((1000.0 * get_position_samples()/* / get_channels_num()*/) / get_frequency());
			}

			void Track::set_position_ms(const size_t ms)
			{
				set_position_samples((ms / 1000.0) * get_frequency()/* * get_channels_num()*/);
			}


			float Track::get_speed() const
			{
				return al_get_sample_instance_speed(playing.get());
			}

			void Track::set_speed(const float speed)
			{
				al_set_sample_instance_speed(playing.get(), speed);
			}

			float Track::get_gain() const
			{
				return al_get_sample_instance_gain(playing.get());
			}

			void Track::set_gain(const float gain)
			{
				al_set_sample_instance_gain(playing.get(), gain);
			}

			float Track::get_pan() const
			{
				return al_get_sample_instance_pan(playing.get());
			}

			void Track::set_pan(const float pan)
			{
				al_set_sample_instance_pan(playing.get(), pan);
			}

			float Track::get_time_s() const
			{
				return al_get_sample_instance_time(playing.get());
			}

			track::play_mode Track::get_play_mode() const
			{
				return static_cast<track::play_mode>(al_get_sample_instance_playmode(playing.get()));
			}

			void Track::set_play_mode(const track::play_mode mode)
			{
				al_set_sample_instance_playmode(playing.get(), static_cast<ALLEGRO_PLAYMODE>(mode));
			}

			// implementation 

			template Tools::Resource<Track>;

		}
	}
}