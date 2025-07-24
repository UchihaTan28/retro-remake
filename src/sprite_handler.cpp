#include "sprite_handler.hpp"

Sprite::Sprite(const std::string &filePath)
  : texture_( LoadTexture(filePath.c_str()) )
{
    // initialise the full‐texture rect
    sourceRect_ = { 0, 0, float(texture_.width), float(texture_.height) };
}