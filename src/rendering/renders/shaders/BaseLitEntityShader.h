#ifndef BASE_LIT_ENTITY_SHADER_H
#define BASE_LIT_ENTITY_SHADER_H

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

#include "BaseEntityShader.h"

struct BaseLitEntityMaterial {
    // Alpha components are just used to store a scalar that is applied before passing to the GPU
    glm::vec4 diffuse_tint;
    glm::vec4 specular_tint;
    glm::vec4 ambient_tint;
    float shininess;
};

struct BaseLitEntityInstanceData : public BaseEntityInstanceData {
    BaseLitEntityInstanceData(const glm::mat4& model_matrix, const BaseLitEntityMaterial& material) : BaseEntityInstanceData(model_matrix), material(material) {}

    // Material properties
    BaseLitEntityMaterial material;
};

struct BaseLitEntityRenderData {
    BaseLitEntityRenderData(std::shared_ptr<TextureHandle> diffuse_texture, std::shared_ptr<TextureHandle> specular_map_texture)
        : diffuse_texture(std::move(diffuse_texture)), specular_map_texture(std::move(specular_map_texture)) {}

    std::shared_ptr<TextureHandle> diffuse_texture;
    std::shared_ptr<TextureHandle> specular_map_texture;
};

using BaseLitEntityGlobalData = BaseEntityGlobalData;

class BaseLitEntityShader : public BaseEntityShader {
public:
    static constexpr uint MAX_PL = 16;

protected:
    // Material
    int diffuse_tint_location{};
    int specular_tint_location{};
    int ambient_tint_location{};
    int shininess_location{};

    static const uint POINT_LIGHT_BINDING = 0;

    UniformBufferArray<PointLight::Data, MAX_PL> point_lights_ubo;
public:
    BaseLitEntityShader(std::string name, const std::string& vertex_path, const std::string& fragment_path,
                        std::unordered_map<std::string, std::string> vert_defines = {},
                        std::unordered_map<std::string, std::string> frag_defines = {});

    void set_instance_data(const BaseLitEntityInstanceData& instance_data);

    void set_point_lights(const std::vector<PointLight>& point_lights);
protected:
    void get_uniforms_set_bindings() override;
};

#endif //BASE_LIT_ENTITY_SHADER_H
