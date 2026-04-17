#include "GroupElement.h"

#include <glm/gtx/transform.hpp>

#include "rendering/imgui/ImGuiManager.h"
#include "scene/SceneContext.h"

void EditorScene::GroupElement::add_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context) {
    ImGui::InputText("Group Name", &name, 0);

    add_local_transform_imgui_edit_section(render_scene, scene_context);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Group Animation");
    ImGui::SameLine();
    ImGui::HelpMarker("These buttons send the command to all animated descendents");

    if (ImGui::Button("Start")) {
        visit_children_recursive([&render_scene](SceneElement& child) {
            auto child_ptr = dynamic_cast<AnimationComponent*>(&child);
            if (child_ptr != nullptr) {
                render_scene.animator.start(child_ptr->get_entity(), child_ptr->get_animation_parameters());
            }
        });
    }

    ImGui::SameLine();

    if (ImGui::Button("Pause")) {
        visit_children_recursive([&render_scene](SceneElement& child) {
            auto child_ptr = dynamic_cast<AnimationComponent*>(&child);
            if (child_ptr != nullptr) {
                render_scene.animator.pause(child_ptr->get_entity());
            }
        });
    }

    ImGui::SameLine();

    if (ImGui::Button("Resume")) {
        visit_children_recursive([&render_scene](SceneElement& child) {
            auto child_ptr = dynamic_cast<AnimationComponent*>(&child);
            if (child_ptr != nullptr) {
                render_scene.animator.resume(child_ptr->get_entity(), child_ptr->get_animation_parameters());
            }
        });
    }

    ImGui::SameLine();

    if (ImGui::Button("Stop")) {
        visit_children_recursive([&render_scene](SceneElement& child) {
            auto child_ptr = dynamic_cast<AnimationComponent*>(&child);
            if (child_ptr != nullptr) {
                render_scene.animator.stop(child_ptr->get_entity());
            }
        });
    }
}

std::unique_ptr<EditorScene::GroupElement> EditorScene::GroupElement::from_json(EditorScene::ElementRef parent, const json& j) {
    auto new_group = std::make_unique<GroupElement>(parent, "New Group");

    new_group->update_local_transform_from_json(j);

    new_group->update_instance_data();
    return new_group;
}

json EditorScene::GroupElement::into_json() const {
    return {
        local_transform_into_json(),
    };
}

void EditorScene::GroupElement::update_instance_data() {
    transform = calc_model_matrix();

    if (!EditorScene::is_null(parent)) {
        // Post multiply by transform so that local transformations are applied first
        transform = (*parent)->transform * transform;
    }

    for (const auto& item: (*children)) {
        item->update_instance_data();
    }
}

void EditorScene::GroupElement::add_child(std::unique_ptr<SceneElement> scene_element) {
    children->push_back(std::move(scene_element));
}

const char* EditorScene::GroupElement::element_type_name() const {
    return ELEMENT_TYPE_NAME;
}
