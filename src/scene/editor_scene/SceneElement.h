#ifndef SCENE_ELEMENT_H
#define SCENE_ELEMENT_H

#include <memory>
#include <list>

#include <glm/glm.hpp>

#include "utility/JsonHelper.h"
#include "../SceneInterface.h"
#include "scene/SceneContext.h"

namespace EditorScene {
    class SceneElement;

    using ElementList = std::shared_ptr<std::list<std::unique_ptr<SceneElement>>>;
    using ElementRef = ElementList::element_type::iterator;
    static const ElementRef NullElementRef{};

    /// An interface that represents a element in the scene tree the scene editor uses to control all the entities
    class SceneElement {
    public:
        /// A reference to the parent element, or NullElementRef
        ElementRef parent;
        /// The name of the element, to be displayed in the UI
        std::string name;
        /// The total transformation of the element, including parent transformations
        glm::mat4 transform{1.0f};
        /// Tracks if the element is enabled or not
        bool enabled = true;

        explicit SceneElement(const ElementRef& parent, std::string name) : parent(parent), name(std::move(name)) {}

        /// Create a json element representing the element
        [[nodiscard]] virtual json into_json() const = 0;

        /// Helper method for storing base data
        void store_json(json& j) const {
            j["enabled"] = enabled;
            j["name"] = name;
        }

        /// Helper method for loading base data
        void load_json(const json& j) {
            enabled = j["enabled"];
            name = j["name"];
        }

        /// Adds the editor fields for the current element, to be specialised to the specific entity
        virtual void add_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context);

        /// Update this entities instance data, and also transform which uses the parents transform
        virtual void update_instance_data() = 0;

        /// Simple add and remove self from the render scene
        virtual void add_to_render_scene(MasterRenderScene& target_render_scene) = 0;
        virtual void remove_from_render_scene(MasterRenderScene& target_render_scene) = 0;

        /// Get a list of child elements, currently only used by GroupElement, null means can't have children
        [[nodiscard]] virtual ElementList get_children() const {
            return nullptr;
        };

        /// Add to the list of children, currently only used by GroupElement, throws exception in case it doesn't support children
        virtual void add_child(std::unique_ptr<SceneElement> /* scene_element */) {
            throw std::logic_error("This type does not support adding children");
        };

        /// Get the type name of the SceneElement, used for loading/saving from/to json/
        ///
        /// MUST return a static string, aka just use a string literal
        [[nodiscard]] virtual const char* element_type_name() const = 0;

        /// Helper to recurse down children from self
        void visit_children_recursive(const std::function<void(SceneElement& child)>& fn) const;

        static json texture_to_json(const std::shared_ptr<TextureHandle>& texture);
        static std::shared_ptr<TextureHandle> texture_from_json(const SceneContext& scene_context, const json& json);

        virtual ~SceneElement() = default;
    };

    /// A component for a SceneElement to add default local transform behaviour
    class LocalTransformComponent : virtual public SceneElement {
    public:
        // Local transformation
        glm::vec3 position;
        glm::vec3 euler_rotation;
        glm::vec3 scale;

    protected:
        LocalTransformComponent(const glm::vec3& position, const glm::vec3& euler_rotation, const glm::vec3& scale) : position(position), euler_rotation(euler_rotation), scale(scale) {}

        void add_local_transform_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context);

        void update_local_transform_from_json(const json& json);
        [[nodiscard]] json local_transform_into_json() const;

        [[nodiscard]] glm::mat4 calc_model_matrix() const;
    };

    class LitMaterialComponent : virtual public SceneElement {
    public:
        BaseLitEntityMaterial material;

    protected:
        explicit LitMaterialComponent(const BaseLitEntityMaterial& material) : material(material) {}

        void add_material_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context);

        void update_material_from_json(const json& json);
        [[nodiscard]] json material_into_json() const;
    };

    class EmissiveMaterialComponent : virtual public SceneElement {
    public:
        EmissiveEntityRenderer::EmissiveEntityMaterial material;

    protected:
        explicit EmissiveMaterialComponent(const EmissiveEntityRenderer::EmissiveEntityMaterial& material) : material(material) {}

        void add_emissive_material_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context);

        void update_emissive_material_from_json(const json& json);
        [[nodiscard]] json emissive_material_into_json() const;
    };

    /// A component for a SceneElement to add support for animation
    class AnimationComponent : virtual public SceneElement {
    public:
        explicit AnimationComponent() = default;

        /// Adds the control for animated elements
        void add_animation_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context);

        /// Getters to be overridden
        [[nodiscard]] virtual std::shared_ptr<AnimatedEntityInterface> get_entity() = 0;
        [[nodiscard]] virtual AnimationParameters& get_animation_parameters() = 0;
    };

    /// Helper to checking if an ElementRef is equal to NullElementRef, needed since `ref == NullElementRef` will
    /// crash with the MSVC compiler in debug mode due to its strictness.
    bool is_null(const ElementRef& ref);
    /// Helper to checking if two ElementRef are equal, needed since `e1 == e2` may
    /// crash with the MSVC compiler in debug mode due to its strictness.
    bool eq(const ElementRef& e1, const ElementRef& e2);
}

#endif //SCENE_ELEMENT_H
