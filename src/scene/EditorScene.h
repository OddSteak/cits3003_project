#ifndef EDITOR_SCENE_H
#define EDITOR_SCENE_H

#include "SceneInterface.h"

#include <list>
#include <memory>
#include <utility>

#include "editor_scene/SceneElement.h"
#include "scene/SceneContext.h"

/// A namespace for all the things related to the EditorScene, since it's rather complicated
namespace EditorScene {
    /// A complex scene, which is an interactive scene editor that allows the use to add, edit and remove entities and lights.
    /// Also allows saving a scene to file and loading it again.
    class EditorScene : public SceneInterface {
        /// The root of the scene tree the scene editor manages, and a reference to which element is selected.
        ElementList scene_root;
        ElementRef selected_element;

        /// The initial camera settings, which is where the camera will be reset to when pressing (R)
        const float init_distance = 8.0f;
        const glm::vec3 init_focus_point = {0.0f, 0.0f, 0.0f};
        const glm::vec3 init_position = {-4.0f, 4.0f * sqrtf(2.0f), 4.0f};
        const float init_pitch = glm::radians(-45.0f);
        const float init_yaw = glm::radians(-45.0f);
        const float init_near = 0.01f;
        const float init_fov = glm::radians(90.0f);

        /// The two supported camera modes
        enum class CameraMode {
            Panning,
            Flying
        } camera_mode = CameraMode::Panning;

        /// The handle of the camera
        std::unique_ptr<CameraInterface> camera = nullptr;

        /// A list of generator functions use to add each type of entity and light to the scene
        std::vector<std::pair<std::string, std::function<std::unique_ptr<SceneElement>(const SceneContext& scene_context, ElementRef parent)>>> entity_generators;
        std::vector<std::pair<std::string, std::function<std::unique_ptr<SceneElement>(const SceneContext& scene_context, ElementRef parent)>>> light_generators;

        /// A list fo generators that construct scene elements from json data
        std::unordered_map<std::string, std::function<std::unique_ptr<SceneElement>(const SceneContext& scene_context, ElementRef parent, const json& j)>> json_generators;
        /// The current save path
        std::optional<std::string> save_path{};

        // The RenderScene of the Scene
        MasterRenderScene render_scene{};
    public:
        EditorScene();

        /// Override the methods from the SceneInterface super class
        void open(const SceneContext& scene_context) override;

        std::pair<TickResponseType, std::shared_ptr<SceneInterface>> tick(float delta_time, const SceneContext& scene_context) override;

        void add_imgui_options_section() override;
        MasterRenderScene& get_render_scene() override;
        CameraInterface& get_camera() override;
        void close(const SceneContext& scene_context) override;

    private:
        /// Helpers to add the two ImGUI windows use to control the scene editor
        void add_imgui_selection_editor(const SceneContext& scene_context);
        void add_imgui_scene_hierarchy(const SceneContext& scene_context);

        /// A helper for switching camera mode
        void set_camera_mode(CameraMode new_camera_mode);

        /// Helpers to recursively iterator down the scene tree
        void visit_children(ElementRef root, const std::function<void(SceneElement&)>& visit);
        void visit_children_and_root(ElementRef root, const std::function<void(SceneElement&)>& visit);

        /// Helpers to save an element to json, and add an element from json
        [[nodiscard]] static json element_to_labelled_json(const SceneElement& element);
        void add_labelled_json_element(const SceneContext& scene_context, ElementRef parent, const ElementList& list, const json& j);

        /// Main save/load calls, which use the current save_path or pop-up a native file dialog
        void save_to_json_file();
        void load_from_json_file(const SceneContext& scene_context);
    };
}

#endif //EDITOR_SCENE_H
