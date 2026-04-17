#ifndef SCENE_INTERFACE_H
#define SCENE_INTERFACE_H

#include <tuple>
#include <memory>
#include <optional>

#include "rendering/scene/MasterRenderScene.h"
#include "rendering/cameras/CameraInterface.h"
#include "rendering/resources/ModelLoader.h"
#include "rendering/resources/TextureLoader.h"
#include "system_interfaces/WindowManager.h"
#include "scene/SceneContext.h"

/// A enum representing the actions that a scene tick might request
enum class TickResponseType {
    /// A Continue, means that the scene should continue to be ticked
    Continue,
    /// A Exit, means that the Scene Manager should ticking the program should close
    Exit,
    /// A ChangeScene must be accompanied with a std::shared_ptr<SceneInterface>,
    /// and the SceneManager will load and switch to that scene
    ChangeScene
};

/// A class with pure virtual functions (aka abstract/interface) that a Scene needs to implement
class SceneInterface {
public:
    /// An open function which the SceneManager will call before calling any ticks on it.
    virtual void open(const SceneContext& scene_context) = 0;

    /// A tick that allows the scene to do work each frame.
    /// It must return a TickResponseType, and if that is `ChangeScene` then the std::shared_ptr<SceneInterface> must be not-null.
    /// For other TickResponseType the value of the shared_ptr is ignored.
    /// delta_time is the time since the last call to tick.
    virtual std::pair<TickResponseType, std::shared_ptr<SceneInterface>> tick(float delta_time, const SceneContext& scene_context) = 0;

    /// A place for the scene to add any imgui controls to the intended section
    virtual void add_imgui_options_section() {};
    /// Will be called each frame, and the provided MasterRenderScene will be drawn.
    virtual MasterRenderScene& get_render_scene() = 0;
    /// Will be called each frame, and the provided Camera will be used to draw.
    virtual CameraInterface& get_camera() = 0;

    /// Will be called when the SceneManager is switching to a new scene.
    /// tick will not be called again after this, without open being called again first
    virtual void close(const SceneContext& scene_context) = 0;

    /// virtual destructor so that destructors of derived classes are called properly
    virtual ~SceneInterface() = default;
};

#endif //SCENE_INTERFACE_H
