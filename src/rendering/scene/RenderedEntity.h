#ifndef RENDERED_ENTITY_H
#define RENDERED_ENTITY_H

#include <memory>

#include "rendering/resources/ModelHandle.h"
#include "rendering/resources/MeshHierarchy.h"

/// A generic RenderedEntity, for use by each Renderer
template<typename VertexData, typename InstanceData, typename RenderData>
struct RenderedEntity {
    std::shared_ptr<ModelHandle<VertexData>> model;
    InstanceData instance_data;
    RenderData render_data;

    RenderedEntity(const std::shared_ptr<ModelHandle<VertexData>>& model, InstanceData instance_data, RenderData render_data);

    static std::shared_ptr<RenderedEntity<VertexData, InstanceData, RenderData>> create(std::shared_ptr<ModelHandle<VertexData>> model_handle, InstanceData instance_data, RenderData render_data);
};

template<typename VertexData, typename InstanceData, typename RenderData>
RenderedEntity<VertexData, InstanceData, RenderData>::RenderedEntity(const std::shared_ptr<ModelHandle<VertexData>>& model, InstanceData instance_data, RenderData render_data): model(model), instance_data(instance_data), render_data(render_data) {}

template<typename VertexData, typename InstanceData, typename RenderData>
std::shared_ptr<RenderedEntity<VertexData, InstanceData, RenderData>> RenderedEntity<VertexData, InstanceData, RenderData>::create(std::shared_ptr<ModelHandle<VertexData>> model_handle, InstanceData instance_data, RenderData render_data) {
    return std::make_shared<RenderedEntity<VertexData, InstanceData, RenderData>>(model_handle, instance_data, render_data);
}

/// A base class for type-erased AnimatedEntity stuff.
struct AnimatedEntityInterface {
    // [animation_id] -> (animation_name, ticks_per_second, duration_ticks)
    [[nodiscard]] virtual const std::vector<std::tuple<std::string, double, double>>& get_animations() const = 0;
    [[nodiscard]] virtual uint& get_animation_id() = 0;
    [[nodiscard]] virtual double& get_animation_time_seconds() = 0;
    [[nodiscard]] virtual double get_animation_duration_seconds() const = 0;
    virtual ~AnimatedEntityInterface() = default;
};

/// A generic AnimatedRenderedEntity for use by animated renderers
template<typename VertexData, typename InstanceData, typename RenderData>
struct AnimatedRenderedEntity : public AnimatedEntityInterface {
    std::shared_ptr<MeshHierarchy<VertexData>> mesh_hierarchy;
    InstanceData instance_data;
    RenderData render_data;

    // Animation Data
    uint animation_id = NONE_ANIMATION; // NONE_ANIMATION means disabled
    double animation_time_seconds = 0.0;

    AnimatedRenderedEntity(const std::shared_ptr<MeshHierarchy<VertexData>>& mesh_hierarchy, InstanceData instance_data, RenderData render_data);

    static std::shared_ptr<AnimatedRenderedEntity<VertexData, InstanceData, RenderData>> create(std::shared_ptr<MeshHierarchy<VertexData>> mesh_hierarchy, InstanceData instance_data, RenderData render_data);

    [[nodiscard]] const std::vector<std::tuple<std::string, double, double>>& get_animations() const override {
        return mesh_hierarchy->animations;
    }

    [[nodiscard]] uint& get_animation_id() override {
        return animation_id;
    }

    [[nodiscard]] double& get_animation_time_seconds() override {
        return animation_time_seconds;
    }

    [[nodiscard]] double get_animation_duration_seconds() const override {
        if (animation_id >= mesh_hierarchy->animations.size()) {
            return 0.0f;
        }
        const auto& [animation_name, ticks_per_second, duration_ticks] = mesh_hierarchy->animations[animation_id];
        return duration_ticks / ticks_per_second;
    }
};

template<typename VertexData, typename InstanceData, typename RenderData>
AnimatedRenderedEntity<VertexData, InstanceData, RenderData>::AnimatedRenderedEntity(const std::shared_ptr<MeshHierarchy<VertexData>>& mesh_hierarchy, InstanceData instance_data, RenderData render_data): AnimatedEntityInterface(), mesh_hierarchy(mesh_hierarchy), instance_data(instance_data), render_data(render_data) {}

template<typename VertexData, typename InstanceData, typename RenderData>
std::shared_ptr<AnimatedRenderedEntity<VertexData, InstanceData, RenderData>> AnimatedRenderedEntity<VertexData, InstanceData, RenderData>::create(std::shared_ptr<MeshHierarchy<VertexData>> mesh_hierarchy, InstanceData instance_data, RenderData render_data) {
    return std::make_shared<AnimatedRenderedEntity<VertexData, InstanceData, RenderData>>(mesh_hierarchy, instance_data, render_data);
}

#endif //RENDERED_ENTITY_H
