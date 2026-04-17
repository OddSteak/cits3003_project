#include "BasicStaticScene.h"

#include <glm/gtx/transform.hpp>

#include "rendering/imgui/ImGuiManager.h"
#include "rendering/cameras/PanningCamera.h"
#include "rendering/cameras/FlyingCamera.h"
#include "scene/SceneContext.h"

/// Nothing to do in the constructor
BasicStaticScene::BasicStaticScene() = default;

void BasicStaticScene::open(const SceneContext& scene_context) {
    /// Load all the needed models and textures
    auto plane = scene_context.model_loader.load_from_file<EntityRenderer::VertexData>("double_plane.obj");
    auto default_black_texture = scene_context.texture_loader.default_black_texture();

    auto model = scene_context.model_loader.load_from_file<EntityRenderer::VertexData>("crate.obj");
    auto texture = scene_context.texture_loader.load_from_file("crate.png");
    auto specular_map = scene_context.texture_loader.load_from_file("crate_specular.png", false);

    auto light_sphere = scene_context.model_loader.load_from_file<EntityRenderer::VertexData>("sphere.obj");
    auto default_white_texture = scene_context.texture_loader.default_white_texture();

    auto cone = scene_context.model_loader.load_from_file<EntityRenderer::VertexData>("cone.obj");
    auto cone_diffuse = scene_context.texture_loader.load_from_file("cone_diffuse.png", true, true);
    auto cone_specular = scene_context.texture_loader.load_from_file("cone_specular.png", false, true);
    auto cone_retro_map = scene_context.texture_loader.load_from_file("cone_retro_map.png", false, true);

    auto light_pos = glm::vec3(2.0f, 3.0f, 4.0f);
    auto light_col = glm::vec3(1.0f);

    /// Crate the ground plane entity
    auto plane_entity = EntityRenderer::Entity::create(
        plane,
        EntityRenderer::InstanceData{
            glm::translate(glm::vec3(0.0f, -0.01f, 0.0f)) * glm::scale(glm::vec3(10.0f, 1.0f, 10.0f)),
            EntityRenderer::EntityMaterial{
                glm::vec4(1.0f),
                glm::vec4(1.0f),
                glm::vec4(1.0f),
                128.0f,
            }
        },
        EntityRenderer::RenderData{
            default_white_texture,
            default_white_texture
        }
    );

    /// Create the box entity
    box_entity = EntityRenderer::Entity::create(
        model,
        EntityRenderer::InstanceData{
            glm::mat4{1.0f},
            EntityRenderer::EntityMaterial{
                glm::vec4(1.0f),
                glm::vec4(1.0f),
                glm::vec4(1.0f),
                32.0f,
            }
        },
        EntityRenderer::RenderData{
            texture,
            specular_map

        }
    );

    /// Create an emissive sphere to place at the lights position
    auto light_sphere_entity = EmissiveEntityRenderer::Entity::create(
        light_sphere,
        EmissiveEntityRenderer::InstanceData{
            glm::translate(light_pos) * glm::scale(glm::vec3{0.1f}),
            EmissiveEntityRenderer::EmissiveEntityMaterial{
                glm::vec4{light_col, 1.0f}
            }
        },
        EmissiveEntityRenderer::RenderData{
            default_white_texture
        }
    );

    /// Create a cone without any retro-reflective
    auto cone_entity = EntityRenderer::Entity::create(
        cone,
        EntityRenderer::InstanceData{
            glm::translate(glm::vec3{3.0f, 0.0f, 0.0f}),
            EntityRenderer::EntityMaterial{
                glm::vec4(1.0f),
                glm::vec4(glm::vec3(1.0f), 0.75f),
                glm::vec4(1.0f),
                512.0f,
            }
        },
        EntityRenderer::RenderData{
            cone_diffuse,
            cone_specular,
        }
    );

    /// Setup the camera with the default state
    camera = std::make_unique<PanningCamera>(init_distance, init_focus_point, init_pitch, init_yaw, init_near, init_fov);
    /// Tell the scene to use this camera to camera the view and projection matrices.
    /// Just calling this once to make sure the initial values are correct.
    render_scene.use_camera(*camera);

    /// Add the entities to the render scene
    render_scene.insert_entity(plane_entity);
    render_scene.insert_entity(box_entity);
    render_scene.insert_entity(light_sphere_entity);
    render_scene.insert_entity(cone_entity);

    /// Create the point light itself, and add it to the render scene
    auto point_light = PointLight::create(
        light_pos,
        glm::vec4{light_col, 1.0f}
    );

    render_scene.insert_light(point_light);
}

std::pair<TickResponseType, std::shared_ptr<SceneInterface>> BasicStaticScene::tick(float /*delta_time*/, const SceneContext& scene_context) {
    /// If the `Esc` key was pressed this tick, then tell the scene manager to exit
    if (scene_context.window.was_key_pressed(GLFW_KEY_ESCAPE)) {
        return {TickResponseType::Exit, nullptr};
    }

    /// If the 'V' key was pressed this tick, then cycle the camera mode
    if (scene_context.window.was_key_pressed(GLFW_KEY_V)) {
        switch (camera_mode) {
            case CameraMode::Panning:
                set_camera_mode(CameraMode::Flying);
                break;
            case CameraMode::Flying:
                set_camera_mode(CameraMode::Panning);
                break;
        }
    }

    /// Rotate the cube around the y-axis at 10 deg/sec, by calculating a model matrix and applying it to the entity.
    /// NOTE: glfwGetTime() returns the number of seconds since the program started
    glm::mat4 model_matrix = glm::rotate(glm::radians(10.0f * (float) glfwGetTime()), glm::vec3{0, 1, 0});
    box_entity->instance_data.model_matrix = model_matrix;

    /// Default to telling the SceneManager to continue ticking
    return {TickResponseType::Continue, nullptr};
}

void BasicStaticScene::add_imgui_options_section() {
    /// Add a section to the ImGUI menu
    if (ImGui::CollapsingHeader("Scene Settings")) {
        // Add radio buttons to switch between the camera modes
        ImGui::Text("Camera Selection (v)");
        if (ImGui::RadioButton("Panning Camera", camera_mode == CameraMode::Panning)) {
            set_camera_mode(CameraMode::Panning);
        }
        if (ImGui::RadioButton("Flying Camera", camera_mode == CameraMode::Flying)) {
            set_camera_mode(CameraMode::Flying);
        }
        ImGui::Separator();
    }
}

MasterRenderScene& BasicStaticScene::get_render_scene() {
    /// Only 1 RenderScene so always just return that
    return render_scene;
}

CameraInterface& BasicStaticScene::get_camera() {
    /// Return the current camera
    return *camera;
}

void BasicStaticScene::close(const SceneContext& /*scene_context*/) {
    // Free up memory by dropping handles
    box_entity.reset();
    render_scene = {};
}

void BasicStaticScene::set_camera_mode(CameraMode new_camera_mode) {
    /// Extract the camera orientation and use that to switch cameras
    auto orientation = camera->save_properties();
    switch (new_camera_mode) {
        case CameraMode::Panning:
            camera = std::make_unique<PanningCamera>(init_distance, init_focus_point, init_pitch, init_yaw, init_near, init_fov);
            break;
        case CameraMode::Flying:
            camera = std::make_unique<FlyingCamera>(init_position, init_pitch, init_yaw, init_near, init_fov);
            break;
    }
    camera->load_properties(orientation);
    this->camera_mode = new_camera_mode;
}
