#include "button.h"

namespace LSW {
	namespace v5 {
		namespace Work {

			void Button::think_task(const int c)
			{
				//get_block().think_task(c); // there is no think_task
				this->Text::think_task(c);
			}

			/*void Button::draw_task(Interface::Camera& c)
			{
				this->Block::draw_task(c);
				this->Text::draw_task(c);
			}*/

			Button::Button() : Block(), Text()
			{
				this->Text::set<Sprite_Base>(text::e_sprite_ref::FOLLOWING, (Block)*this);
			}

			Button::Button(const Button& other) : Block(other), Text(other)
			{
				this->Text::set<Sprite_Base>(text::e_sprite_ref::FOLLOWING, (Block)*this);
			}

			Button::Button(Button&& other) noexcept : Block(std::move((Block)other)), Text(std::move((Text)other))
			{
				this->Text::set<Sprite_Base>(text::e_sprite_ref::FOLLOWING, (Block)*this);
			}

			void Button::operator=(const Button& other)
			{
				this->Block::operator=((Block)other);
				this->Text::operator=((Text)other);
				this->Text::set<Sprite_Base>(text::e_sprite_ref::FOLLOWING, (Block)*this);
			}

			void Button::operator=(Button&& other) noexcept
			{
				this->Block::operator=(std::move((Block)other));
				this->Text::operator=(std::move((Text)other));
				this->Text::set<Sprite_Base>(text::e_sprite_ref::FOLLOWING, (Block)*this);
			}

			void Button::clone(const Button& other)
			{
				this->Block::clone((Block)other);
				this->Text::clone((Text)other);
				this->Text::set<Sprite_Base>(text::e_sprite_ref::FOLLOWING, (Block)*this);
			}

			void Button::draw(const Interface::Camera& c, const bool ign)
			{
				get_block().draw(c, ign);
				get_text().draw(c, ign);
			}

			Block& Button::get_block()
			{
				return *(Block*)this;
			}

			Text& Button::get_text()
			{
				return *(Text*)this;
			}

			// implementation surely is done on Block and Text, this is a "merged" class

		}
	}
}