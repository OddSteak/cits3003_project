#include "EmissiveEntityElement.h"

#include <glm/gtx/transform.hpp>

#include "rendering/imgui/ImGuiManager.h"
#include "scene/SceneContext.h"

std::unique_ptr<EditorScene::EmissiveEntityElement> EditorScene::EmissiveEntityElement::new_default(const SceneContext& scene_context, ElementRef parent) {
    auto rendered_entity = EmissiveEntityRenderer::Entity::create(
        scene_context.model_loader.load_from_file<EmissiveEntityRenderer::VertexData>("cube.obj"),
        EmissiveEntityRenderer::InstanceData{
            glm::mat4{}, // Set via update_instance_data()
            EmissiveEntityRenderer::EmissiveEntityMaterial{
                glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}
            }
        },
        EmissiveEntityRenderer::RenderData{
            scene_context.texture_loader.default_white_texture()
        }
    );

    auto new_entity = std::make_unique<EmissiveEntityElement>(
        parent,
        "New Emissive Entity",
        glm::vec3{0.0f},
        glm::vec3{0.0f},
        glm::vec3{1.0f},
        rendered_entity
    );

    new_entity->update_instance_data();
    return new_entity;
}

std::unique_ptr<EditorScene::EmissiveEntityElement> EditorScene::EmissiveEntityElement::from_json(const SceneContext& scene_context, EditorScene::ElementRef parent, const json& j) {
    auto new_entity = new_default(scene_context, parent);

    new_entity->update_local_transform_from_json(j);
    new_entity->update_emissive_material_from_json(j);

    new_entity->rendered_entity->model = scene_context.model_loader.load_from_file<EmissiveEntityRenderer::VertexData>(j["model"]);
    new_entity->rendered_entity->render_data.emission_texture = texture_from_json(scene_context, j["emission_texture"]);

    new_entity->update_instance_data();
    return new_entity;
}

json EditorScene::EmissiveEntityElement::into_json() const {
    if (!rendered_entity->model->get_filename().has_value()) {
        return {
            {"error", Formatter() << "Entity [" << name << "]'s model does not have a filename so can not be exported, and has been skipped."}
        };
    }

    return {
        local_transform_into_json(),
        emissive_material_into_json(),
        {"model", rendered_entity->model->get_filename().value()},
        {"emission_texture", texture_to_json(rendered_entity->render_data.emission_texture)},
    };
}


void EditorScene::EmissiveEntityElement::add_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context) {
    ImGui::Text("EmissiveEntity");
    SceneElement::add_imgui_edit_section(render_scene, scene_context);

    add_local_transform_imgui_edit_section(render_scene, scene_context);
    add_emissive_material_imgui_edit_section(render_scene, scene_context);

    ImGui::Text("Model & Textures");
    scene_context.model_loader.add_imgui_model_selector("Model Selection", rendered_entity->model);
    scene_context.texture_loader.add_imgui_texture_selector("Emission Texture", rendered_entity->render_data.emission_texture);
    ImGui::Spacing();
}

void EditorScene::EmissiveEntityElement::update_instance_data() {
    transform = calc_model_matrix();

    if (!EditorScene::is_null(parent)) {
        // Post multiply by transform so that local transformations are applied first
        transform = (*parent)->transform * transform;
    }

    rendered_entity->instance_data.model_matrix = transform;
    rendered_entity->instance_data.material = material;
}

const char* EditorScene::EmissiveEntityElement::element_type_name() const {
    return ELEMENT_TYPE_NAME;
}
