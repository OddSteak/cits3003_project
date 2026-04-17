#ifndef BASIC_SCENE_H
#define BASIC_SCENE_H

#include "SceneInterface.h"
#include "scene/SceneContext.h"

/// A Scene which just a basic collection of a few static entities
/// and a rotating box, with no user interaction.
class BasicStaticScene : public SceneInterface {
    /// The handle of the box entity, which we hold onto so that we can rotate it
    std::shared_ptr<EntityRenderer::Entity> box_entity = nullptr;

    /// The initial camera settings, which is where the camera will be reset to when pressing (R)
    const float init_distance = 5.0f;
    const glm::vec3 init_focus_point = {2.5f, 0.0f, 1.5f};
    const glm::vec3 init_position = {0.0f, 2.5f * sqrtf(2.0f), 4.0f};
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

    // The RenderScene of the Scene
    MasterRenderScene render_scene{};
public:
    BasicStaticScene();

    /// Override the methods from the SceneInterface super class
    void open(const SceneContext& scene_context) override;

    std::pair<TickResponseType, std::shared_ptr<SceneInterface>> tick(float delta_time, const SceneContext& scene_context) override;

    void add_imgui_options_section() override;
    MasterRenderScene& get_render_scene() override;
    CameraInterface& get_camera() override;
    void close(const SceneContext& scene_context) override;

private:
    /// A helper for switching camera mode
    void set_camera_mode(CameraMode new_camera_mode);
};

#endif //BASIC_SCENE_H
