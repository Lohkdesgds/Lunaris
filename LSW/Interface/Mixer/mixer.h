#pragma once

// C++
#include <memory>
#include <string>

// Others
#include "../../Handling/Initialize/initialize.h"
#include "../../Tools/Resource/resource.h"
#include "../Voice/voice.h"


namespace LSW {
	namespace v5 {
		namespace Interface {

			/// <summary>
			/// <para>Mixer is the Track manager. You can attach this to a Voice device and play some Tracks in it.</para>
			/// </summary>
			class Mixer {
				/*
				* Friend class Track: because Track should link to a Mixer, and it's better if you just set(Mixer). get_instance is just another friend-like.
				*/
				friend class Track;

				std::shared_ptr<ALLEGRO_MIXER> mixer;

				bool set(ALLEGRO_MIXER*);
			public:
				Mixer();
								
				/// <summary>
				/// <para>Creates/Load the Mixer with those params.</para>
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
				/// <para>Attach THIS Mixer into a Mixer.</para>
				/// <para>This Mixer will be BELOW the Mixer you attach to.</para>
				/// </summary>
				/// <param name="{Mixer}">The other Mixer.</param>
				/// <returns>{bool} True if success.</returns>
				bool attach_to(const Mixer&);

				/// <summary>
				/// <para>Attach THIS Mixer into a Voice device.</para>
				/// <para>This Mixer will be BELOW the Voice you attach to.</para>
				/// </summary>
				/// <param name="{Voice}">The Voice.</param>
				/// <returns>{bool} True if success.</returns>
				bool attach_to(const Voice&);

				/// <summary>
				/// <para>Get Mixer's gain.</para>
				/// </summary>
				/// <returns>{float} Mixer gain.</returns>
				float get_gain() const;
				/// <summary>
				/// <para>Set Mixer gain.</para>
				/// </summary>
				/// <param name="{float}">Mixer gain.</param>
				void set_gain(const float);

				/// <summary>
				/// <para>Is this Mixer loaded and ready?</para>
				/// </summary>
				/// <returns>{bool} True if exists.</returns>
				bool exists() const;
			};

		}
	}
}