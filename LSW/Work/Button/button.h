#pragma once

#include "../Block/block.h"
#include "../Text/text.h"
#include "../Sprite/sprite.h"

#undef max

namespace LSW {
	namespace v5 {
		namespace Work {

			/// <summary>
			/// <para>Button is a shortcut to have one Block and one Text linked and ready.</para>
			/// <para>You can do the exact same thing creating them both and then setting Text to follow the Block.</para>
			/// <para>———————————————————————————————————————————————————————</para>
			/// <para># Things you SHOULD NOT change:</para>
			/// <para>- text::e_sprite_ref::FOLLOWING (Text part)</para>
			/// </summary>
			class Button : protected Text, protected Block {
			public:
				using Block::collide;
				using Block::update_and_clear;

				/// <summary>
				/// <para>Empty default start.</para>
				/// </summary>
				Button();

				/// <summary>
				/// <para>Constructor referencing a Button (not a copy).</para>
				/// </summary>
				/// <param name="{Button}">The one to reference attributes from (no copy).</param>
				Button(const Button&);

				/// <summary>
				/// <para>Constructor to move a Button to this (move).</para>
				/// </summary>
				/// <param name="{Button}">The one to move attributes from.</param>
				Button(Button&&);

				/// <summary>
				/// <para>Reference a Button (not a copy).</para>
				/// </summary>
				/// <param name="{Button}">The one to reference attributes from (no copy).</param>
				void operator=(const Button&);

				/// <summary>
				/// <para>Move a Button to this (move).</para>
				/// </summary>
				/// <param name="{Button}">The one to move attributes from.</param>
				void operator=(Button&&);

				/// <summary>
				/// <para>Clone other Button attributes.</para>
				/// <para>You can also clone a specific type by doing set(*get()).</para>
				/// </summary>
				/// <param name="{Button}">Other to copy from.</param>
				void clone(const Button&);

				/// <summary>
				/// <para>Call both Text and Block draw calls.</para>
				/// </summary>
				/// <param name="{Camera}">Reference camera used in draw, properties may change it to default.</param>
				/// <param name="{bool}">Do draw even though this is a copy (if it is)?</param>
				void draw(const Interface::Camera&, const bool = false);

				/// <summary>
				/// <para>Specific non sprite:: setting? Go ahead.</para>
				/// </summary>
				/// <returns>{Block} A reference to the Block inside.</returns>
				Block& get_block();

				/// <summary>
				/// <para>Specific non sprite:: setting? Go ahead.</para>
				/// </summary>
				/// <returns>{Text} A reference to the Text inside.</returns>
				Text& get_text();
			};

		}
	}
}
