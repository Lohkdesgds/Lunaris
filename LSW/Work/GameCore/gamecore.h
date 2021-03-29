#pragma once

#include "../../Tools/SuperResource/superresource.h"
#include "../../Interface/Logger/logger.h"
#include "../../Interface/Config/config.h"
#include "../../Interface/Voice/voice.h"
#include "../../Interface/Mixer/mixer.h"
#include "../../Interface/Display/display.h"
// include layering


namespace LSW {
	namespace v5 {
		namespace Work {

			/// <summary>
			/// <para>Generates a string based on build date.</para>
			/// </summary>
			/// <returns>{std::string} A string used for versioning.</returns>
			const std::string __interpret_date();

			inline const std::string version_app = std::string("B") + __interpret_date();

			namespace gamecore {
				const std::string conf_versioning		= "versioning";
				const std::string conf_displaying		= "display";
				const std::string conf_audio			= "audio";
				const std::string conf_debug			= "debug";
				const std::string conf_mouse_memory		= "mouse"; // in memory only ### TIED TO SPRITE
				// add other common always used section names

				const double delta_checking_s = 5.0; // sec, check registered events like display
				const double delta_mouse_s = 1.0 / 60; // sec
			}

			/// <summary>
			/// <para>This is the quickest way to handle common tasks. Work classes will work with this.</para>
			/// <para>If you want custom, you can do the same creating a GameCore-like class via inheritance.</para>
			/// <para>Included on "common tasks":</para>
			/// <para>- Automatic latest version in config file (ensure, no set)</para>
			/// <para>- Automatic display size and fullscreen/window mode save/load</para>
			/// <para>- Automatic relink display (if recreated or something)</para>
			/// <para>- Relative and absolute (x and y, rx and ry) mouse positioning (conf_mouse_memory)</para>
			/// <para>- Mouse button state (down_latest and press_count, bNUMBER is set to true NUMBER being button number)</para>
			/// </summary>
			class GameCore {
			protected:
				struct shared {
					Interface::Logger logg;
					Interface::Config conf;
					Interface::Voice audio;
					Interface::Mixer mixer;
					Interface::Display display{0};

					Interface::EventHandler events{ Tools::superthread::performance_mode::HIGH_PERFORMANCE }; // just display close event and mouse registration for now

					Interface::EventTimer check_sources; // like when you reload screen the source change.
					Interface::EventTimer update_mouse;
					bool _m_newdata = false;
					float _m_x{}, _m_y{};

					Interface::Event latest_display_source; // copy
					Tools::Future<bool> _temp_holder = Tools::fake_future<bool>(false);

					bool closed = false; // if app is closed, true
					bool loaded = false; // loaded once

					Tools::SuperMutex m;

					shared(const size_t);
				};
				
				std::shared_ptr<shared> share;// = std::make_shared<shared>();

				// copy of GameCore so multiple call easy
				void _load(const std::string&, const std::string&, const size_t);
			public:
				/// <summary>
				/// <para>Start Logger and Config directly. Display index is set by the parameter.</para>
				/// <para>As Logger is a global logging system, if it has been set earlier, this won't overwrite its settings.</para>
				/// <para>You can create a Future&lt;bool&gt; before if you do fake_future&lt;bool&gt;(false). After calling this, your future will be a real future.</para>
				/// </summary>
				/// <param name="{std::string}">Logger logging output.</param>
				/// <param name="{std::string}">Configuration path.</param>
				/// <param name="{size_t}">Display index (targeting).</param>
				/// <param name="{Future}">Display's Future (when closed this is .get()able).</param>
				GameCore(const std::string&, const std::string&, const size_t, Tools::Future<bool>&);

				/// <summary>
				/// <para>Start Logger and Config directly. Display index is set by the parameter.</para>
				/// <para>As Logger is a global logging system, if it has been set earlier, this won't overwrite its settings.</para>
				/// </summary>
				/// <param name="{std::string}">Logger logging output.</param>
				/// <param name="{std::string}">Configuration path.</param>
				/// <param name="{size_t}">Display index (targeting).</param>
				GameCore(const std::string&, const std::string&, const size_t = 0);

				/// <summary>
				/// <para>Copy reference constructor.</para>
				/// <para>Shutdown itself if anything is set.</para>
				/// </summary>
				/// <param name="{GameCore}">GameCore to reference.</param>
				GameCore(const GameCore&);

				/// <summary>
				/// <para>Move constructor.</para>
				/// <para>Shutdown itself if anything is set.</para>
				/// </summary>
				/// <param name="{GameCore}">GameCore to move from.</param>
				GameCore(GameCore&&);

				/// <summary>
				/// <para>Ends all tasks and shutdown everything. You'll have to restart via constructor.</para>
				/// </summary>
				void shutdown();

				/// <summary>
				/// <para>Waits for display close or close app call.</para>
				/// </summary>
				void yield();

				/// <summary>
				/// <para>Is the app up?</para>
				/// </summary>
				/// <returns>{bool} True if it's not set as closed.</returns>
				bool running() const;


				/// <summary>
				/// <para>Copy reference operator.</para>
				/// <para>Shutdown itself if anything is set.</para>
				/// </summary>
				/// <param name="{GameCore}">GameCore to reference.</param>
				void operator=(const GameCore&);

				/// <summary>
				/// <para>Move operator.</para>
				/// <para>Shutdown itself if anything is set.</para>
				/// </summary>
				/// <param name="{GameCore}">GameCore to move from.</param>
				void operator=(GameCore&&);

				/// <summary>
				/// <para>Can only be done once AND ONLY IF NOT DONE IN CONSTRUCTOR!</para>
				/// <para>If called otherwise, it might throw an exception or do unknown behaviour!</para>
				/// </summary>
				/// <returns>{Future} Display's Future.</returns>
				Tools::Future<bool> get_display_future();

				/// <summary>
				/// <para>Gets configuration.</para>
				/// </summary>
				/// <returns>{Config} Config here set.</returns>
				Interface::Config& get_config();

				/// <summary>
				/// <para>Gets main Mixer.</para>
				/// </summary>
				/// <returns>{Mixer} Main Mixer.</returns>
				Interface::Mixer& get_mixer();

				/// <summary>
				/// <para>Gets this core's display.</para>
				/// </summary>
				/// <returns>{Display} Display.</returns>
				Interface::Display& get_display();


				/// <summary>
				/// <para>Gets configuration.</para>
				/// </summary>
				/// <returns>{Config} Config here set.</returns>
				const Interface::Config& get_config() const;

				/// <summary>
				/// <para>Gets main Mixer.</para>
				/// </summary>
				/// <returns>{Mixer} Main Mixer.</returns>
				const Interface::Mixer& get_mixer() const;

				/// <summary>
				/// <para>Gets this core's display.</para>
				/// </summary>
				/// <returns>{Display} Display.</returns>
				const Interface::Display& get_display() const;
			};

		}
	}
}