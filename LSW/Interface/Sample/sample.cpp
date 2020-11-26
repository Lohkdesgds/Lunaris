#include "sample.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			bool Sample::set(ALLEGRO_SAMPLE* s)
			{
				if (!s) return false;
				file_sample = std::shared_ptr<ALLEGRO_SAMPLE>(s, [](ALLEGRO_SAMPLE*& b) { if (al_is_system_installed() && b) { al_destroy_sample(b); b = nullptr; } });
				return file_sample.get();
			}
			Sample::Sample()
			{
				Handling::init_basic();
				Handling::init_audio();
			}

			bool Sample::load(const std::string p)
			{
				return set(al_load_sample(p.c_str()));
			}

			Sample::operator bool() const
			{
				return file_sample.get();
			}

			bool Sample::operator!() const
			{
				return !file_sample;
			}

			bool Sample::operator==(const Sample& u) const
			{
				return file_sample == u.file_sample;
			}

			bool Sample::exists() const
			{
				return file_sample.get();
			}

			// implementation 

			template Tools::Resource<Sample>;

		}
	}
}