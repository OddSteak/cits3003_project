#include "ModelLoader.h"
#include <filesystem>

const std::vector<std::string>& ModelLoader::get_available_models(bool force_refresh) {
    if (!force_refresh && available_models.has_value()) {
        return available_models.value();
    }
    available_models = std::vector<std::string>{};

    for (auto const& dir_entry: std::filesystem::recursive_directory_iterator(import_path)) {
        available_models->push_back(std::filesystem::relative(dir_entry, import_path).string());
    }
    std::sort(available_models.value().begin(), available_models.value().end());

    return available_models.value();
}