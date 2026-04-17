#include "SceneManager.h"
#include "scene/SceneContext.h"

#include <vector>

#include <imgui/imgui.h>

SceneGeneratorHandle SceneManager::register_scene_generator(std::string name, std::function<std::shared_ptr<SceneInterface>()> scene_generator) {
    int id = next_handle++;
    SceneGeneratorHandle handle{id};

    scene_generators.insert({id, {
        std::move(name),
        std::move(scene_generator)
    }});
    ordered_scene_generators.push_back(id);

    return handle;
}

void SceneManager::switch_scene(const SceneGeneratorHandle& generator_handle, const SceneContext& scene_context) {
    switch_scene((scene_generators[generator_handle.handle].generator)(), scene_context);
}

void SceneManager::switch_scene(std::shared_ptr<SceneInterface> scene, const SceneContext& scene_context) {
    if (current_scene != nullptr) {
        current_scene->close(scene_context);
    }
    current_scene = std::move(scene);

    scene_context.window.set_title_suffix(std::nullopt);
    current_scene->open(scene_context);
}

void SceneManager::add_imgui_options_section(const SceneContext& scene_context) {
    if (ImGui::CollapsingHeader("Scene Manager")) {
        if (!scene_generators.empty()) {
            selected_scene_generator = std::min(selected_scene_generator, (int) ordered_scene_generators.size());
            if (ImGui::BeginCombo("Scene Generator Selector", scene_generators[ordered_scene_generators[selected_scene_generator]].name.c_str(), 0)) {
                for (int n = 0; n < (int) ordered_scene_generators.size(); n++) {
                    const bool is_selected = (selected_scene_generator == n);
                    if (ImGui::Selectable(scene_generators[ordered_scene_generators[n]].name.c_str(), is_selected))
                        selected_scene_generator = n;

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if (ImGui::Button("Generate")) {
                switch_scene((scene_generators[ordered_scene_generators[selected_scene_generator]].generator)(), scene_context);
            }
        }
    }

    if (current_scene != nullptr) {
        current_scene->add_imgui_options_section();
        current_scene->get_camera().add_imgui_options_section(scene_context);
    }
}

void SceneManager::tick_scene(const SceneContext& scene_context) {
    if (current_scene == nullptr) return;
    auto& window = scene_context.window;
    auto& window_manager = scene_context.window_manager;

    auto dt = (float) window_manager.get_delta_time();
    auto tick_response = current_scene->tick(dt, scene_context);
    current_scene->get_camera().update(window, dt, true);
    current_scene->get_render_scene().use_camera(current_scene->get_camera());

    switch (tick_response.first) {
        case TickResponseType::Continue:
            break;
        case TickResponseType::Exit:
            window.set_should_close();
            break;
        case TickResponseType::ChangeScene: {
            if (tick_response.second == nullptr) {
                std::cerr << "TickResponseType::ChangeScene must be accompanied by a null null Scene" << std::endl;
                exit(EXIT_FAILURE);
            }
            switch_scene(tick_response.second, scene_context);
            break;
        }
    }
}

const std::shared_ptr<SceneInterface>& SceneManager::get_current_scene() const {
    return current_scene;
}

void SceneManager::cleanup(const SceneContext& scene_context) {
    if (current_scene != nullptr) {
        current_scene->close(scene_context);
        current_scene = nullptr;
    }
    scene_generators.clear();
}
