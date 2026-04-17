#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <algorithm>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>

#include "TextureHandle.h"

/// A loader class intended for the use of loading textures from disk. Includes caching functionality.
class TextureLoader {
    std::string import_path;

    static constexpr int DEFAULT_TEXTURE_SIZE = 16;
    static constexpr int DEFAULT_TEXTURE_BPP = 3;
    static constexpr int DEFAULT_TEXTURE_LEN = DEFAULT_TEXTURE_SIZE * DEFAULT_TEXTURE_SIZE * DEFAULT_TEXTURE_BPP;
    unsigned char default_white_texture_data[DEFAULT_TEXTURE_LEN]{};
    unsigned char default_black_texture_data[DEFAULT_TEXTURE_LEN]{};

    std::shared_ptr<TextureHandle> default_white_texture_cache{};
    std::shared_ptr<TextureHandle> default_black_texture_cache{};
    std::unordered_set<std::string> special_names;

    std::optional<std::vector<std::string>> available_textures{};

    // Map (relative_path, srgb, is_flipped) -> (last_modified, weak_handle)
    std::unordered_map<std::tuple<std::string, bool, bool>, std::pair<std::filesystem::file_time_type, std::weak_ptr<TextureHandle>>, TripleHash> cache{};
public:
    /// Construct the loader with a import_path which is prepended to any path you try and load.
    /// It also scans the directory for all files, which is used to populate the list of get_available_textures()
    explicit TextureLoader(std::string import_path);

    /// Loads the file at the specified path into GPU memory, with flags for if the texture is sRGB and to flip it vertically.
    std::shared_ptr<TextureHandle> load_from_file(const std::string& file, bool srgb = true, bool flip_vertical = false);

    /// Provides a pure white (0xFFFFFF) texture
    std::shared_ptr<TextureHandle> default_white_texture();
    /// Provides a pure black (0x000000) texture
    std::shared_ptr<TextureHandle> default_black_texture();

    /// Helper method to provide a selector over all the texture files in the import_path directory.
    /// If the prefer_srgb flag is selected, then when going from no texture to a valid texture it will default to enabling srgb.
    void add_imgui_texture_selector(const std::string& caption, std::shared_ptr<TextureHandle>& texture_handle, bool prefer_srgb = true);
    /// Helper method to provide a selector over all the texture files in the import_path directory.
    /// if force_refresh is selected, it will rescan the directory, otherwise it just uses a cached list from the last scan.
    const std::vector<std::string>& get_available_textures(bool force_refresh = false);

    /// Free up any resources.
    void cleanup();
};


#endif //TEXTURE_LOADER_H
