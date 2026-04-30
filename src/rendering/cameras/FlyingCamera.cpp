#include "FlyingCamera.h"

#include <cmath>

#include <glm/gtx/transform.hpp>

#include "utility/Math.h"
#include "rendering/imgui/ImGuiManager.h"


FlyingCamera::FlyingCamera() : pitch(init_pitch), yaw(init_yaw), near(init_near), fov(init_fov) {}

FlyingCamera::FlyingCamera(glm::vec3 position, float pitch, float yaw, float near, float fov)
    : init_position(position), init_pitch(pitch), init_yaw(yaw), init_near(near), init_fov(fov), position(position), pitch(pitch), yaw(yaw), near(near), fov(fov) {}

void FlyingCamera::update(const Window& window, float dt, bool controlsEnabled) {
    if (controlsEnabled) {
        bool ctrl_is_pressed = window.is_key_pressed(GLFW_KEY_LEFT_CONTROL) || window.is_key_pressed(GLFW_KEY_RIGHT_CONTROL);

        bool resetSeq = false;
        if (window.was_key_pressed(GLFW_KEY_R) && !ctrl_is_pressed) {
            reset();
            resetSeq = true;
        }

        if (!resetSeq) {
            // Extract basis vectors from inverse view matrix to find world space directions of view space basis
            auto x_basis = glm::vec3{inverse_view_matrix[0]};
            // auto y_basis = glm::vec3{inverse_view_matrix[1]};
            auto z_basis = glm::vec3{inverse_view_matrix[2]};

            glm::vec3 forward_flat = glm::normalize(glm::vec3{-z_basis.x, 0.0f, -z_basis.z}); // -ve needed since -z is forward
            glm::vec3 right_flat = glm::normalize(glm::vec3{x_basis.x, 0.0f, x_basis.z});

            bool any_forward = false;
            if (window.is_key_pressed(GLFW_KEY_W) && !ctrl_is_pressed) {
                velocity += forward_flat * ACCELERATION * dt;
                any_forward = true;
            }
            if (window.is_key_pressed(GLFW_KEY_S) && !ctrl_is_pressed) {
                velocity -= forward_flat * ACCELERATION * dt;
                any_forward = true;
            }
            if (!any_forward) {
                float forward_speed = glm::dot(forward_flat, velocity);
                velocity -= forward_flat * std::min(EXPONENTIAL_DECAY * dt * forward_speed + LINEAR_DECAY, forward_speed);
            }

            bool any_right = false;
            if (window.is_key_pressed(GLFW_KEY_D) && !ctrl_is_pressed) {
                velocity += right_flat * ACCELERATION * dt;
                any_right = true;
            }
            if (window.is_key_pressed(GLFW_KEY_A) && !ctrl_is_pressed) {
                velocity -= right_flat * ACCELERATION * dt;
                any_right = true;
            }
            if (!any_right) {
                float right_speed = glm::dot(right_flat, velocity);
                velocity -= right_flat * std::min(EXPONENTIAL_DECAY * dt * right_speed + LINEAR_DECAY, right_speed);
            }

            bool any_up = false;
            if (window.is_key_pressed(GLFW_KEY_SPACE) && !ctrl_is_pressed) {
                velocity += UP * ACCELERATION * dt;
                any_up = true;
            }
            if (window.is_key_pressed(GLFW_KEY_LEFT_SHIFT) && !ctrl_is_pressed) {
                velocity -= UP * ACCELERATION * dt;
                any_up = true;
            }
            if (!any_up) {
                float up_speed = glm::dot(UP, velocity);
                velocity -= UP * std::min(EXPONENTIAL_DECAY * dt * up_speed + LINEAR_DECAY, up_speed);
            }

            float speed = glm::length(velocity);
            glm::vec3 dir = speed < std::numeric_limits<float>::epsilon() ? glm::vec3{0.0f} : velocity / speed;

            if (speed > MAX_MOVE_SPEED) {
                velocity -= dir * std::min(EXPONENTIAL_DECAY * dt * speed + LINEAR_DECAY, speed);
            }

            position += velocity * dt;

            pitch -= PITCH_SPEED * (float) window.get_mouse_delta(GLFW_MOUSE_BUTTON_RIGHT).y;
            yaw -= YAW_SPEED * (float) window.get_mouse_delta(GLFW_MOUSE_BUTTON_RIGHT).x;

            if (window.is_mouse_pressed(GLFW_MOUSE_BUTTON_RIGHT)) {
                window.set_cursor_disabled(true);
            }
        }
    }

    yaw = std::fmod(yaw + YAW_PERIOD, YAW_PERIOD);
    pitch = clamp(pitch, PITCH_MIN, PITCH_MAX);

    view_matrix =
        glm::rotate(-pitch, glm::vec3{1.0f, 0.0f, 0.0f}) *
        glm::rotate(-yaw, glm::vec3{0.0f, 1.0f, 0.0f}) *
        glm::translate(-position);
    inverse_view_matrix = glm::inverse(view_matrix);

    projection_matrix = glm::infinitePerspective(fov, window.get_framebuffer_aspect_ratio(), near);
    inverse_projection_matrix = glm::inverse(projection_matrix);
}

void FlyingCamera::reset() {
    position = init_position;
    velocity = glm::vec3{0.0f};
    pitch = init_pitch;
    yaw = init_yaw;
    fov = init_fov;
    near = init_near;
    gamma = init_gamma;
}

void FlyingCamera::add_imgui_options_section(const SceneContext& scene_context) {
    if (!ImGui::CollapsingHeader("Camera Options")) {
        return;
    }

    ImGui::DragFloat3("Position (x,y,z)", &position[0], 0.01f);
    ImGui::DragDisableCursor(scene_context.window);

    float pitch_degrees = glm::degrees(pitch);
    ImGui::SliderFloat("Pitch", &pitch_degrees, -89.99f, 89.99f);
    pitch = glm::radians(pitch_degrees);

    float yaw_degrees = glm::degrees(yaw);
    ImGui::DragFloat("Yaw", &yaw_degrees);
    ImGui::DragDisableCursor(scene_context.window);
    yaw = glm::radians(glm::mod(yaw_degrees, 360.0f));

    ImGui::SliderFloat("Near Plane", &near, 0.001f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);

    float fov_degrees = glm::degrees(fov);
    ImGui::SliderFloat("FOV", &fov_degrees, 40.0f, 170.0f);
    fov = glm::radians(fov_degrees);

    ImGui::Spacing();
    ImGui::SliderFloat("Gamma", &gamma, 1.0f, 5.0f, "%.2f");

    if (ImGui::Button("Reset (R)")) {
        reset();
    }
}

CameraProperties FlyingCamera::save_properties() const {
    return CameraProperties{
        position,
        yaw,
        pitch,
        fov,
        gamma
    };
}

void FlyingCamera::load_properties(const CameraProperties& camera_properties) {
    position = camera_properties.position;
    yaw = camera_properties.yaw;
    pitch = camera_properties.pitch;
    fov = camera_properties.fov;
    gamma = camera_properties.gamma;
}

glm::mat4 FlyingCamera::get_view_matrix() const {
    return view_matrix;
}

glm::mat4 FlyingCamera::get_inverse_view_matrix() const {
    return inverse_view_matrix;
}

glm::vec3 FlyingCamera::get_position() const {
    return position;
}

glm::mat4 FlyingCamera::get_projection_matrix() const {
    return projection_matrix;
}

glm::mat4 FlyingCamera::get_inverse_projection_matrix() const {
    return inverse_projection_matrix;
}

float FlyingCamera::get_gamma() const {
    return gamma;
}
