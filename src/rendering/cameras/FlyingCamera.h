#ifndef FLYING_CAMERA_H
#define FLYING_CAMERA_H

#include "CameraInterface.h"

/// A concrete implementation of CameraInterface, that is a camera with basic flight like controls.
class FlyingCamera : public CameraInterface {
    glm::vec3 init_position = {0.0f, 0.0f, 0.0f};
    float init_pitch = 0.0f;
    float init_yaw = 0.0f;

    float init_near = 0.01f;
    float init_fov = glm::radians(90.0f);

    float init_gamma = 2.2f;

    glm::vec3 position{};
    glm::vec3 velocity{};
    float pitch;
    float yaw;

    float near;
    float fov;

    const glm::vec3 UP = {0.0f, 1.0f, 0.0f};
    const glm::vec3 RIGHT = {1.0f, 0.0f, 0.0f};
    const glm::vec3 FORWARD = {0.0f, 0.0f, -1.0f};

    const float YAW_SPEED = 0.01f;
    const float PITCH_SPEED = 0.01f;
    const float MAX_MOVE_SPEED = 2.0f;
    const float ACCELERATION = 5.0f;
    const float EXPONENTIAL_DECAY = 1.0f;
    const float LINEAR_DECAY = 0.01f;

    const float YAW_PERIOD = 2.0f * (float) M_PI;
    const float PITCH_MIN = (float) -M_PI_2 + 0.01f;
    const float PITCH_MAX = (float) M_PI_2 - 0.01f;

    glm::mat4 view_matrix{1.0f};
    glm::mat4 inverse_view_matrix{1.0f};
    glm::mat4 projection_matrix{1.0f};
    glm::mat4 inverse_projection_matrix{1.0f};

    float gamma = 2.2f;
public:
    /// Construct the Camera with default initial state
    FlyingCamera();
    /// Construct the Camera with a specific initial state, which will also be the reset state.
    FlyingCamera(glm::vec3 position, float pitch, float yaw, float near, float fov);

    void update(const Window& window, float dt, bool controlsEnabled) override;

    void reset() override;
    void add_imgui_options_section(const SceneContext& scene_context) override;

    [[nodiscard]] CameraProperties save_properties() const override;
    void load_properties(const CameraProperties& camera_properties) override;

    [[nodiscard]] glm::mat4 get_view_matrix() const override;
    [[nodiscard]] glm::mat4 get_inverse_view_matrix() const override;
    [[nodiscard]] glm::vec3 get_position() const override;
    [[nodiscard]] glm::mat4 get_projection_matrix() const override;
    [[nodiscard]] glm::mat4 get_inverse_projection_matrix() const override;
    [[nodiscard]] float get_gamma() const override;
};


#endif //FLYING_CAMERA_H
