#ifndef CAMERA_INTERFACE_H
#define CAMERA_INTERFACE_H

#ifdef _MSC_VER
// This is needed to be able to use M_PI on MSVC compiler
// Despite not using M_PI here, most camera implementations will.
#define _USE_MATH_DEFINES
#include <math.h>
#endif

#include <glm/glm.hpp>

#include "system_interfaces/Window.h"
#include "scene/SceneContext.h"

/// A struct that defines some common properties of a camera.
/// It's intended usage is for switching between different cameras without actually moving,
/// and avoiding the combinatorial explosion of trying to handle each case uniquely.
struct CameraProperties {
    glm::vec3 position;
    float yaw;
    float pitch;
    float fov;
    float gamma;
};

/// A common interface that a Camera must implement.
class CameraInterface {
public:
    /// Called one per frame, to process key/mouse inputs and move the camera around.
    virtual void update(const Window& window, float dt, bool controls_enabled) = 0;

    /// Should move the camera back to some kind of default state
    virtual void reset() = 0;
    /// Add any camera specific controls to ImGUI
    virtual void add_imgui_options_section(const SceneContext& scene_context) = 0;

    /// Saving and loading of the common camera properties, for transferring a state between two cameras.
    [[nodiscard]] virtual CameraProperties save_properties() const = 0;
    virtual void load_properties(const CameraProperties& camera_properties) = 0;

    /// Get the view matrix of the camera in its current state,
    /// that is the matrix which maps from world space to view space.
    [[nodiscard]] virtual glm::mat4 get_view_matrix() const = 0;

    /// The inverse matrix of get_view_matrix(), can be overridden if you want to cache the result or something
    [[nodiscard]] virtual glm::mat4 get_inverse_view_matrix() const {
        return glm::inverse(get_view_matrix());
    }

    /// Get the position in world space of the camera, by default extracts it from the view matrix.
    [[nodiscard]] virtual glm::vec3 get_position() const {
        // Extract the translation from the inverse matrix
        return {get_inverse_view_matrix()[3]};
    }

    /// Get the projection matrix of the camera in its current state,
    /// ths is the matrix which maps from view space to clip space (after perspective division)
    [[nodiscard]] virtual glm::mat4 get_projection_matrix() const = 0;

    /// The inverse matrix of get_projection_matrix(), can be overridden if you want to cache the result or something
    [[nodiscard]] virtual glm::mat4 get_inverse_projection_matrix() const {
        return glm::inverse(get_projection_matrix());
    }

    /// Get the gamma value the camera should use
    [[nodiscard]] virtual float get_gamma() const = 0;

    virtual ~CameraInterface() = default;
};


#endif //CAMERA_INTERFACE_H
