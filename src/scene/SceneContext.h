#ifndef SCENE_CONTEXT_H
#define SCENE_CONTEXT_H

#include "system_interfaces/WindowManager.h"
#include "rendering/resources/TextureLoader.h"
#include "rendering/resources/ModelLoader.h"

/// Definition of the struct that is a collection of things a scene needs
struct SceneContext {
    Window& window;
    WindowManager& window_manager;
    ModelLoader& model_loader;
    TextureLoader& texture_loader;
    bool imgui_enabled;
};

#endif //SCENE_CONTEXT_H
