#include "AnimatedEntityElement.h"

#include <glm/gtx/transform.hpp>

#include "rendering/imgui/ImGuiManager.h"
#include "scene/SceneContext.h"

std::unique_ptr<EditorScene::AnimatedEntityElement>
EditorScene::AnimatedEntityElement::new_default(
    const SceneContext &scene_context, ElementRef parent) {
  auto rendered_entity = AnimatedEntityRenderer::Entity::create(
      scene_context.model_loader
          .load_hierarchy_from_file<AnimatedEntityRenderer::VertexData>(
              "cube.obj"),
      AnimatedEntityRenderer::InstanceData{
          glm::mat4{},
          AnimatedEntityRenderer::EntityMaterial{
              {1.0f, 1.0f, 1.0f, 1.0f},
              {1.0f, 1.0f, 1.0f, 1.0f},
              {1.0f, 1.0f, 1.0f, 1.0f},
              512.0f,
          }},
      AnimatedEntityRenderer::RenderData{
          scene_context.texture_loader.default_white_texture(),
          scene_context.texture_loader.default_white_texture()});

  auto new_entity = std::make_unique<AnimatedEntityElement>(
      parent, "New Animated Entity", glm::vec3{0.0f}, glm::vec3{0.0f},
      glm::vec3{1.0f}, rendered_entity);

  new_entity->update_instance_data();
  return new_entity;
}

std::unique_ptr<EditorScene::AnimatedEntityElement>
EditorScene::AnimatedEntityElement::from_json(const SceneContext &scene_context,
                                              EditorScene::ElementRef parent,
                                              const json &j) {
  auto new_entity = new_default(scene_context, parent);

  new_entity->update_local_transform_from_json(j);
  new_entity->update_material_from_json(j);

  new_entity->rendered_entity->mesh_hierarchy =
      scene_context.model_loader
          .load_hierarchy_from_file<AnimatedEntityRenderer::VertexData>(
              j["model"]);
  new_entity->rendered_entity->render_data.diffuse_texture =
      texture_from_json(scene_context, j["diffuse_texture"]);
  new_entity->rendered_entity->render_data.specular_map_texture =
      texture_from_json(scene_context, j["specular_map_texture"]);

  json animation_parameters = j["animation_parameters"];
  new_entity->animation_parameters.animation_id =
      animation_parameters["animation_id"];
  new_entity->animation_parameters.speed = animation_parameters["speed"];
  new_entity->animation_parameters.paused = animation_parameters["paused"];
  new_entity->animation_parameters.loop = animation_parameters["loop"];
  new_entity->rendered_entity->animation_id =
      animation_parameters["animation_id"];
  new_entity->rendered_entity->animation_time_seconds =
      animation_parameters["animation_time_seconds"];

  if (j.contains("motion_parameters")) {
    auto mp = j["motion_parameters"];
    new_entity->motion_parameters.enabled = mp.value("enabled", false);
    int motion_type_int = mp.value("motion_type", 0);
    new_entity->motion_parameters.motion_type = static_cast<CustomMotionType>(motion_type_int);
    if (mp.contains("move_initial"))
      new_entity->motion_parameters.move_initial = mp["move_initial"];
    if (mp.contains("move_final"))
      new_entity->motion_parameters.move_final = mp["move_final"];
    new_entity->motion_parameters.move_speed = mp.value("move_speed", 1.0f);
    if (mp.contains("rotation_velocity"))
      new_entity->motion_parameters.rotation_velocity = mp["rotation_velocity"];
  }

  new_entity->update_instance_data();
  return new_entity;
}

json EditorScene::AnimatedEntityElement::into_json() const {
  if (!rendered_entity->mesh_hierarchy->filename.has_value()) {
    return {{"error", Formatter()
                          << "Animated Entity [" << name
                          << "]'s model does not have a filename so can not be "
                             "exported, and has been skipped."}};
  }

  return {
      local_transform_into_json(),
      material_into_json(),
      {"model", rendered_entity->mesh_hierarchy->filename.value()},
      {"diffuse_texture",
       texture_to_json(rendered_entity->render_data.diffuse_texture)},
      {"specular_map_texture",
       texture_to_json(rendered_entity->render_data.specular_map_texture)},
      {"animation_parameters",
       {
           {"animation_id", animation_parameters.animation_id},
           {"speed", animation_parameters.speed},
           {"paused", animation_parameters.paused},
           {"loop", animation_parameters.loop},
           {"animation_time_seconds", rendered_entity->animation_time_seconds},
       }},
      {"motion_parameters",
       {
           {"motion_type", static_cast<int>(motion_parameters.motion_type)},
           {"enabled", motion_parameters.enabled},
           {"move_initial", motion_parameters.move_initial},
           {"move_final", motion_parameters.move_final},
           {"move_speed", motion_parameters.move_speed},
           {"rotation_velocity", motion_parameters.rotation_velocity},
       }}};
}

void EditorScene::AnimatedEntityElement::add_imgui_edit_section(
    MasterRenderScene &render_scene, const SceneContext &scene_context) {
  ImGui::Text("Animated Entity");
  SceneElement::add_imgui_edit_section(render_scene, scene_context);

  add_local_transform_imgui_edit_section(render_scene, scene_context);

  ImGui::Text("Model & Textures");
  if (scene_context.model_loader.add_imgui_hierarchy_selector(
          "Model Selection", rendered_entity->mesh_hierarchy)) {
    animation_parameters.animation_id = NONE_ANIMATION;
    rendered_entity->animation_time_seconds = 0.0;
    // Reset ghosts so they're recreated with the new mesh next sync
    remove_move_ghosts(render_scene);
    ghost_initial = nullptr;
    ghost_final = nullptr;
  }
  scene_context.texture_loader.add_imgui_texture_selector(
      "Diffuse Texture", rendered_entity->render_data.diffuse_texture);
  scene_context.texture_loader.add_imgui_texture_selector(
      "Specular Map", rendered_entity->render_data.specular_map_texture, false);

  add_material_imgui_edit_section(render_scene, scene_context);

}

void EditorScene::AnimatedEntityElement::update_instance_data() {
  transform = calc_model_matrix();

  if (!EditorScene::is_null(parent)) {
    // Post multiply by transform so that local transformations are applied
    // first
    transform = (*parent)->transform * transform;
  }

  rendered_entity->instance_data.model_matrix = transform;
  rendered_entity->instance_data.material = material;
}

const char *EditorScene::AnimatedEntityElement::element_type_name() const {
  return ELEMENT_TYPE_NAME;
}

glm::mat4 EditorScene::AnimatedEntityElement::calc_ghost_matrix(glm::vec3 ghost_pos) const {
  glm::mat4 rot = glm::rotate(glm::mat4(1.0f), euler_rotation.x, {1.0f, 0.0f, 0.0f});
  rot = glm::rotate(rot, euler_rotation.y, {0.0f, 1.0f, 0.0f});
  rot = glm::rotate(rot, euler_rotation.z, {0.0f, 0.0f, 1.0f});
  glm::mat4 local = glm::translate(ghost_pos) * rot * glm::scale(scale);
  if (!is_null(parent)) return (*parent)->transform * local;
  return local;
}

void EditorScene::AnimatedEntityElement::sync_move_ghosts(MasterRenderScene &render_scene) {
  if (motion_parameters.motion_type != CustomMotionType::Move) {
    if (ghosts_in_scene) remove_move_ghosts(render_scene);
    return;
  }

  // Create ghost entities (share mesh and textures, distinct tinted materials)
  if (!ghost_initial) {
    ghost_initial = AnimatedEntityRenderer::Entity::create(
        rendered_entity->mesh_hierarchy,
        AnimatedEntityRenderer::InstanceData{
            glm::mat4{1.0f},
            AnimatedEntityRenderer::EntityMaterial{
                {0.1f, 0.4f, 1.0f, 1.0f},   // ambient: blue
                {0.1f, 0.4f, 1.0f, 1.0f},   // diffuse: blue
                {0.0f, 0.0f, 0.0f, 1.0f},   // no specular
                1.0f,
            }},
        rendered_entity->render_data);
  }
  if (!ghost_final) {
    ghost_final = AnimatedEntityRenderer::Entity::create(
        rendered_entity->mesh_hierarchy,
        AnimatedEntityRenderer::InstanceData{
            glm::mat4{1.0f},
            AnimatedEntityRenderer::EntityMaterial{
                {1.0f, 0.5f, 0.1f, 1.0f},   // ambient: orange
                {1.0f, 0.5f, 0.1f, 1.0f},   // diffuse: orange
                {0.0f, 0.0f, 0.0f, 1.0f},   // no specular
                1.0f,
            }},
        rendered_entity->render_data);
  }

  if (!ghosts_in_scene) {
    render_scene.insert_entity(ghost_initial);
    render_scene.insert_entity(ghost_final);
    ghosts_in_scene = true;
  }

  ghost_initial->instance_data.model_matrix = calc_ghost_matrix(motion_parameters.move_initial);
  ghost_final->instance_data.model_matrix = calc_ghost_matrix(motion_parameters.move_final);
}

void EditorScene::AnimatedEntityElement::remove_move_ghosts(MasterRenderScene &render_scene) {
  if (ghosts_in_scene) {
    render_scene.remove_entity(ghost_initial);
    render_scene.remove_entity(ghost_final);
    ghosts_in_scene = false;
  }
}

std::shared_ptr<AnimatedEntityInterface>
EditorScene::AnimatedEntityElement::get_entity() {
  return std::dynamic_pointer_cast<AnimatedEntityInterface>(rendered_entity);
}

AnimationParameters &
EditorScene::AnimatedEntityElement::get_animation_parameters() {
  return animation_parameters;
}
