#pragma once

#include "Graphics/transform.h"
#include "Graphics/color.h"
#include "Graphics/display.h"
#include "Graphics/texture.h" // depends on COLOR
#include "Graphics/font.h" // depends on COLOR
#include "Graphics/sprite.h" // depends on TEXTURE, TRANSFORM, COLOR, Utility/memory, Utility/multi_map
#include "Graphics/block.h"  // depends on SPRITE, TRANSFORM
#include "Graphics/text.h"   // depends on SPRITE, TRANSFORM, BLOCK, UTILITY::SAFE_DATA
#include "Graphics/collisionable.h" // depends on SPRITE