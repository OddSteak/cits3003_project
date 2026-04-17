#ifndef GROUP_ELEMENT_H
#define GROUP_ELEMENT_H

#include "SceneElement.h"
#include "scene/SceneContext.h"

namespace EditorScene {
    class GroupElement : virtual public SceneElement, public LocalTransformComponent {
    public:
        /// NOTE: Must be unique per element type, as it is used to select generators,
        ///       so if you are creating a new element type make sure to change this to a new unique name.
        static constexpr const char* ELEMENT_TYPE_NAME = "Group";

        std::shared_ptr<std::list<std::unique_ptr<SceneElement>>> children;

        explicit GroupElement(const ElementRef& parent, std::string name, std::list<std::unique_ptr<SceneElement>> children)
            : SceneElement(parent, std::move(name)), LocalTransformComponent(glm::vec3{0.0f}, glm::vec3{0.0f}, glm::vec3{1.0f}), children(std::make_shared<std::list<std::unique_ptr<SceneElement>>>(std::move(children))) {}

        explicit GroupElement(const ElementRef& parent, std::string name)
            : SceneElement(parent, std::move(name)), LocalTransformComponent(glm::vec3{0.0f}, glm::vec3{0.0f}, glm::vec3{1.0f}), children(std::make_shared<std::list<std::unique_ptr<SceneElement>>>(std::list<std::unique_ptr<SceneElement>>{})) {}

        static std::unique_ptr<GroupElement> from_json(ElementRef parent, const json& j);
        [[nodiscard]] json into_json() const override;

        void add_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context) override;

        void update_instance_data() override;

        void add_to_render_scene(MasterRenderScene& /*target_render_scene*/) override {}

        void remove_from_render_scene(MasterRenderScene& /*target_render_scene*/) override {}

        [[nodiscard]] ElementList get_children() const override {
            return children;
        }

        void add_child(std::unique_ptr<SceneElement> scene_element) override;

        [[nodiscard]] const char* element_type_name() const override;
    };
}

#endif //GROUP_ELEMENT_H
