#ifndef BASE_ENTITY_SHADER_H
#define BASE_ENTITY_SHADER_H

#include <utility>
#include <vector>
#include <unordered_set>

#include "glm/glm.hpp"

#include "ShaderInterface.h"
#include "rendering/scene/Lights.h"
#include "rendering/scene/GlobalData.h"
#include "rendering/scene/RenderScene.h"
#include "rendering/scene/RenderedEntity.h"
#include "rendering/resources/ModelLoader.h"
#include "rendering/resources/TextureHandle.h"
#include "rendering/memory/UniformBufferArray.h"

struct BaseEntityInstanceData {
    explicit BaseEntityInstanceData(const glm::mat4& model_matrix) : model_matrix(model_matrix) {}

    glm::mat4 model_matrix;
};

struct BaseEntityGlobalData : public GlobalDataCameraInterface {
    BaseEntityGlobalData() = default;

    BaseEntityGlobalData(const glm::mat4& projection_view_matrix, const glm::vec3& camera_position, float gamma) : projection_view_matrix(projection_view_matrix), camera_position(camera_position), gamma(gamma) {}

    glm::mat4 projection_view_matrix{};
    glm::vec3 camera_position{};
    float gamma = 1.0f;

    void use_camera(const CameraInterface& camera_interface) override {
        projection_view_matrix = camera_interface.get_projection_matrix() * camera_interface.get_view_matrix();
        camera_position = camera_interface.get_position();
        gamma = camera_interface.get_gamma();
    }
};

class BaseEntityShader : public ShaderInterface {
protected:
    int model_matrix_location{};
    int projection_view_matrix_location{};
    // Global Data
    int ws_view_position_location{};
    int inverse_gamma_location{};
public:
    BaseEntityShader(std::string name, const std::string& vertex_path, const std::string& fragment_path,
                     std::unordered_map<std::string, std::string> vert_defines = {},
                     std::unordered_map<std::string, std::string> frag_defines = {});

    void set_instance_data(const BaseEntityInstanceData& instance_data);

    void set_global_data(const BaseEntityGlobalData& global_data);
protected:
    virtual void get_uniforms_set_bindings();
};

#endif //BASE_ENTITY_SHADER_H
