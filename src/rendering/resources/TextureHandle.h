#ifndef TEXTURE_HANDLE_H
#define TEXTURE_HANDLE_H

#include <string>
#include <optional>

#include <glm/glm.hpp>
#include "utility/HelperTypes.h"

class TextureLoader;

/// A class representing a handle to a loaded texture, also storing some of its configuration data.
class TextureHandle : private NonCopyable {
    uint texture_id;
    uint width;
    uint height;

    bool srgb = true;
    bool flipped = false;
    std::optional<std::string> filename{};

    friend class TextureLoader;

public:
    TextureHandle(uint texture_id, uint width, uint height, bool srgb = true, bool flipped = false, std::optional<std::string> filename = {});

    [[nodiscard]] uint get_texture_id() const;
    [[nodiscard]] glm::uvec2 get_size() const;
    [[nodiscard]] uint get_width() const;
    [[nodiscard]] uint get_height() const;

    [[nodiscard]] bool is_flipped() const;
    [[nodiscard]] bool is_srgb() const;
    [[nodiscard]] const std::optional<std::string>& get_filename() const;

    virtual ~TextureHandle();
};


#endif //TEXTURE_HANDLE_H
