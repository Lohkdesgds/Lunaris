#pragma once

/*
 * SPRITE_BASE WILL ASSUME GAMECORE MOUSE CONFIG FORMAT, EVEN THOUGH IT'S NOT DEPENDENT.
 * 
 * This basically means it will only work 10/10 if you set config with:
 * 
 * FORMAT: '[ SECTION, KEY ]: description; type'
 * 
 * ## MOUSE CONFIGURATION ##
 * - [ "mouse", "x" ]:				Position X; float
 * - [ "mouse", "y" ]:				Position Y; float
 * - [ "mouse", "rx" ]:				Position X (no camera); float
 * - [ "mouse", "ry" ]:				Position Y (no camera); float
 * - [ "mouse", "press_count" ]:	Pressed buttons count; unsigned
 * - [ "mouse", "down_latest" ]:	Latest event, from what key, starting with 1 for button 0, negative for release; int
 * - [ "mouse", "b#" ]:				Button # (placeholder) latest state, if not set, never had one; bool
*/

#include "../../Handling/Abort/abort.h"
#include "../../Handling/Initialize/initialize.h"
#include "../../Tools/Any/any.h"
#include "../../Tools/SuperMap/SuperMap.h"
#include "../../Tools/SuperFunctionalMap/superfunctionalmap.h"
#include "../../Tools/SuperMutex/supermutex.h"
#include "../../Tools/Resource/resource.h"
#include "../../Interface/Camera/camera.h"
#include "../../Interface/Config/config.h"
#include "../../Interface/Color/color.h"

#include <array>

namespace LSW {
	namespace v5 {
		namespace Work {

			namespace sprite {
				/*
				COL_MINDIST_... = the distance it has to move in ... (on a collision tick)
				...UPDATE... = automatic smoothing based on collision calls
				*/
				enum class e_double_readonly { PERC_CALC_SMOOTH, SPEED_X, SPEED_Y, LAST_COLLISION_TIME /* related to: COLLISION_COLLIDED */,
					LAST_DRAW, LAST_UPDATE, UPDATE_DELTA, POSX, POSY, PROJECTED_POSX, PROJECTED_POSY, ROTATION/*, COL_MINDIST_X, COL_MINDIST_Y*/,
					MOUSE_CLICK_LAST_X, MOUSE_CLICK_LAST_Y,
					REALISTIC_RESULT_POSX, REALISTIC_RESULT_POSY, REALISTIC_RESULT_SCALE_X, REALISTIC_RESULT_SCALE_Y
				};
				enum class e_boolean_readonly { COLLISION_MOUSE_PRESSED, COLLISION_MOUSE_CLICK, COLLISION_COLLIDED /* related to: LAST_COLLISION_TIME */, INVALIDATE_MOUSE_NOMOVE };
				enum class e_tief_readonly { LAST_STATE };

				enum class e_string { ID };
				enum class e_double { TARG_POSX, TARG_POSY, SCALE_X, SCALE_Y, SCALE_G, CENTER_X, CENTER_Y, TARG_ROTATION, ACCELERATION_X, ACCELERATION_Y, SPEEDXY_LIMIT, ELASTICITY_X, ELASTICITY_Y, ROUGHNESS };
				enum class e_boolean { DRAW, USE_COLOR, AFFECTED_BY_CAM, DRAW_DOT, DRAW_COLOR_BOX, DRAW_DEBUG_BOX, RESPECT_CAMERA_LIMITS /*Readonly means no collision or acceleration, just f()*/ };
				enum class e_integer { COLLISION_MODE };
				enum class e_color { COLOR };
				enum class e_uintptrt { DATA_FROM, INDEX_TARGET_IN_USE }; // == size_t

				enum class e_collision_mode_cast {
					COLLISION_BOTH, // collide and move if colliding
					COLLISION_STATIC, // collide with others and stay static
					COLLISION_INVERSE, // doesn't cause collision on others, but move if someone is colliding with it
					COLLISION_NONE // no collision behaviour at all
				};

				enum class e_direction_array_version {
					NORTH, SOUTH, EAST, WEST
				};

				// saving as a int with binary vals
				enum class e_direction {
					NONE = 0,
					NORTH = 1 << 0,
					SOUTH = 1 << 1,
					EAST  = 1 << 2,
					WEST  = 1 << 3
				};

				enum class e_tie_functional { 
					COLLISION_NONE = 0,
					DELAYED_WORK_AUTODEL,

					COLLISION_MOUSE_ON,
					COLLISION_MOUSE_OFF,
					COLLISION_MOUSE_CLICK,
					COLLISION_MOUSE_UNCLICK,
					COLLISION_MOUSE_CLICK_NOMOVE, // also triggers CLICK above, may overwrite sprite::e_tief_readonly::LAST_STATE
					COLLISION_MOUSE_UNCLICK_NOMOVE, // also triggers UNCLICK above, may overwrite sprite::e_tief_readonly::LAST_STATE
					COLLISION_COLLIDED_OTHER,

					_MOUSE_BEGIN = COLLISION_MOUSE_ON,
					_MOUSE_END = COLLISION_MOUSE_UNCLICK_NOMOVE
				};


				using functional = std::function<void(const Tools::Any&)>;


				constexpr double minimum_sprite_accel_collision = 1e-3;
				constexpr double game_collision_oversize = 1e-3;
				constexpr double maximum_time_between_collisions = 1.0; // sec, used for smoothness too
				constexpr double move_accept_move_max_as_none = 0.025; // click event, how much movement is considered no move? (around the point, +/-)

				const Tools::SuperMap<Tools::FastFunction<std::string>> e_string_defaults = {
					{std::string(""),												(e_string::ID),										("id")}
				};

				const Tools::SuperMap<Tools::FastFunction<double>> e_double_defaults = {
					{0.0,															(e_double_readonly::PERC_CALC_SMOOTH),				("perc_calc_smooth")},
					{0.0,															(e_double_readonly::SPEED_X),						("speed_x")},
					{0.0,															(e_double_readonly::SPEED_Y),						("speed_y")},
					{0.0,															(e_double_readonly::LAST_COLLISION_TIME),			("last_collision_time")},
					{0.0,															(e_double_readonly::LAST_DRAW),						("last_draw")},
					{0.0,															(e_double_readonly::LAST_UPDATE),					("last_update")},
					{0.0,															(e_double_readonly::UPDATE_DELTA),					("update_delta")},
					{0.0,															(e_double_readonly::POSX),							("pos_x")}, // WAS drawing POSX
					{0.0,															(e_double_readonly::POSY),							("pos_y")}, // WAS drawing POSY
					{0.0,															(e_double_readonly::PROJECTED_POSX),				("projected_pos_x")}, // drawing POSX
					{0.0,															(e_double_readonly::PROJECTED_POSY),				("projected_pos_y")}, // drawing POSY
					{0.0,															(e_double_readonly::ROTATION),						("rotation")}, // drawing ROTATION
					{0.0,															(e_double_readonly::MOUSE_CLICK_LAST_X),			("mouse_click_last_x")},
					{0.0,															(e_double_readonly::MOUSE_CLICK_LAST_Y),			("mouse_click_last_y")},
					{0.0,															(e_double_readonly::REALISTIC_RESULT_POSX),			("realistic_result_x")}, // after affected_by_cam, how does it look for affected ones in position?
					{0.0,															(e_double_readonly::REALISTIC_RESULT_POSY),			("realistic_result_y")}, // after affected_by_cam, how does it look for affected ones in position?
					{0.0,															(e_double_readonly::REALISTIC_RESULT_SCALE_X),		("realistic_result_scale_x")}, // after affected_by_cam, how does it look for affected ones in scale?
					{0.0,															(e_double_readonly::REALISTIC_RESULT_SCALE_Y),		("realistic_result_scale_y")}, // after affected_by_cam, how does it look for affected ones in scale?

					{0.0,															(e_double::TARG_POSX),								("target_pos_x")},
					{0.0,															(e_double::TARG_POSY),								("target_pos_y")},
					{1.0,															(e_double::SCALE_X),								("scale_x")},
					{1.0,															(e_double::SCALE_Y),								("scale_y")},
					{1.0,															(e_double::SCALE_G),								("scale_g")},
					{0.0,															(e_double::CENTER_X),								("center_x")},
					{0.0,															(e_double::CENTER_Y),								("center_y")},
					{0.0,															(e_double::TARG_ROTATION),							("target_rotation")},
					{0.0,															(e_double::ACCELERATION_X),							("acceleration_x")},
					{0.0,															(e_double::ACCELERATION_Y),							("acceleration_y")},
					{0.3,															(e_double::SPEEDXY_LIMIT),							("speed_limit")},
					{0.85,															(e_double::ELASTICITY_X),							("elasticity_x")},
					{0.85,															(e_double::ELASTICITY_Y),							("elasticity_y")},
					{0.98,															(e_double::ROUGHNESS),								("roughness")}
				};

				const Tools::SuperMap<Tools::FastFunction<bool>> e_boolean_defaults = {
					{false,															(e_boolean_readonly::COLLISION_MOUSE_PRESSED),		("collision_mouse_on")},
					{false,															(e_boolean_readonly::COLLISION_MOUSE_CLICK),		("collision_mouse_click")},
					{false,															(e_boolean_readonly::COLLISION_COLLIDED),			("collision_collided")},
					{false,															(e_boolean_readonly::INVALIDATE_MOUSE_NOMOVE),		("invalidate_mouse_nomove")},

					{true,															(e_boolean::DRAW),									("draw")},
					{false,															(e_boolean::USE_COLOR),								("use_color")},
					{true,															(e_boolean::AFFECTED_BY_CAM),						("affected_by_camera")},
					{false,															(e_boolean::DRAW_DOT),								("draw_dot")}, // shows dot where it will be drawn
					{false,															(e_boolean::DRAW_COLOR_BOX),						("draw_color_box")}, // box with COLOR
					{false,															(e_boolean::DRAW_DEBUG_BOX),						("draw_debug_box")}, // shows rectangle where collision updated (latest update)
					{true,															(e_boolean::RESPECT_CAMERA_LIMITS),					("respect_camera_limits")}
				};

				const Tools::SuperMap<Tools::FastFunction<int>> e_integer_defaults = {
					{static_cast<int>(e_collision_mode_cast::COLLISION_BOTH),		(e_integer::COLLISION_MODE),						("collision_mode")}
				};

				const Tools::SuperMap<Tools::FastFunction<Interface::Color>> e_color_defaults = {
					{Interface::Color(255,255,255),									(e_color::COLOR),									("color")}
				};

				const Tools::SuperMap<Tools::FastFunction<uintptr_t>> e_uintptrt_defaults = {
					{(uintptr_t)0,													(e_uintptrt::DATA_FROM)},
					{(uintptr_t)0,													(e_uintptrt::INDEX_TARGET_IN_USE)}
				};

				const Tools::SuperMap<Tools::FastFunction<e_tie_functional>> e_tief_defaults = {
					{e_tie_functional::COLLISION_NONE,								(e_tief_readonly::LAST_STATE)}
				};

				const Tools::SuperMap<Tools::FastFunction<functional>> e_functional_defaults = {
					{functional(),													(e_tie_functional::DELAYED_WORK_AUTODEL)},
					{functional(),													(e_tie_functional::COLLISION_MOUSE_ON)},
					{functional(),													(e_tie_functional::COLLISION_MOUSE_CLICK)},
					{functional(),													(e_tie_functional::COLLISION_MOUSE_UNCLICK)},
					{functional(),													(e_tie_functional::COLLISION_COLLIDED_OTHER)},
					{functional(),													(e_tie_functional::COLLISION_NONE)}
				};
			}

			/// <summary>
			/// <para>Sprite_Base: it is the "core" for many things in the Work part of the library.</para>
			/// <para>This has collision, transformation, common tags, debugging and some features to get other derived classes almost up and running already. You define draw_task and think_task and you're good to go!</para>
			/// <para>This allows classes to be compatible collision-wide and makes drawing them so easy via draw().</para>
			/// </summary>
			class Sprite_Base :
				public Tools::SuperFunctionalMap<double>,public Tools::SuperFunctionalMap<bool>,public Tools::SuperFunctionalMap<std::string>,public Tools::SuperFunctionalMap<int>,public Tools::SuperFunctionalMap<Interface::Color>,public Tools::SuperFunctionalMap<uintptr_t>,public Tools::SuperFunctionalMap<sprite::functional>,public Tools::SuperFunctionalMap<sprite::e_tie_functional>
			{
				struct easier_collision_handle {
					double	posx = 0.0,
							posy = 0.0,
							dx_max = 0.0,
							dy_max = 0.0,
							sizx = 0.0, // full size
							sizy = 0.0; // full size

					int directions_cases[4] = { 0 };
					bool was_col = false;
					Interface::classic_2d latest_camera;

					easier_collision_handle() = default;
					easier_collision_handle(const easier_collision_handle&);

					// test overlap between this and someone else
					bool overlap(const easier_collision_handle&);
					// Where should I go? (combo x/y of sprite::e_direction)
					int process_result();
					// Resets directions history
					void reset_directions();
					// X, Y, SX, SY
					void setup(const double, const double, const double, const double);
				} easy_collision;

				/// <summary>
				/// <para>Set this as your drawing function (Sprite_Base's draw will call this).</para>
				/// </summary>
				/// <param name="{Camera}">The Camera applied here (AFFECTED_BY_CAM affect this).</param>
				virtual void draw_task(Interface::Camera&) {}

				/// <summary>
				/// <para>Set this as your thinking function (Sprite_Base's update_and_clear will call this).</para>
				/// </summary>
				/// <param name="{int}">The collision result direction (e_direction combination).</param>
				virtual void think_task(const int) {}

				/// <summary>
				/// <para>Set this as your mouse event function (Sprite_Base's update_and_clear will call this).</para>
				/// </summary>
				/// <param name="{sprite::e_tie_functional}">Latest event (trigger once per change).</param>
				/// <param name="{Tools::Any}">It depends on the event.</param>
				virtual void mouse_event(const sprite::e_tie_functional, const Tools::Any&) {}

			public:
				using Tools::SuperFunctionalMap<double>::set;
				using Tools::SuperFunctionalMap<double>::get;
				using Tools::SuperFunctionalMap<double>::get_direct;
				using Tools::SuperFunctionalMap<int>::set;
				using Tools::SuperFunctionalMap<int>::get;
				using Tools::SuperFunctionalMap<int>::get_direct;
				using Tools::SuperFunctionalMap<Interface::Color>::set;
				using Tools::SuperFunctionalMap<Interface::Color>::get;
				using Tools::SuperFunctionalMap<Interface::Color>::get_direct;
				using Tools::SuperFunctionalMap<std::string>::set;
				using Tools::SuperFunctionalMap<std::string>::get;
				using Tools::SuperFunctionalMap<std::string>::get_direct;
				using Tools::SuperFunctionalMap<bool>::set;
				using Tools::SuperFunctionalMap<bool>::get;
				using Tools::SuperFunctionalMap<bool>::get_direct;
				using Tools::SuperFunctionalMap<uintptr_t>::set;
				using Tools::SuperFunctionalMap<uintptr_t>::get;
				using Tools::SuperFunctionalMap<uintptr_t>::get_direct;
				using Tools::SuperFunctionalMap<sprite::functional>::set;
				using Tools::SuperFunctionalMap<sprite::functional>::get;
				using Tools::SuperFunctionalMap<sprite::functional>::get_direct;
				using Tools::SuperFunctionalMap<sprite::e_tie_functional>::set;
				using Tools::SuperFunctionalMap<sprite::e_tie_functional>::get;
				using Tools::SuperFunctionalMap<sprite::e_tie_functional>::get_direct;

				/// <summary>
				/// <para>Empty default start.</para>
				/// </summary>
				Sprite_Base();

				/// <summary>
				/// <para>Constructor referencing a Sprite_Base (not a copy).</para>
				/// </summary>
				/// <param name="{Sprite_Base}">The one to reference attributes from (no copy).</param>
				Sprite_Base(const Sprite_Base&);

				/// <summary>
				/// <para>Constructor to move a Sprite_Base to this (move).</para>
				/// </summary>
				/// <param name="{Sprite_Base}">The one to move attributes from.</param>
				Sprite_Base(Sprite_Base&&);

				/// <summary>
				/// <para>Clone other Sprite_Base attributes.</para>
				/// <para>You can also clone a specific type by doing set(*get()).</para>
				/// </summary>
				/// <param name="{Sprite_Base}">Other to copy from.</param>
				void clone(const Sprite_Base&);

				/// <summary>
				/// <para>Reference a Sprite_Base (not a copy).</para>
				/// </summary>
				/// <param name="{Sprite_Base}">The one to reference attributes from (no copy).</param>
				void operator=(const Sprite_Base&);

				/// <summary>
				/// <para>Move a Sprite_Base to this (move).</para>
				/// </summary>
				/// <param name="{Sprite_Base}">The one to move attributes from.</param>
				void operator=(Sprite_Base&&);

				/// <summary>
				/// <para>Compare values right now given a key.</para>
				/// </summary>
				/// <param name="{T}">A key.</param>
				/// <param name="{V}">A value</param>
				/// <returns>{bool} True if equal right now.</returns>
				template<typename T, typename V> inline bool is_eq(const T& e, const V& v) const { return get_direct<V>(e) == v; }

				/// <summary>
				/// <para>Compare values right now directly to another Sprite_Base given a key.</para>
				/// </summary>
				/// <param name="{T}">A key.</param>
				/// <param name="{Sprite_Base}">A Sprite_Base with same key set to compare directly.</param>
				/// <returns>{bool} True if equal right now.</returns>
				template<typename V, typename T> inline bool is_eq_s(const T& e, const Sprite_Base& s) const { return get_direct<V>(e) == s.get_direct<V>(e); }

				/// <summary>
				/// <para>Calculate and draw this (calls derived class implementation of draw_task. PLEASE DON'T OVERRIDE THIS).</para>
				/// </summary>
				/// <param name="{Camera}">Reference camera used in draw, properties may change it to default.</param>
				/// <param name="{bool}">Do draw even though this is a copy (if it is)?</param>
				void draw(const Interface::Camera&, const bool = false);

				/// <summary>
				/// <para>Collide with another Sprite_Base.</para>
				/// </summary>
				/// <param name="{Sprite_Base}">Another Sprite_Base derived class.</param>
				/// <param name="{bool}">Do collision even though this is a copy (if it is)?</param>
				void collide(const Sprite_Base&, const bool = false);

				/// <summary>
				/// <para>Update based on collisions and task other specific stuff like mouse collision (based on the Config) and smoothness automatic calculation.</para>
				/// </summary>
				/// <param name="{Config}">A Config with mouse data (see header top lines).</param>
				/// <param name="{bool}">Do update even though this is a copy (if it is)?</param>
				void update_and_clear(const Interface::Config&, const bool = false);
			};
		}
	}
}