#include "TextureHandle.h"

#include <glad/gl.h>

TextureHandle::TextureHandle(uint texture_id, uint width, uint height, bool srgb, bool flipped, std::optional<std::string> filename) : texture_id(texture_id), width(width), height(height), srgb(srgb), flipped(flipped), filename(std::move(filename)) {}

uint TextureHandle::get_texture_id() const {
    return texture_id;
}

glm::uvec2 TextureHandle::get_size() const {
    return {width, height};
}

uint TextureHandle::get_width() const {
    return width;
}

uint TextureHandle::get_height() const {
    return height;
}

bool TextureHandle::is_srgb() const {
    return srgb;
}

bool TextureHandle::is_flipped() const {
    return flipped;
}

const std::optional<std::string>& TextureHandle::get_filename() const {
    return filename;
}

TextureHandle::~TextureHandle() {
    glDeleteTextures(1, &texture_id);
}
