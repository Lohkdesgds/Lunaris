#pragma once

#include <Lunaris/__macro/macros.h>

#include <Lunaris/Graphics/transform.h>
#include <Lunaris/Graphics/color.h>
#include <Lunaris/Graphics/display.h> // depends on EVENT
#include <Lunaris/Graphics/menu.h> // depends on DISPLAY, EVENT
#include <Lunaris/Graphics/texture.h> // depends on COLOR
#include <Lunaris/Graphics/vertex.h> // depends on COLOR, TEXTURE
#include <Lunaris/Graphics/font.h> // depends on COLOR, TEXTURE
#include <Lunaris/Graphics/sprite.h> // depends on TEXTURE, TRANSFORM, COLOR, Utility/memory, Utility/multi_map
#include <Lunaris/Graphics/block.h>  // depends on SPRITE, TRANSFORM
#include <Lunaris/Graphics/text.h>   // depends on SPRITE, TRANSFORM, BLOCK, UTILITY::SAFE_DATA
#include <Lunaris/Graphics/collisionable.h> // depends on SPRITE