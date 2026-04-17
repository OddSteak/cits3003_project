#include "MasterRenderer.h"
#include <glad/gl.h>

#include "rendering/imgui/ImGuiManager.h"
#include "scene/SceneContext.h"

MasterRenderer::MasterRenderer() : entity_renderer(), animated_entity_renderer(), emissive_entity_renderer(), render_settings() {
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

void MasterRenderer::update(const Window& window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glViewport(0, 0, (int) window.get_framebuffer_width(), (int) window.get_framebuffer_height());
}

void MasterRenderer::render_scene(MasterRenderScene& render_scene, const SceneContext& scene_context) {
    render_scene.animator.animate(scene_context.window_manager.get_delta_time());
    entity_renderer.render(render_scene.entity_scene, render_scene.light_scene);
    animated_entity_renderer.render(render_scene.animated_entity_scene, render_scene.light_scene);
    emissive_entity_renderer.render(render_scene.emissive_entity_scene);
}

void MasterRenderer::sync() {
    if (render_settings.enable_fps_cap) {
        sync_manager.sync(render_settings.fps_cap);
    }
}

void MasterRenderer::add_imgui_options_section(WindowManager& window_manager) {
    if (ImGui::CollapsingHeader("Render Settings")) {
        if (ImGui::Checkbox("Show Wireframe", &render_settings.show_wireframe)) {
            glPolygonMode(GL_FRONT_AND_BACK, render_settings.show_wireframe ? GL_LINE : GL_FILL);
        }

        if (ImGui::Checkbox("Cull Back Faces", &render_settings.cull_back_face) ||
            ImGui::Checkbox("Cull Front Faces", &render_settings.cull_front_face)) {
            if (render_settings.cull_front_face && render_settings.cull_back_face) {
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT_AND_BACK);
            } else if (render_settings.cull_front_face) {
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
            } else if (render_settings.cull_back_face) {
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
            } else {
                glDisable(GL_CULL_FACE);
            }
        }

        if (ImGui::Checkbox("V-Sync", &render_settings.v_sync)) {
            window_manager.set_v_sync(render_settings.v_sync);
        }

        ImGui::Checkbox("Enable FPS Cap", &render_settings.enable_fps_cap);

        if (ImGui::SliderFloat("FPS Cap", &render_settings.fps_cap, 24.0f, 240.0f)) {
            if (render_settings.fps_cap < 24.0f) {
                render_settings.fps_cap = 24.0f;
            }
        }
    }

    if (ImGui::CollapsingHeader("Shader Options")) {
        static int failures = 0;
        static double last_time = -std::numeric_limits<double>::infinity();
        if (ImGui::Button("Reload Shader Files")) {
            last_time = glfwGetTime();
            failures = 0;
            failures += entity_renderer.refresh_shaders() ? 0 : 1;
            failures += animated_entity_renderer.refresh_shaders() ? 0 : 1;
            failures += emissive_entity_renderer.refresh_shaders() ? 0 : 1;
        }
        if (glfwGetTime() - 2.0 <= last_time) {
            ImGui::SameLine();
            if (failures == 0) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
                ImGui::Text("Success!");
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.0, 0.0, 1.0f));
                ImGui::Text("[%d] Failed, see Console", failures);
            }
            ImGui::PopStyleColor();
        }
    }
}
