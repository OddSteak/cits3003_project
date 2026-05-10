#include "SunLightElement.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/component_wise.hpp>

#include "rendering/imgui/ImGuiManager.h"
#include "scene/SceneContext.h"

std::unique_ptr<EditorScene::SunLightElement> EditorScene::SunLightElement::new_default(const SceneContext& scene_context, EditorScene::ElementRef parent) {
    auto light_element = std::make_unique<SunLightElement>(
        parent,
        "New Directional Light",
        glm::vec3{0.0f, 3.0f, 0.0f},
        glm::vec3{1.0f, -1.0f, 0.0f},
        TheSun::create(
            glm::vec3{}, // Set via update_instance_data()
            glm::vec4{1.0f}
        ),
        // Dark cylinder body
        EmissiveEntityRenderer::Entity::create(
            scene_context.model_loader.load_from_file<EmissiveEntityRenderer::VertexData>("cylinder.obj"),
            EmissiveEntityRenderer::InstanceData{
                glm::mat4{}, // Set via update_instance_data()
                EmissiveEntityRenderer::EmissiveEntityMaterial{
                    glm::vec4{0.4f, 0.4f, 0.4f, 1.0f}
                }
            },
            EmissiveEntityRenderer::RenderData{
                scene_context.texture_loader.default_white_texture()
            }
        ),
        // Bright emitting face (flattened sphere)
        EmissiveEntityRenderer::Entity::create(
            scene_context.model_loader.load_from_file<EmissiveEntityRenderer::VertexData>("sphere.obj"),
            EmissiveEntityRenderer::InstanceData{
                glm::mat4{}, // Set via update_instance_data()
                EmissiveEntityRenderer::EmissiveEntityMaterial{
                    glm::vec4{1.0f}
                }
            },
            EmissiveEntityRenderer::RenderData{
                scene_context.texture_loader.default_white_texture()
            }
        )
    );

    light_element->update_instance_data();
    return light_element;
}

std::unique_ptr<EditorScene::SunLightElement> EditorScene::SunLightElement::from_json(const SceneContext& scene_context, EditorScene::ElementRef parent, const json& j) {
    auto light_element = new_default(scene_context, parent);

    light_element->position = j.value("position", glm::vec3{0.0f, 3.0f, 0.0f});
    light_element->direction = j["direction"];
    light_element->light->colour = j["colour"];
    light_element->visible = j["visible"];
    light_element->visual_scale = j["visual_scale"];

    light_element->update_instance_data();
    return light_element;
}

json EditorScene::SunLightElement::into_json() const {
    return {
        {"position",     position},
        {"direction",    direction},
        {"colour",       light->colour},
        {"visible",      visible},
        {"visual_scale", visual_scale},
    };
}

void EditorScene::SunLightElement::add_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context) {
    ImGui::Text("Directional Light");
    SceneElement::add_imgui_edit_section(render_scene, scene_context);

    ImGui::Text("Local Transformation");
    bool transformUpdated = false;
    transformUpdated |= ImGui::DragFloat3("Position", &position[0], 0.01f);
    ImGui::DragDisableCursor(scene_context.window);
    transformUpdated |= ImGui::DragFloat3("Direction", &direction[0], 0.01f);
    ImGui::DragDisableCursor(scene_context.window);
    ImGui::Spacing();

    ImGui::Text("Light Properties");
    transformUpdated |= ImGui::ColorEdit3("Colour", &light->colour[0]);
    ImGui::Spacing();
    ImGui::DragFloat("Intensity", &light->colour.a, 0.01f, 0.0f, FLT_MAX);
    ImGui::DragDisableCursor(scene_context.window);
    ImGui::DragDisableCursor(scene_context.window);
    ImGui::Spacing();
    ImGui::Text("Visuals");

    transformUpdated |= ImGui::Checkbox("Show Visuals", &visible);
    transformUpdated |= ImGui::DragFloat("Visual Scale", &visual_scale, 0.01f, 0.0f, FLT_MAX);
    ImGui::DragDisableCursor(scene_context.window);

    if (transformUpdated) {
        update_instance_data();
    }
}

void EditorScene::SunLightElement::update_instance_data() {
    light->direction = direction;

    if (!visible) {
        auto inf = glm::scale(glm::vec3{std::numeric_limits<float>::infinity()});
        light_body->instance_data.model_matrix = inf;
        light_face->instance_data.model_matrix = inf;
        return;
    }

    glm::vec3 norm_dir = glm::length(direction) > 0.0f ? glm::normalize(direction) : glm::vec3(0, -1, 0);
    glm::vec3 vis_dir(-norm_dir.x, norm_dir.y, -norm_dir.z);
    glm::vec3 y_axis(0.0f, 1.0f, 0.0f);
    float cos_a = glm::clamp(glm::dot(y_axis, vis_dir), -1.0f, 1.0f);

    glm::mat4 orientation;
    if (cos_a > 0.9999f) {
        orientation = glm::mat4(1.0f);
    } else if (cos_a < -0.9999f) {
        orientation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    } else {
        glm::vec3 rot_axis = glm::normalize(glm::cross(y_axis, vis_dir));
        orientation = glm::rotate(glm::mat4(1.0f), glm::acos(cos_a), rot_axis);
    }

    float body_radius      = 0.12f * visual_scale;
    float body_half_height = 0.20f * visual_scale;
    float face_radius      = 0.14f * visual_scale;

    // Cylinder body: centred at position, Y-axis aligned with vis_dir
    light_body->instance_data.model_matrix =
        glm::translate(position) * orientation *
        glm::scale(glm::vec3{body_radius, body_half_height, body_radius});

    // Bright face: flat disk at the vis_dir tip (the source / top of the cylinder)
    glm::vec3 face_pos = position + vis_dir * body_half_height;
    light_face->instance_data.model_matrix =
        glm::translate(face_pos) * orientation *
        glm::scale(glm::vec3{face_radius, face_radius * 0.08f, face_radius});

    // Colour: body always gray, face matches (normalised) light colour
    glm::vec3 light_rgb = glm::vec3(light->colour);
    float max_c = glm::compMax(light_rgb);
    glm::vec3 norm_colour = max_c > 0.0f ? light_rgb / max_c : glm::vec3(1.0f);

    light_body->instance_data.material.emission_tint = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
    light_face->instance_data.material.emission_tint = glm::vec4(norm_colour, 1.0f);
}

const char* EditorScene::SunLightElement::element_type_name() const {
    return ELEMENT_TYPE_NAME;
}
