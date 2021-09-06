#include "sample.h"

namespace Lunaris {

	bool sample::set(ALLEGRO_SAMPLE* sp)
	{
		if (sp) {
			destroy();
			file_sample = sp;
			return true;
		}
		return false;
	}

	sample::sample()
	{
		__audio_allegro_start(); // from voice.h
	}

	sample::~sample()
	{
		destroy();
	}

	sample::sample(sample&& sp) noexcept
		: file_sample(sp.file_sample)
	{
		sp.file_sample = nullptr;
	}

	void sample::operator=(sample&& sp) noexcept
	{
		destroy();
		file_sample = sp.file_sample;
		sp.file_sample = nullptr;
	}

	bool sample::load(const std::string& str)
	{
		return set(al_load_sample(str.c_str()));
	}

	void sample::destroy()
	{
		if (file_sample) {
			al_destroy_sample(file_sample);
			file_sample = nullptr;
		}
	}

	bool sample::operator==(const sample& sp) const
	{
		return file_sample == sp.file_sample;
	}

	bool sample::exists() const
	{
		return file_sample != nullptr;
	}

}
