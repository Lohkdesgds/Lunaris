#include "button.h"

namespace LSW {
	namespace v5 {
		namespace Work {

			Button::Button() : Block(), Text()
			{
				this->Text::set<Sprite_Base>(text::e_sprite_ref::FOLLOWING, (Block)*this);
			}

			Button::Button(const Button& other) : Block(other), Text(other)
			{
				this->Text::set<Sprite_Base>(text::e_sprite_ref::FOLLOWING, (Block)*this);
			}

			Button::Button(Button&& other) : Block(std::move((Block)other)), Text(std::move((Text)other))
			{
				this->Text::set<Sprite_Base>(text::e_sprite_ref::FOLLOWING, (Block)*this);
			}

			void Button::operator=(const Button& other)
			{
				this->Block::operator=((Block)other);
				this->Text::operator=((Text)other);
				this->Text::set<Sprite_Base>(text::e_sprite_ref::FOLLOWING, (Block)*this);
			}

			void Button::operator=(Button&& other)
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
				return *this;
			}

			Text& Button::get_text()
			{
				return *this;
			}

			// implementation surely is done on Block and Text, this is a "merged" class

		}
	}
}