#ifndef SUN_LIGHT_ELEMENT_H
#define SUN_LIGHT_ELEMENT_H

#include "SceneElement.h"
#include "scene/SceneContext.h"

namespace EditorScene {
    class SunLightElement : public SceneElement {
    public:
        /// NOTE: Must be unique per element type, as it is used to select generators,
        ///       so if you are creating a new element type make sure to change this to a new unique name.
        static constexpr const char* ELEMENT_TYPE_NAME = "Directional Light";

        // Local transformation
        glm::vec3 position;
        glm::vec3 direction;
        bool visible = true;
        float visual_scale = 1.0f;
        // Light stores direction; entities store world transform
        std::shared_ptr<TheSun> light;
        std::shared_ptr<EmissiveEntityRenderer::Entity> light_body;  // Cylinder body
        std::shared_ptr<EmissiveEntityRenderer::Entity> light_face;  // Bright emitting face

        SunLightElement(const ElementRef& parent, std::string name, glm::vec3 position, glm::vec3 direction,
                        std::shared_ptr<TheSun> light,
                        std::shared_ptr<EmissiveEntityRenderer::Entity> light_body,
                        std::shared_ptr<EmissiveEntityRenderer::Entity> light_face) :
            SceneElement(parent, std::move(name)), position(position), direction(direction),
            light(std::move(light)), light_body(std::move(light_body)), light_face(std::move(light_face)) {}

        static std::unique_ptr<SunLightElement> new_default(const SceneContext& scene_context, ElementRef parent);
        static std::unique_ptr<SunLightElement> from_json(const SceneContext& scene_context, ElementRef parent, const json& j);

        [[nodiscard]] json into_json() const override;

        void add_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context) override;

        void update_instance_data() override;

        void add_to_render_scene(MasterRenderScene& target_render_scene) override {
            target_render_scene.insert_entity(light_body);
            target_render_scene.insert_entity(light_face);
            target_render_scene.insert_dir_light(light);
        }

        void remove_from_render_scene(MasterRenderScene& target_render_scene) override {
            target_render_scene.remove_entity(light_body);
            target_render_scene.remove_entity(light_face);
            target_render_scene.remove_dir_light(light);
        }

        [[nodiscard]] const char* element_type_name() const override;
    };
}

#endif //SUN_LIGHT_ELEMENT_H
