#include "TextureLoader.h"

#include <iostream>
#include <filesystem>

#include <imgui/imgui.h>

#include <stb/stb_image.h>
#include <glad/gl.h>

#define WHITE_TEXTURE_NAME "[WHITE]"
#define BLACK_TEXTURE_NAME "[BLACK]"

TextureLoader::TextureLoader(std::string import_path) : import_path(std::move(import_path)), special_names({WHITE_TEXTURE_NAME, BLACK_TEXTURE_NAME}) {
    std::fill_n(default_white_texture_data, DEFAULT_TEXTURE_LEN, (unsigned char) 0xFF);
}

float get_max_anisotropy() {
    float max_ani = 1.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_ani);
    std::cout << "Max Anisotropic Filtering: " << max_ani << std::endl;
    return max_ani;
}

std::shared_ptr<TextureHandle> TextureLoader::load_from_file(const std::string& file, bool srgb, bool flip_vertical) {
    if (file == WHITE_TEXTURE_NAME) {
        auto white = default_white_texture();
        white->flipped = flip_vertical;
        return white;
    };
    if (file == BLACK_TEXTURE_NAME) {
        auto black = default_black_texture();
        black->flipped = flip_vertical;
        return black;
    };

    std::string full_path = import_path + "/" + file;

    if (!std::filesystem::exists(full_path)) {
        throw std::runtime_error(Formatter() << "Failed to load texture file: " << full_path << "\n\t Reason: File does not exist");
    }

    static float max_ani = get_max_anisotropy();
    auto last_write_time = std::filesystem::last_write_time(full_path);

    auto existing = cache.find({file, srgb, flip_vertical});
    if (existing != cache.end()) {
        // Cache exist, so try lock
        auto handle = existing->second.second.lock();
        if (handle != nullptr && existing->second.first >= last_write_time) {
            // Lock was successful and the cache is for an up-to-date version of the file, so can use it
            return handle;
        }
    }

    stbi_set_flip_vertically_on_load(flip_vertical);

    int width, height;
    stbi_uc* data = stbi_load(full_path.c_str(), &width, &height, nullptr, STBI_rgb);
    if (!data) {
        throw std::runtime_error(Formatter() << "Failed to load texture file: " << full_path << "\n\t Reason: " << stbi_failure_reason());
    }

    uint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, max_ani);

    glTexImage2D(GL_TEXTURE_2D, 0, srgb ? GL_SRGB : GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    auto texture = std::make_shared<TextureHandle>(texture_id, width, height, srgb, flip_vertical, file);

    cache[{file, srgb, flip_vertical}] = {last_write_time, texture};

    return texture;
}

std::shared_ptr<TextureHandle> TextureLoader::default_white_texture() {
    if (default_white_texture_cache != nullptr) return default_white_texture_cache;

    uint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, DEFAULT_TEXTURE_SIZE, DEFAULT_TEXTURE_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, &default_white_texture_data[0]);

    default_white_texture_cache = std::make_shared<TextureHandle>(texture_id, DEFAULT_TEXTURE_SIZE, DEFAULT_TEXTURE_SIZE, false, false, WHITE_TEXTURE_NAME);
    return default_white_texture_cache;
}

std::shared_ptr<TextureHandle> TextureLoader::default_black_texture() {
    if (default_black_texture_cache != nullptr) return default_black_texture_cache;

    uint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, DEFAULT_TEXTURE_SIZE, DEFAULT_TEXTURE_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, &default_black_texture_data[0]);

    default_black_texture_cache = std::make_shared<TextureHandle>(texture_id, DEFAULT_TEXTURE_SIZE, DEFAULT_TEXTURE_SIZE, false, false, BLACK_TEXTURE_NAME);
    return default_black_texture_cache;
}

void TextureLoader::cleanup() {
    default_black_texture_cache = nullptr;
    default_white_texture_cache = nullptr;
}

void TextureLoader::add_imgui_texture_selector(const std::string& caption, std::shared_ptr<TextureHandle>& texture_handle, bool prefer_srgb) {
    std::string current_selection = texture_handle->get_filename().value_or("Generated Texture");

    bool is_file = texture_handle->get_filename().has_value() && special_names.count(texture_handle->get_filename().value()) == 0;

    if (!is_file) ImGui::BeginDisabled();

    bool is_rgb = texture_handle->is_srgb();
    bool is_flipped = texture_handle->is_flipped();

    std::string srgb_checkbox_label = Formatter() << "sRGB##" << caption;
    bool update_param = ImGui::Checkbox(srgb_checkbox_label.c_str(), &is_rgb);

    ImGui::SameLine();

    std::string flip_checkbox_label = Formatter() << "Flip Y##" << caption;
    update_param |= ImGui::Checkbox(flip_checkbox_label.c_str(), &is_flipped);

    if (update_param && is_file) {
        try {
            texture_handle = load_from_file(texture_handle->get_filename().value(), is_rgb, is_flipped);
        } catch (const std::exception& e) {
            std::cerr << "Error while trying to update texture parameters:" << std::endl;
            std::cerr << e.what() << std::endl;
        }
    }

    if (!is_file) ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::PushItemWidth(ImGui::CalcItemWidth() - 132);

    static bool just_opened = true;
    if (ImGui::BeginCombo(caption.c_str(), current_selection.c_str(), 0)) {
        const auto& textures = get_available_textures(just_opened);
        just_opened = false;

        for (const auto& texture: textures) {
            const bool is_selected = texture_handle->get_filename().has_value() && current_selection == texture;
            if (ImGui::Selectable(texture.c_str(), is_selected)) {
                bool was_srgb = texture_handle->is_srgb();
                bool was_flipped = texture_handle->is_flipped();
                bool was_special = texture_handle->filename.has_value() && special_names.count(texture_handle->filename.value()) != 0;
                try {
                    texture_handle = load_from_file(texture, was_srgb || (prefer_srgb && was_special), was_flipped);
                } catch (const std::exception& e) {
                    std::cerr << "Error while trying to update texture file:" << std::endl;
                    std::cerr << e.what() << std::endl;
                }
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    } else {
        just_opened = true;
    }

    ImGui::PopItemWidth();
}

const std::vector<std::string>& TextureLoader::get_available_textures(bool force_refresh) {
    if (!force_refresh && available_textures.has_value()) {
        return available_textures.value();
    }
    available_textures = std::vector<std::string>{};
    available_textures->push_back(WHITE_TEXTURE_NAME);
    available_textures->push_back(BLACK_TEXTURE_NAME);

    for (auto const& dir_entry: std::filesystem::recursive_directory_iterator(import_path)) {
        available_textures->push_back(std::filesystem::relative(dir_entry, import_path).string());
    }
    std::sort(available_textures.value().begin() + 2, available_textures.value().end());

    return available_textures.value();
}
