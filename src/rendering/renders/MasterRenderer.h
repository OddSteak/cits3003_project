#ifndef MASTER_RENDERER_H
#define MASTER_RENDERER_H

#include "utility/SyncManager.h"
#include "EntityRenderer.h"
#include "EmissiveEntityRenderer.h"
#include "rendering/scene/MasterRenderScene.h"
#include "system_interfaces/WindowManager.h"
#include "scene/SceneInterface.h"
#include "scene/SceneContext.h"

/// The Master Renderer, which contains each of the individual renderers, and calls render on them.
class MasterRenderer {
    EntityRenderer::EntityRenderer entity_renderer;
    AnimatedEntityRenderer::AnimatedEntityRenderer animated_entity_renderer;
    EmissiveEntityRenderer::EmissiveEntityRenderer emissive_entity_renderer;
    SyncManager sync_manager;

    struct RenderSettings {
        bool show_wireframe = false;
        bool cull_back_face = true;
        bool cull_front_face = false;
        bool v_sync = false;
        bool enable_fps_cap = true;
        float fps_cap = 240.0f;
    } render_settings;
public:
    MasterRenderer();

    /// Prepare the master renderer for a new frame
    void update(const Window& window);
    /// Render the provided MasterRenderScene with the provided SceneContext
    void render_scene(MasterRenderScene& render_scene, const SceneContext& scene_context);
    /// Synchronise the framerate if enabled.
    void sync();

    /// Adds a control for editing the RenderSettings
    void add_imgui_options_section(WindowManager& window_manager);
};

#endif //MASTER_RENDERER_H
