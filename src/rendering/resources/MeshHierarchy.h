#ifndef MESH_HIERARCHY_H
#define MESH_HIERARCHY_H

#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ModelHandle.h"

#define NONE_ANIMATION UINT_MAX

struct AnimationData {
    std::map<double, glm::vec3> positions{};
    std::map<double, glm::quat> rotations{};
    std::map<double, glm::vec3> scalings{};

    [[nodiscard]] glm::mat4 sample(double time) const;
};

struct MeshHierarchyNode {
    std::vector<uint> meshes{};
    glm::mat4 transformation{1.0f};
    // [(mesh_index, bone_id, offset_matrix)]
    std::vector<std::tuple<uint, uint, glm::mat4>> bones{};
    // { animation_id } -> { Animation Data }
    std::unordered_map<int, AnimationData> animation_data{};
    std::vector<MeshHierarchyNode> children{};
};

template<typename VertexData>
struct ModelInfo {
    std::shared_ptr<ModelHandle<VertexData>> model{};
    // { bone_name } -> { bone_id }
    std::unordered_map<std::string, uint> bones{};
    // Computed based on an input time
    std::vector<glm::mat4> bone_transforms{};

    ModelInfo(const std::shared_ptr<ModelHandle<VertexData>>& model, const std::unordered_map<std::string, uint>& bones) : model(model), bones(bones) {
        bone_transforms.resize(bones.size(), glm::mat4{1.0f});
    }
};

class BaseMeshHierarchy : private NonCopyable {
public:
    virtual ~BaseMeshHierarchy() = default;
};

/// A struct representing a hierarchy of meshes, for use in animation.
template<typename VertexData>
struct MeshHierarchy : public BaseMeshHierarchy {
    std::vector<ModelInfo<VertexData>> meshes{};
    // { bone_name } -> [(mesh_index, bone_id, offset_matrix)]
    std::unordered_map<std::string, std::vector<std::tuple<uint, uint, glm::mat4>>> total_bones{};
    // [animation_id] -> (animation_name, ticks_per_second, duration_ticks)
    std::vector<std::tuple<std::string, double, double>> animations{};
    // The name of the file the MeshHierarchy was loaded from, if any
    std::optional<std::string> filename{};
    MeshHierarchyNode root_node{};

    explicit MeshHierarchy(const std::optional<std::string>& filename = std::nullopt) : filename(filename) {}

    /// Set the transformation field of each node to the correct state for the given time
    void calculate_animation(uint animation_id, double time_seconds);
    /// Recursively iterator over node tree
    void visit_nodes(std::function<void(const MeshHierarchyNode& node, glm::mat4 accumulated_transformation)> fn);
};

template<typename VertexData>
void MeshHierarchy<VertexData>::calculate_animation(uint animation_id, double time_seconds) {
    if (animation_id == NONE_ANIMATION) {
        for (auto& mesh: meshes) {
            std::fill(mesh.bone_transforms.begin(), mesh.bone_transforms.end(), glm::mat4{1.0f});
        }
        return;
    }

    if (animation_id >= animations.size()) {
        throw std::runtime_error(Formatter() << "Invalid animation id: " << animation_id);
    }

    std::function<void(const MeshHierarchyNode& node, glm::mat4 accumulated_transformation, bool is_skeleton)> animate;
    double time_ticks = time_seconds * std::get<1>(animations[animation_id]);

    animate = [&animate, this, animation_id, time_ticks](const MeshHierarchyNode& node, glm::mat4 accumulated_transformation, bool is_skeleton) {
        is_skeleton |= !node.bones.empty();
        glm::mat4 transform = is_skeleton ? node.transformation : glm::mat4{1.0f};
        const auto animation = node.animation_data.find(animation_id);
        if (animation != node.animation_data.end()) {
            transform = animation->second.sample(time_ticks);
        }
        accumulated_transformation = accumulated_transformation * transform;

        for (const auto& [mesh_id, bone_id, offset_matrix]: node.bones) {
            meshes[mesh_id].bone_transforms[bone_id] = accumulated_transformation * offset_matrix;
        }

        for (const auto& child: node.children) {
            animate(child, accumulated_transformation, is_skeleton);
        }
    };

    animate(root_node, glm::mat4{1.0f}, false);
}

template<typename VertexData>
void MeshHierarchy<VertexData>::visit_nodes(std::function<void(const MeshHierarchyNode&, glm::mat4)> fn) {
    std::function<void(const MeshHierarchyNode& node, glm::mat4 accumulated_transformation)> visit;

    visit = [&visit, &fn](const MeshHierarchyNode& node, glm::mat4 accumulated_transformation) {
        accumulated_transformation = accumulated_transformation * node.transformation;
        fn(node, accumulated_transformation);
        for (const auto& child: node.children) {
            visit(child, accumulated_transformation);
        }
    };

    visit(root_node, glm::mat4{1.0f});
}

#endif //MESH_HIERARCHY_H
