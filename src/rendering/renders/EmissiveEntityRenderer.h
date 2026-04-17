#ifndef EMISSIVE_ENTITY_RENDERER_H
#define EMISSIVE_ENTITY_RENDERER_H

#include <utility>
#include <vector>
#include <unordered_set>

#include <glm/glm.hpp>

#include <assimp/scene.h>

#include "rendering/renders/shaders/ShaderInterface.h"
#include "rendering/scene/GlobalData.h"
#include "rendering/scene/RenderScene.h"
#include "rendering/scene/RenderedEntity.h"
#include "rendering/resources/TextureHandle.h"

#include "EntityRenderer.h"

#include "rendering/renders/shaders/BaseEntityShader.h"

namespace EmissiveEntityRenderer {
    using VertexData = EntityRenderer::VertexData;

    struct EmissiveEntityMaterial {
        // Alpha components are just used to store a scalar that is applied before passing to the GPU
        glm::vec4 emission_tint;
    };

    struct InstanceData : public BaseEntityInstanceData {
        InstanceData(const glm::mat4& model_matrix, const EmissiveEntityMaterial& material) : BaseEntityInstanceData(model_matrix), material(material) {}

        // Material properties
        EmissiveEntityMaterial material;
    };

    using GlobalData = BaseEntityGlobalData;

    struct RenderData {
        explicit RenderData(std::shared_ptr<TextureHandle> emission_texture)
            : emission_texture(std::move(emission_texture)) {}

        std::shared_ptr<TextureHandle> emission_texture;
    };

    using Entity = RenderedEntity<VertexData, InstanceData, RenderData>;

    using RenderScene = RenderScene<Entity, GlobalData>;

    class EmissiveEntityShader : public BaseEntityShader {
        // Material
        int emission_tint_location{};
    public:
        EmissiveEntityShader();

        void set_instance_data(const InstanceData& instance_data);
    private:
        void get_uniforms_set_bindings() override;
    };

    class EmissiveEntityRenderer {
        EmissiveEntityShader shader;

    public:
        EmissiveEntityRenderer();

        void render(const RenderScene& render_scene);

        bool refresh_shaders();
    };
}

#endif //EMISSIVE_ENTITY_RENDERER_H
