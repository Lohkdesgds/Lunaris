#include "sample.h"

namespace Lunaris {

	LUNARIS_DECL bool sample::set(ALLEGRO_SAMPLE* sp)
	{
		if (sp) {
#ifdef LUNARIS_VERBOSE_BUILD
			if (file_sample) PRINT_DEBUG("Moved sample %p <- %p", file_sample, sp);
			else PRINT_DEBUG("New sample %p", sp);
#endif
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
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Moved sample new <- %p", sp.file_sample);
#endif
		sp.file_sample = nullptr;
	}

	LUNARIS_DECL void sample::operator=(sample&& sp) noexcept
	{
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Moved sample %p <- %p", file_sample, sp.file_sample);
#endif
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
#ifdef LUNARIS_VERBOSE_BUILD
			PRINT_DEBUG("Del sample %p", file_sample);
#endif
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
