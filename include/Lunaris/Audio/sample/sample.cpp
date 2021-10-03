#include "sample.h"

namespace Lunaris {

	LUNARIS_DECL bool sample::set(ALLEGRO_SAMPLE* sp)
	{
		if (sp) {
			destroy();
			file_sample = sp;
			return true;
		}
		return false;
	}

	LUNARIS_DECL sample::sample()
	{
		__audio_allegro_start(); // from voice.h
	}

	LUNARIS_DECL sample::~sample()
	{
		destroy();
	}

	LUNARIS_DECL sample::sample(sample&& sp) noexcept
		: file_sample(sp.file_sample)
	{
		sp.file_sample = nullptr;
	}

	LUNARIS_DECL void sample::operator=(sample&& sp) noexcept
	{
		destroy();
		file_sample = sp.file_sample;
		sp.file_sample = nullptr;
	}

	LUNARIS_DECL bool sample::load(const std::string& str)
	{
		return set(al_load_sample(str.c_str()));
	}

	LUNARIS_DECL void sample::destroy()
	{
		if (file_sample) {
			al_destroy_sample(file_sample);
			file_sample = nullptr;
		}
	}

	LUNARIS_DECL bool sample::operator==(const sample& sp) const
	{
		return file_sample == sp.file_sample;
	}

	LUNARIS_DECL bool sample::exists() const
	{
		return file_sample != nullptr;
	}

}
