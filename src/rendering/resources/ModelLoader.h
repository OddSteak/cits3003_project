#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <map>
#include <set>
#include <utility>
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include <typeindex>
#include <filesystem>
#include <unordered_set>

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <imgui/imgui.h>

#include "ModelHandle.h"
#include "MeshHierarchy.h"

struct VertexCollection {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> tex_coords;
    // [(bone_weights, bone_indices)]
    std::vector<std::pair<glm::vec4, glm::uvec4>> bones;
};

/// A loader class intended for the use of loading models from disk. Includes caching functionality.
class ModelLoader {
    std::string import_path;
    Assimp::Importer importer{};

    std::optional<std::vector<std::string>> available_models{};

    // Map (relative_path, vertex_type) -> (last_modified, weak_handle)
    std::unordered_map<std::pair<std::string, std::type_index>, std::pair<std::filesystem::file_time_type, std::weak_ptr<BaseModelHandle>>, PairHash> cache{};
    std::unordered_map<std::pair<std::string, std::type_index>, std::pair<std::filesystem::file_time_type, std::weak_ptr<BaseMeshHierarchy>>, PairHash> hierarchy_cache{};
public:
    /// Construct the loader with a import_path which is prepended to any path you try and load.
    /// It also scans the directory for all files, which is used to populate the list of get_available_models()
    explicit ModelLoader(std::string import_path) : import_path(std::move(import_path)) {}

    /// Loads the provided model data into GPU memory
    template<typename VertexData>
    static std::shared_ptr<ModelHandle<VertexData>> load_from_data(const std::vector<VertexData>& vertices, const std::vector<uint>& indices, std::optional<std::string> filename = {});

    /// Loads the file specified from disk into GPU memory
    template<typename VertexData>
    std::shared_ptr<ModelHandle<VertexData>> load_from_file(const std::string& file);

    /// Load the file specified, as a hierarchy of meshes, for use with animated models.
    template<typename VertexData>
    std::shared_ptr<MeshHierarchy<VertexData>> load_hierarchy_from_file(const std::string& file);

    /// Helper method to provide a selector over all the model files in the import_path directory.
    template<typename VertexData>
    bool add_imgui_model_selector(const std::string& caption, std::shared_ptr<ModelHandle<VertexData>>& model_handle);

    /// Helper method to provide a selector over all the model files in the import_path directory, but to be loaded as a hierarchy.
    template<typename VertexData>
    bool add_imgui_hierarchy_selector(const std::string& caption, std::shared_ptr<MeshHierarchy<VertexData>>& mesh_hierarchy);

    /// Helper method to provide a selector over all the model files in the import_path directory.
    /// if force_refresh is selected, it will rescan the directory, otherwise it just uses a cached list from the last scan.
    const std::vector<std::string>& get_available_models(bool force_refresh = false);

    /// Free up any resources.
    void cleanup() {}

private:
    template<typename VertexData>
    static void load_node(const aiScene* scene, const aiNode* node, std::vector<VertexData>& vertices, std::vector<uint>& indices, glm::mat4 parent_transform);
};

template<typename VertexData>
std::shared_ptr<ModelHandle<VertexData>> ModelLoader::load_from_data(const std::vector<VertexData>& vertices, const std::vector<uint>& indices, std::optional<std::string> filename) {
    uint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    uint vertex_vbo;
    glGenBuffers(1, &vertex_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
    glBufferData(GL_ARRAY_BUFFER, (long) (sizeof(VertexData) * vertices.size()), vertices.data(), GL_STATIC_DRAW);
    VertexData::setup_attrib_pointers();

    uint index_vbo;
    glGenBuffers(1, &index_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long) (sizeof(uint) * indices.size()), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return std::make_shared<ModelHandle<VertexData>>(vertex_vbo, index_vbo, vao, (int) indices.size(), 0, std::move(filename));
}

template<typename VertexData>
std::shared_ptr<ModelHandle<VertexData>> ModelLoader::load_from_file(const std::string& file) {
    auto path = import_path + "/" + file;
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error(Formatter() << "Failed to load model (" << path << "): \n\t File does not exist");
    }

    auto last_write_time = std::filesystem::last_write_time(path);

    auto existing = cache.find({file, std::type_index(typeid(VertexData))});
    if (existing != cache.end()) {
        // Cache exist, so try lock
        auto handle = existing->second.second.lock();
        if (handle != nullptr && existing->second.first >= last_write_time) {
            // Lock was successful and the cache is for an up-to-date version of the file, so can use it
            return std::dynamic_pointer_cast<ModelHandle<VertexData>>(handle);
        }
    }

    const aiScene* scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_TransformUVCoords | aiProcess_SortByPType);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error(Formatter() << "Failed to load model (" << file << "): \n\t" << importer.GetErrorString());
    }

    if (scene->mNumMeshes == 0) {
        throw std::runtime_error(Formatter() << "Failed to load model (" << file << "): \n\t" << "No meshes");
    }

    auto triangle_meshes = 0;
    for (auto i = 0u; i < scene->mNumMeshes; ++i) {
        if ((scene->mMeshes[i]->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) != 0) {
            triangle_meshes++;
        }
    }

    if (triangle_meshes == 0) {
        throw std::runtime_error(Formatter() << "Failed to load model (" << file << "): \n\t" << "No triangle meshes");
    }

    std::vector<VertexData> vertices{};
    std::vector<uint> indices{};

    load_node(scene, scene->mRootNode, vertices, indices, glm::mat4{1.0f});

    auto model = load_from_data(vertices, indices, file);

    importer.FreeScene();

    cache[{file, std::type_index(typeid(VertexData))}] = {last_write_time, model};

    return model;
}

template<typename VertexData>
void ModelLoader::load_node(const aiScene* scene, const aiNode* node, std::vector<VertexData>& vertices, std::vector<uint>& indices, glm::mat4 parent_transform) {
    glm::mat4 node_transform;
    {
        auto node_transform_ai = node->mTransformation;
        node_transform = reinterpret_cast<glm::mat4&>(node_transform_ai.Transpose());
    }

    // Post-multiply by node_transform since it is relative to parent and should be applied before it.
    glm::mat4 total_transform = parent_transform * node_transform;
    // Calculate a normal matrix so that non-uniform scale transformations properly transform normals
    // See: https://github.com/graphitemaster/normals_revisited
    // and: https://gist.github.com/shakesoda/8485880f71010b79bc8fed0f166dabac
    glm::mat3 normal_matrix = glm::mat3(
        glm::cross(glm::vec3(total_transform[1]), glm::vec3(total_transform[2])),
        glm::cross(glm::vec3(total_transform[2]), glm::vec3(total_transform[0])),
        glm::cross(glm::vec3(total_transform[0]), glm::vec3(total_transform[1]))
    );

    for (auto mesh_i = 0u; mesh_i < node->mNumMeshes; ++mesh_i) {
        auto index_offset = (uint) vertices.size();
        const auto mesh = scene->mMeshes[node->mMeshes[mesh_i]];
        if ((mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) == 0) continue;

        const auto v = reinterpret_cast<glm::vec3*>(mesh->mVertices);
        const auto n = reinterpret_cast<glm::vec3*>(mesh->mNormals);
        const auto t = reinterpret_cast<glm::vec3*>(mesh->mTextureCoords[0]);

        VertexCollection vertex_collection{
            v ? std::vector<glm::vec3>{v, v + mesh->mNumVertices} : std::vector<glm::vec3>{},
            n ? std::vector<glm::vec3>{n, n + mesh->mNumVertices} : std::vector<glm::vec3>{},
            t ? std::vector<glm::vec2>{t, t + mesh->mNumVertices} : std::vector<glm::vec2>{},
            {}
        };

        for (auto& position: vertex_collection.positions) {
            position = total_transform * glm::vec4(position, 1.0f);
        }

        for (auto& normal: vertex_collection.normals) {
            normal = normal_matrix * normal;
        }

        VertexData::from_mesh(vertex_collection, vertices);

        for (auto i = 0u; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (auto j = 0u; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j] + index_offset);
            }
        }
    }

    for (auto i = 0u; i < node->mNumChildren; ++i) {
        load_node(scene, node->mChildren[i], vertices, indices, total_transform);
    }
}

template<typename VertexData>
std::shared_ptr<MeshHierarchy<VertexData>> ModelLoader::load_hierarchy_from_file(const std::string& file) {
    auto path = import_path + "/" + file;
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error(Formatter() << "Failed to load model (" << path << "): \n\t File does not exist");
    }
    auto last_write_time = std::filesystem::last_write_time(path);

    auto existing = hierarchy_cache.find({file, std::type_index(typeid(VertexData))});
    if (existing != hierarchy_cache.end()) {
        // Cache exist, so try lock
        auto handle = existing->second.second.lock();
        if (handle != nullptr && existing->second.first >= last_write_time) {
            // Lock was successful and the cache is for an up-to-date version of the file, so can use it
            return std::dynamic_pointer_cast<MeshHierarchy<VertexData>>(handle);
        }
    }

    const aiScene* scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_TransformUVCoords | aiProcess_SortByPType);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error(Formatter() << "Failed to load model (" << file << "): \n\t" << importer.GetErrorString());
    }

    if (scene->mNumMeshes == 0) {
        throw std::runtime_error(Formatter() << "Failed to load model (" << file << "): \n\t" << "No meshes");
    }

    auto mesh_hierarchy = std::make_shared<MeshHierarchy<VertexData>>(file);

    // {index into scene->mMeshes} -> {index into mesh_hierarchy->models}
    std::unordered_map<uint, uint> mesh_index_map{};

    for (auto mesh_i = 0u; mesh_i < scene->mNumMeshes; ++mesh_i) {
        const auto* mesh = scene->mMeshes[mesh_i];
        if ((mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) == 0) {
            continue;
        }

        const auto v = reinterpret_cast<glm::vec3*>(mesh->mVertices);
        const auto n = reinterpret_cast<glm::vec3*>(mesh->mNormals);
        const auto t = reinterpret_cast<glm::vec3*>(mesh->mTextureCoords[0]);

        // { bone_name } -> { bone_id }
        std::unordered_map<std::string, uint> bone_names{};

        // [vertex_id] -> (bone_weight -> bone_id)
        std::vector<std::map<float, std::set<uint>>> bone_weights_total{};
        bone_weights_total.resize(mesh->mNumVertices, {});
        for (auto bone_i = 0u; bone_i < mesh->mNumBones; ++bone_i) {
            const auto* bone = mesh->mBones[bone_i];
            bone_names[bone->mName.C_Str()] = bone_i;
            auto ai_offset_matrix = bone->mOffsetMatrix;
            mesh_hierarchy->total_bones[bone->mName.C_Str()].push_back({mesh_i, bone_i, reinterpret_cast<glm::mat4&>(ai_offset_matrix.Transpose())});

            for (auto weight_i = 0u; weight_i < bone->mNumWeights; ++weight_i) {
                const auto* weight = &bone->mWeights[weight_i];
                bone_weights_total[weight->mVertexId][weight->mWeight].insert(bone_i);
            }
        }

        // [vertex_id] -> ([bone_id; 4], [bone_weight; 4])
        std::vector<std::pair<glm::vec4, glm::uvec4>> bone_weights;
        bone_weights.resize(mesh->mNumVertices, {});
        for (auto vert_i = 0u; vert_i < mesh->mNumVertices; ++vert_i) {
            const auto& vert = bone_weights_total[vert_i];
            auto i = 0;
            for (auto iter = vert.rbegin(); i < 4 && iter != vert.rend(); ++iter) {
                for (auto inner_iter = iter->second.begin(); i < 4 && inner_iter != iter->second.end(); ++inner_iter) {
                    bone_weights[vert_i].first[i] = iter->first;
                    bone_weights[vert_i].second[i] = *inner_iter;
                    ++i;
                }
            }
            float weight_sum = glm::compAdd(bone_weights[vert_i].first);
            if (weight_sum != 0.0f) {
                // Normalise the sum of the weights
                bone_weights[vert_i].first /= weight_sum;
            }
        }

        VertexCollection vertex_collection{
            v ? std::vector<glm::vec3>{v, v + mesh->mNumVertices} : std::vector<glm::vec3>{},
            n ? std::vector<glm::vec3>{n, n + mesh->mNumVertices} : std::vector<glm::vec3>{},
            t ? std::vector<glm::vec2>{t, t + mesh->mNumVertices} : std::vector<glm::vec2>{},
            bone_weights
        };

        std::vector<VertexData> vertices{};
        VertexData::from_mesh(vertex_collection, vertices);

        std::vector<uint> indices{};
        for (auto face_i = 0u; face_i < mesh->mNumFaces; ++face_i) {
            aiFace face = mesh->mFaces[face_i];
            indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
        }

        mesh_index_map[mesh_i] = (int) mesh_hierarchy->meshes.size();
        mesh_hierarchy->meshes.push_back(ModelInfo{
            load_from_data(vertices, indices),
            bone_names
        });
    }

    if (mesh_hierarchy->meshes.empty()) {
        throw std::runtime_error(Formatter() << "Failed to load model (" << file << "): \n\t" << "No triangle meshes");
    }

    // { node_name } -> [(animation_id, node_animation)]
    std::unordered_map<std::string, std::vector<std::pair<uint, const aiNodeAnim*>>> animations{};
    for (auto animation_i = 0u; animation_i < scene->mNumAnimations; ++animation_i) {
        const auto* animation = scene->mAnimations[animation_i];
        std::string name = animation->mName.C_Str();
        double ticks_per_second = animation->mTicksPerSecond;
        // Default to "[Unnamed] ({id})", in case file doesn't specify
        // Default to 1 tick-per-second, in case file doesn't specify
        mesh_hierarchy->animations.emplace_back(name.empty() ? Formatter() << "[Unnamed] (" << animation_i << ")" : name, ticks_per_second == 0.0 ? 1.0 : ticks_per_second, animation->mDuration);

        for (auto channel_i = 0u; channel_i < animation->mNumChannels; ++channel_i) {
            const auto* node_animation = animation->mChannels[channel_i];
            animations[node_animation->mNodeName.C_Str()].emplace_back(animation_i, node_animation);
        }
    }

    std::function<void(const aiNode* node, MeshHierarchyNode& hierarchy_node)> load_hierarchy_node;

    load_hierarchy_node = [&mesh_index_map, &load_hierarchy_node, &mesh_hierarchy, &animations](const aiNode* node, MeshHierarchyNode& hierarchy_node) {
        auto ai_transformation = node->mTransformation;
        hierarchy_node.transformation = reinterpret_cast<glm::mat4&>(ai_transformation.Transpose());
        for (auto mesh_i = 0u; mesh_i < node->mNumMeshes; ++mesh_i) {
            hierarchy_node.meshes.push_back(mesh_index_map[node->mMeshes[mesh_i]]);
        }
        const auto& bones = mesh_hierarchy->total_bones[node->mName.C_Str()];
        hierarchy_node.bones.insert(hierarchy_node.bones.end(), bones.begin(), bones.end());

        const auto animation = animations.find(node->mName.C_Str());
        if (animation != animations.end()) {
            for (const auto& [animation_id, node_animation]: animation->second) {
                auto& animation_data = hierarchy_node.animation_data[animation_id];
                for (auto i = 0u; i < node_animation->mNumPositionKeys; ++i) {
                    const auto& key = node_animation->mPositionKeys[i];
                    animation_data.positions[key.mTime] = glm::vec3{key.mValue.x, key.mValue.y, key.mValue.z};
                }
                for (auto i = 0u; i < node_animation->mNumRotationKeys; ++i) {
                    const auto& key = node_animation->mRotationKeys[i];
                    animation_data.rotations[key.mTime] = glm::quat{key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z};
                }
                for (auto i = 0u; i < node_animation->mNumScalingKeys; ++i) {
                    const auto& key = node_animation->mScalingKeys[i];
                    animation_data.scalings[key.mTime] = glm::vec3{key.mValue.x, key.mValue.y, key.mValue.z};
                }
            }
        }

        hierarchy_node.children.resize(node->mNumChildren);
        for (auto child_i = 0u; child_i < node->mNumChildren; ++child_i) {
            load_hierarchy_node(node->mChildren[child_i], hierarchy_node.children[child_i]);
        }
    };

    load_hierarchy_node(scene->mRootNode, mesh_hierarchy->root_node);

    importer.FreeScene();

    hierarchy_cache[{file, std::type_index(typeid(VertexData))}] = {last_write_time, mesh_hierarchy};

    return mesh_hierarchy;
}

template<typename VertexData>
bool ModelLoader::add_imgui_model_selector(const std::string& caption, std::shared_ptr<ModelHandle<VertexData>>& model_handle) {
    std::string current_selection = model_handle->get_filename().value_or("Generated Model");

    bool changed = false;
    static bool just_opened = true;
    if (ImGui::BeginCombo(caption.c_str(), current_selection.c_str(), 0)) {
        const auto& models = get_available_models(just_opened);
        just_opened = false;

        for (const auto& model: models) {
            const bool is_selected = model_handle->get_filename().has_value() && current_selection == model;
            if (ImGui::Selectable(model.c_str(), is_selected)) {
                try {
                    model_handle = load_from_file<VertexData>(model);
                    changed = true;
                } catch (const std::exception& e) {
                    std::cerr << "Error while trying to update model file:" << std::endl;
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

    return changed;
}

template<typename VertexData>
bool ModelLoader::add_imgui_hierarchy_selector(const std::string& caption, std::shared_ptr<MeshHierarchy<VertexData>>& mesh_hierarchy) {
    std::string current_selection = mesh_hierarchy->filename.value_or("Generated Model");

    bool changed = false;
    static bool just_opened = true;
    if (ImGui::BeginCombo(caption.c_str(), current_selection.c_str(), 0)) {
        const auto& models = get_available_models(just_opened);
        just_opened = false;

        for (const auto& model: models) {
            const bool is_selected = mesh_hierarchy->filename.has_value() && current_selection == model;
            if (ImGui::Selectable(model.c_str(), is_selected)) {
                try {
                    mesh_hierarchy = load_hierarchy_from_file<VertexData>(model);
                    changed = true;
                } catch (const std::exception& e) {
                    std::cerr << "Error while trying to update model hierarchy file:" << std::endl;
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

    return changed;
}

#endif //MODEL_LOADER_H
