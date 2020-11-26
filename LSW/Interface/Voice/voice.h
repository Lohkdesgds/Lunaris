#pragma once

// C++
#include <memory>
#include <string>

// Others
#include "../../Handling/Initialize/initialize.h"
#include "../../Tools/Resource/resource.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			/// <summary>
			/// <para>Voice is an audio device. You can attach ONE Mixer to this.</para>
			/// </summary>
			class Voice {
				/*
				* Friend class Mixer: because Mixer should link to a Voice, and it's better if you just set(Voice). get_instance is just another friend-like.
				*/
				friend class Mixer;

				std::shared_ptr<ALLEGRO_VOICE> device;

				bool set(ALLEGRO_VOICE*);
			public:
				Voice();

				/// <summary>
				/// <para>Creates/Load the Voice with those params.</para>
				/// </summary>
				/// <param name="{int}">The frequency.</param>
				/// <param name="{ALLEGRO_AUDIO_DEPTH}">The depth (bits).</param>
				/// <param name="{ALLEGRO_CHANNEL_CONF}">Channel configuration (surround, 2.0...).</param>
				/// <returns>{bool} True if success.</returns>
				bool load(const int = 48000, const ALLEGRO_AUDIO_DEPTH = ALLEGRO_AUDIO_DEPTH_INT16, const ALLEGRO_CHANNEL_CONF = ALLEGRO_CHANNEL_CONF_2);

				/// <summary>
				/// <para>Mute sound.</para>
				/// </summary>
				/// <param name="{bool}">If true, mute, else unmute.</param>
				void mute(const bool);

				/// <summary>
				/// <para>Is this Voice loaded and ready?</para>
				/// </summary>
				/// <returns>{bool} True if exists.</returns>
				bool exists() const;
			};

		}
	}
}