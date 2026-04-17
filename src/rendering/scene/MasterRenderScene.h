#ifndef MASTER_RENDER_SCENE_H
#define MASTER_RENDER_SCENE_H

#include "utility/HelperTypes.h"
#include "Lights.h"
#include "GlobalData.h"
#include "RenderScene.h"
#include "RenderedEntity.h"
#include "Animator.h"
#include "rendering/renders/EntityRenderer.h"
#include "rendering/renders/AnimatedEntityRenderer.h"
#include "rendering/renders/EmissiveEntityRenderer.h"

/// The master render scene, which holds a copy of each renderers RenderScene,
/// as well as the light scene, and offers an interface for adding/removing entities and lights.
/// Also holds the animator, which offers an API for controlling animation.
class MasterRenderScene {
    EntityRenderer::RenderScene entity_scene{};
    AnimatedEntityRenderer::RenderScene animated_entity_scene{};
    EmissiveEntityRenderer::RenderScene emissive_entity_scene{};

    LightScene light_scene{};
public:
    MasterRenderScene() = default;

    Animator animator{};

    void insert_entity(std::shared_ptr<EntityRenderer::Entity> entity);
    void insert_entity(std::shared_ptr<AnimatedEntityRenderer::Entity> entity);
    void insert_entity(std::shared_ptr<EmissiveEntityRenderer::Entity> entity);

    bool remove_entity(const std::shared_ptr<EntityRenderer::Entity>& entity);
    bool remove_entity(const std::shared_ptr<AnimatedEntityRenderer::Entity>& entity);
    bool remove_entity(const std::shared_ptr<EmissiveEntityRenderer::Entity>& entity);

    void insert_light(std::shared_ptr<PointLight> point_light);

    bool remove_light(const std::shared_ptr<PointLight>& point_light);

    /// Propagates a camera state to all the render scenes
    void use_camera(const CameraInterface& camera_interface);

    friend class MasterRenderer;
};

#endif //MASTER_RENDER_SCENE_H
