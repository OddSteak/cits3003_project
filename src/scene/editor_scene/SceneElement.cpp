#include "SceneElement.h"
#include "scene/SceneContext.h"
#include "rendering/imgui/ImGuiManager.h"

void EditorScene::SceneElement::add_imgui_edit_section(MasterRenderScene& /*render_scene*/, const SceneContext& /*scene_context*/) {
    ImGui::InputText("Name", &name, 0);
    ImGui::Spacing();
}

void EditorScene::SceneElement::visit_children_recursive(const std::function<void(SceneElement&)>& fn) const {
    auto children = get_children();
    if (children == nullptr) return;
    for (auto& child: *children) {
        fn(*child);
        child->visit_children_recursive(fn);
    }
}

json EditorScene::SceneElement::texture_to_json(const std::shared_ptr<TextureHandle>& texture) {
    if (!texture->get_filename().has_value()) {
        return {
            {"error", Formatter() << "Texture does not have a filename so can not be exported, and has been skipped."}
        };
    }

    return {
        {"filename",   texture->get_filename().value()},
        {"is_srgb",    texture->is_srgb()},
        {"is_flipped", texture->is_flipped()},
    };
}

std::shared_ptr<TextureHandle> EditorScene::SceneElement::texture_from_json(const SceneContext& scene_context, const json& json) {
    if (json.contains("error")) {
        return scene_context.texture_loader.default_white_texture();
    }

    return scene_context.texture_loader.load_from_file(json["filename"], json["is_srgb"], json["is_flipped"]);
}

void EditorScene::LocalTransformComponent::add_local_transform_imgui_edit_section(MasterRenderScene& /*render_scene*/, const SceneContext& scene_context) {
    ImGui::Text("Local Transformation");
    bool transformUpdated = false;
    transformUpdated |= ImGui::DragFloat3("Translation", &position[0], 0.01f);
    ImGui::DragDisableCursor(scene_context.window);

    glm::vec3 euler_rotation_degrees = glm::degrees(euler_rotation);
    transformUpdated |= ImGui::DragFloat3("Rotation", &euler_rotation_degrees[0]);
    ImGui::DragDisableCursor(scene_context.window);
    euler_rotation = glm::radians(glm::mod(euler_rotation_degrees, 360.0f));

    {
        // Static also means that all [EntityElement] will share the value
        static bool lock_scale = true;

        glm::vec3 temp_scale = scale;
        if (ImGui::DragFloat3("Scale", &temp_scale[0], 0.01f)) {
            transformUpdated = true;

            if (lock_scale) {
                // Assume only channel can change at a time (I think this is true based on how ImGui works?)
                if (temp_scale.x != scale.x) {
                    if (scale.x == 0.0f) {
                        scale = glm::vec3(temp_scale.x);
                    } else {
                        scale *= temp_scale.x / scale.x;
                    }
                } else if (temp_scale.y != scale.y) {
                    if (scale.y == 0.0f) {
                        scale = glm::vec3(temp_scale.y);
                    } else {
                        scale *= temp_scale.y / scale.y;
                    }
                } else if (temp_scale.z != scale.z) {
                    if (scale.z == 0.0f) {
                        scale = glm::vec3(temp_scale.z);
                    } else {
                        scale *= temp_scale.z / scale.z;
                    }
                }
            } else {
                scale = temp_scale;
            }
        }

        ImGui::SameLine();
        ImGui::Checkbox("[Lock]", &lock_scale);
    }
    ImGui::Spacing();

    if (transformUpdated) {
        update_instance_data();
    }
}

glm::mat4 EditorScene::LocalTransformComponent::calc_model_matrix() const {
    return glm::translate(position) * glm::scale(scale);
}

void EditorScene::LocalTransformComponent::update_local_transform_from_json(const json& json) {
    auto t = json["local_transform"];
    position = t["position"];
    euler_rotation = t["euler_rotation"];
    scale = t["scale"];
}

json EditorScene::LocalTransformComponent::local_transform_into_json() const {
    return {"local_transform", {
        {"position", position},
        {"euler_rotation", euler_rotation},
        {"scale", scale},
    }};
}

void EditorScene::LitMaterialComponent::add_material_imgui_edit_section(MasterRenderScene& /*render_scene*/, const SceneContext& /*scene_context*/) {
    // Set this to true if the user has changed any of the material values, otherwise the changes won't be propagated
    bool material_changed = false;
    ImGui::Text("Material");

    // Add UI controls here

    ImGui::Spacing();
    if (material_changed) {
        update_instance_data();
    }
}

void EditorScene::LitMaterialComponent::update_material_from_json(const json& json) {
    auto m = json["material"];
    material.diffuse_tint = m["diffuse_tint"];
    material.specular_tint = m["specular_tint"];
    material.ambient_tint = m["ambient_tint"];
    material.shininess = m["shininess"];
}

json EditorScene::LitMaterialComponent::material_into_json() const {
    return {"material", {
        {"diffuse_tint", material.diffuse_tint},
        {"specular_tint", material.specular_tint},
        {"ambient_tint", material.ambient_tint},
        {"shininess", material.shininess},
    }};
}

void EditorScene::EmissiveMaterialComponent::add_emissive_material_imgui_edit_section(MasterRenderScene& /*render_scene*/, const SceneContext& /*scene_context*/) {
    // Set this to true if the user has changed any of the material values, otherwise the changes won't be propagated
    bool material_changed = false;
    ImGui::Text("Emissive Material");

    // Add UI controls here

    ImGui::Spacing();
    if (material_changed) {
        update_instance_data();
    }
}

void EditorScene::EmissiveMaterialComponent::update_emissive_material_from_json(const json& json) {
    auto m = json["material"];
    material.emission_tint = m["emission_tint"];
}

json EditorScene::EmissiveMaterialComponent::emissive_material_into_json() const {
    return {"material", {
        {"emission_tint", material.emission_tint},
    }};
}

void EditorScene::AnimationComponent::add_animation_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& /*scene_context*/) {
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    auto entity = get_entity();
    const auto& animations = entity->get_animations();

    ImGui::Text("Animation");
    std::string selected_animation = "[NONE]";
    double ticks_per_second = 1.0;
    double duration_ticks = 0.0;
    if (get_animation_parameters().animation_id != NONE_ANIMATION) {
        std::tie(selected_animation, ticks_per_second, duration_ticks) = animations[get_animation_parameters().animation_id];
    }
    if (ImGui::BeginCombo("Animation Selection", selected_animation.c_str(), 0)) {
        for (auto i = 0u; i < animations.size(); ++i) {
            const auto& animation = animations[i];
            const bool is_selected = i == get_animation_parameters().animation_id;
            if (ImGui::Selectable(std::get<0>(animation).c_str(), is_selected)) {
                render_scene.animator.stop(entity);
                get_animation_parameters().animation_id = i;
                entity->get_animation_time_seconds() = 0.0;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        if (ImGui::Selectable("[NONE]", get_animation_parameters().animation_id == NONE_ANIMATION)) {
            render_scene.animator.stop(entity);
            get_animation_parameters().animation_id = NONE_ANIMATION;
            entity->get_animation_time_seconds() = 0.0;
        }
        ImGui::EndCombo();

        entity->get_animation_id() = get_animation_parameters().animation_id;
    }
    if (get_animation_parameters().animation_id != NONE_ANIMATION) {
        std::tie(selected_animation, ticks_per_second, duration_ticks) = animations[get_animation_parameters().animation_id];

        auto float_time = (float) entity->get_animation_time_seconds();
        auto float_duration = (float) (duration_ticks / ticks_per_second);
        if (ImGui::SliderFloat("Animation Time (sec)", &float_time, 0.0f, float_duration, "%.3f", ImGuiSliderFlags_NoRoundToFormat)) {
            entity->get_animation_time_seconds() = float_time;
        }

        bool is_playing = render_scene.animator.is_animating(entity).has_value();

        if (ImGui::Button("Start")) {
            render_scene.animator.start(entity, get_animation_parameters());
        }

        ImGui::SameLine();

        if (!is_playing) ImGui::BeginDisabled();
        if (ImGui::Button("Pause")) {
            render_scene.animator.pause(entity);
        }
        if (!is_playing) ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("Resume")) {
            render_scene.animator.resume(entity, get_animation_parameters());
        }

        ImGui::SameLine();

        if (ImGui::Button("Stop")) {
            render_scene.animator.stop(entity);
        }

        ImGui::SameLine();

        if (ImGui::Checkbox("Loop", &get_animation_parameters().loop) && is_playing) {
            render_scene.animator.update_param(entity, get_animation_parameters());
        }

        auto float_speed = (float) get_animation_parameters().speed;
        if (ImGui::SliderFloat("Speed", &float_speed, 0.0, 10.0)) {
            get_animation_parameters().speed = float_speed;
            if (is_playing) {
                render_scene.animator.update_param(entity, get_animation_parameters());
            }
        }
    }
}

bool EditorScene::is_null(const ElementRef& ref) {
    return *((const void**) &ref) == nullptr;
}

bool EditorScene::eq(const ElementRef& e1, const ElementRef& e2) {
    return *((const void**) &e1) == *((const void**) &e2);
}
