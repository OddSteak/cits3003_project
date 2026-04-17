#include "PanningCamera.h"

#include <cmath>

#include <glm/gtx/transform.hpp>

#include "utility/Math.h"
#include "rendering/imgui/ImGuiManager.h"

PanningCamera::PanningCamera() : distance(init_distance), focus_point(init_focus_point), pitch(init_pitch), yaw(init_yaw), near(init_near), fov(init_fov) {}

PanningCamera::PanningCamera(float distance, glm::vec3 focus_point, float pitch, float yaw, float near, float fov)
    : init_distance(distance), init_focus_point(focus_point), init_pitch(pitch), init_yaw(yaw), init_near(near), init_fov(fov), distance(distance), focus_point(focus_point), pitch(pitch), yaw(yaw), near(near), fov(fov) {}

void PanningCamera::update(const Window& window, float dt, bool controls_enabled) {
    if (controls_enabled) {
        bool ctrl_is_pressed = window.is_key_pressed(GLFW_KEY_LEFT_CONTROL) || window.is_key_pressed(GLFW_KEY_RIGHT_CONTROL);

        bool resetSeq = false;
        if (window.was_key_pressed(GLFW_KEY_R) && !ctrl_is_pressed) {
            reset();
            resetSeq = true;
        }

        if (!resetSeq) {
            // Extract basis vectors from inverse view matrix to find world space directions of view space basis
            auto x_basis = glm::vec3{inverse_view_matrix[0]};
            auto y_basis = glm::vec3{inverse_view_matrix[1]};

            auto pan = window.get_mouse_delta(GLFW_MOUSE_BUTTON_MIDDLE);
            focus_point += (x_basis * (float) -pan.x + y_basis * (float) pan.y) * PAN_SPEED * dt * distance / (float) window.get_window_height();

            pitch -= PITCH_SPEED * (float) window.get_mouse_delta(GLFW_MOUSE_BUTTON_RIGHT).y;
            yaw -= YAW_SPEED * (float) window.get_mouse_delta(GLFW_MOUSE_BUTTON_RIGHT).x;
            distance -= ZOOM_SCROLL_MULTIPLIER * ZOOM_SPEED * window.get_scroll_delta();

            auto is_dragging = window.is_mouse_pressed(GLFW_MOUSE_BUTTON_RIGHT) || window.is_mouse_pressed(GLFW_MOUSE_BUTTON_MIDDLE);
            if (is_dragging) {
                window.set_cursor_disabled(true);
            }
        }
    }

    yaw = std::fmod(yaw + YAW_PERIOD, YAW_PERIOD);
    pitch = clamp(pitch, PITCH_MIN, PITCH_MAX);
    distance = clamp(distance, MIN_DISTANCE, MAX_DISTANCE);

    view_matrix = glm::translate(glm::vec3{0.0f, 0.0f, -distance});
    inverse_view_matrix = glm::inverse(view_matrix);

    projection_matrix = glm::infinitePerspective(fov, window.get_framebuffer_aspect_ratio(), 1.0f);
    inverse_projection_matrix = glm::inverse(projection_matrix);
}

void PanningCamera::reset() {
    distance = init_distance;
    focus_point = init_focus_point;
    pitch = init_pitch;
    yaw = init_yaw;
    fov = init_fov;
    near = init_near;
    gamma = init_gamma;
}

void PanningCamera::add_imgui_options_section(const SceneContext& scene_context) {
    if (!ImGui::CollapsingHeader("Camera Options")) {
        return;
    }

    ImGui::DragFloat3("Focus Point (x,y,z)", &focus_point[0], 0.01f);
    ImGui::DragDisableCursor(scene_context.window);

    ImGui::DragFloat("Distance", &distance, 0.01f, MIN_DISTANCE, MAX_DISTANCE);
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

CameraProperties PanningCamera::save_properties() const {
    return CameraProperties{
        get_position(),
        yaw,
        pitch,
        fov,
        gamma
    };
}

void PanningCamera::load_properties(const CameraProperties& camera_properties) {
    yaw = camera_properties.yaw;
    pitch = camera_properties.pitch;
    focus_point = camera_properties.position;
    fov = camera_properties.fov;
    gamma = camera_properties.gamma;
    distance = 1.0f;

    auto inverse_rot_matrix = glm::rotate(yaw, glm::vec3{0.0f, 1.0f, 0.0f}) * glm::rotate(pitch, glm::vec3{1.0f, 0.0f, 0.0f});
    auto forward = glm::vec3{inverse_rot_matrix[2]};

    focus_point -= distance * forward;
}


glm::mat4 PanningCamera::get_view_matrix() const {
    return view_matrix;
}

glm::mat4 PanningCamera::get_inverse_view_matrix() const {
    return inverse_view_matrix;
}

glm::mat4 PanningCamera::get_projection_matrix() const {
    return projection_matrix;
}

glm::mat4 PanningCamera::get_inverse_projection_matrix() const {
    return inverse_projection_matrix;
}

float PanningCamera::get_gamma() const {
    return gamma;
}
