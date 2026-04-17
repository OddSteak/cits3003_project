#include "AnimatedEntityElement.h"

#include <glm/gtx/transform.hpp>

#include "rendering/imgui/ImGuiManager.h"
#include "scene/SceneContext.h"

std::unique_ptr<EditorScene::AnimatedEntityElement> EditorScene::AnimatedEntityElement::new_default(const SceneContext& scene_context, ElementRef parent) {
    auto rendered_entity = AnimatedEntityRenderer::Entity::create(
        scene_context.model_loader.load_hierarchy_from_file<AnimatedEntityRenderer::VertexData>("cube.obj"),
        AnimatedEntityRenderer::InstanceData{glm::mat4{}, AnimatedEntityRenderer::EntityMaterial{
            {1.0f, 1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f},
            512.0f,
        }},
        AnimatedEntityRenderer::RenderData{
            scene_context.texture_loader.default_white_texture(),
            scene_context.texture_loader.default_white_texture()
        }
    );

    auto new_entity = std::make_unique<AnimatedEntityElement>(
        parent,
        "New Animated Entity",
        glm::vec3{0.0f},
        glm::vec3{0.0f},
        glm::vec3{1.0f},
        rendered_entity
    );

    new_entity->update_instance_data();
    return new_entity;
}

std::unique_ptr<EditorScene::AnimatedEntityElement> EditorScene::AnimatedEntityElement::from_json(const SceneContext& scene_context, EditorScene::ElementRef parent, const json& j) {
    auto new_entity = new_default(scene_context, parent);

    new_entity->update_local_transform_from_json(j);
    new_entity->update_material_from_json(j);

    new_entity->rendered_entity->mesh_hierarchy = scene_context.model_loader.load_hierarchy_from_file<AnimatedEntityRenderer::VertexData>(j["model"]);
    new_entity->rendered_entity->render_data.diffuse_texture = texture_from_json(scene_context, j["diffuse_texture"]);
    new_entity->rendered_entity->render_data.specular_map_texture = texture_from_json(scene_context, j["specular_map_texture"]);

    json animation_parameters = j["animation_parameters"];
    new_entity->animation_parameters.animation_id = animation_parameters["animation_id"];
    new_entity->animation_parameters.speed = animation_parameters["speed"];
    new_entity->animation_parameters.paused = animation_parameters["paused"];
    new_entity->animation_parameters.loop = animation_parameters["loop"];
    new_entity->rendered_entity->animation_id = animation_parameters["animation_id"];
    new_entity->rendered_entity->animation_time_seconds = animation_parameters["animation_time_seconds"];

    new_entity->update_instance_data();
    return new_entity;
}

json EditorScene::AnimatedEntityElement::into_json() const {
    if (!rendered_entity->mesh_hierarchy->filename.has_value()) {
        return {
            {"error", Formatter() << "Animated Entity [" << name << "]'s model does not have a filename so can not be exported, and has been skipped."}
        };
    }

    return {
        local_transform_into_json(),
        material_into_json(),
        {"model", rendered_entity->mesh_hierarchy->filename.value()},
        {"diffuse_texture", texture_to_json(rendered_entity->render_data.diffuse_texture)},
        {"specular_map_texture", texture_to_json(rendered_entity->render_data.specular_map_texture)},
        {"animation_parameters", {
            {"animation_id", animation_parameters.animation_id},
            {"speed", animation_parameters.speed},
            {"paused", animation_parameters.paused},
            {"loop", animation_parameters.loop},
            {"animation_time_seconds", rendered_entity->animation_time_seconds},
        }}
    };
}

void EditorScene::AnimatedEntityElement::add_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context) {
    ImGui::Text("Animated Entity");
    SceneElement::add_imgui_edit_section(render_scene, scene_context);

    add_local_transform_imgui_edit_section(render_scene, scene_context);
    add_material_imgui_edit_section(render_scene, scene_context);

    ImGui::Text("Model & Textures");
    if (scene_context.model_loader.add_imgui_hierarchy_selector("Model Selection", rendered_entity->mesh_hierarchy)) {
        animation_parameters.animation_id = NONE_ANIMATION;
        rendered_entity->animation_time_seconds = 0.0;
    }
    scene_context.texture_loader.add_imgui_texture_selector("Diffuse Texture", rendered_entity->render_data.diffuse_texture);
    scene_context.texture_loader.add_imgui_texture_selector("Specular Map", rendered_entity->render_data.specular_map_texture, false);
    ImGui::Spacing();
}

void EditorScene::AnimatedEntityElement::update_instance_data() {
    transform = calc_model_matrix();

    if (!EditorScene::is_null(parent)) {
        // Post multiply by transform so that local transformations are applied first
        transform = (*parent)->transform * transform;
    }

    rendered_entity->instance_data.model_matrix = transform;
    rendered_entity->instance_data.material = material;
}

const char* EditorScene::AnimatedEntityElement::element_type_name() const {
    return ELEMENT_TYPE_NAME;
}

std::shared_ptr<AnimatedEntityInterface> EditorScene::AnimatedEntityElement::get_entity() {
    return std::dynamic_pointer_cast<AnimatedEntityInterface>(rendered_entity);
}

AnimationParameters& EditorScene::AnimatedEntityElement::get_animation_parameters() {
    return animation_parameters;
}
