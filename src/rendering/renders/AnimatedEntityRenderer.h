#ifndef ANIMATED_ENTITY_RENDERER_H
#define ANIMATED_ENTITY_RENDERER_H

#include <utility>
#include <vector>
#include <unordered_set>

#include <glm/glm.hpp>

#include "rendering/renders/shaders/ShaderInterface.h"
#include "rendering/scene/Lights.h"
#include "rendering/scene/GlobalData.h"
#include "rendering/scene/RenderScene.h"
#include "rendering/scene/RenderedEntity.h"
#include "rendering/resources/ModelLoader.h"
#include "rendering/resources/TextureHandle.h"
#include "rendering/memory/UniformBufferArray.h"

#include "rendering/renders/shaders/BaseLitEntityShader.h"

#define BONE_TRANSFORMS 64
#define BONE_TRANSFORMS_STR "64"

namespace AnimatedEntityRenderer {
    struct VertexData {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texture_coordinate;
        glm::vec4 bone_weights;
        glm::uvec4 bone_indices;

        static void from_mesh(const VertexCollection& vertex_collection, std::vector<VertexData>& out_vertices);
        static void setup_attrib_pointers();
    };

    using EntityMaterial = BaseLitEntityMaterial;
    using InstanceData = BaseLitEntityInstanceData;
    using GlobalData = BaseLitEntityGlobalData;
    using RenderData = BaseLitEntityRenderData;

    using Entity = AnimatedRenderedEntity<VertexData, InstanceData, RenderData>;

    using RenderScene = RenderScene<Entity, GlobalData>;

    class AnimatedEntityShader : public BaseLitEntityShader {
        // Animation Data
        int bone_transforms_location{};
    public:
        AnimatedEntityShader();

        void set_model_matrix(const glm::mat4& model_matrix);

        void set_bone_transforms(const std::vector<glm::mat4>& bone_transforms);
    private:
        // Override get_uniforms_set_bindings to get the extra uniform for bone transforms
        void get_uniforms_set_bindings() override;
    };

    class AnimatedEntityRenderer {
        AnimatedEntityShader shader;

    public:
        AnimatedEntityRenderer();

        void render(const RenderScene& render_scene, const LightScene& light_scene);

        bool refresh_shaders();
    };
}

#endif //ANIMATED_ENTITY_RENDERER_H
