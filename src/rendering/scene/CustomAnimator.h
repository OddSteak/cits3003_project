#ifndef CUSTOM_ANIMATOR_H
#define CUSTOM_ANIMATOR_H

#include <functional>
#include <optional>
#include <unordered_map>

#include <glm/glm.hpp>

#include "rendering/scene/RenderedEntity.h"

struct CustomMotionParams {
    glm::vec3 translation_velocity = {0.0f, 0.0f, 0.0f}; // units/second
    glm::vec3 rotation_velocity    = {0.0f, 0.0f, 0.0f}; // degrees/second
    bool enabled = false;
};

class CustomAnimator {
    struct AnimatedEntry {
        CustomMotionParams parameters;
        glm::vec3 initial_position;
        glm::vec3 initial_rotation;
        glm::vec3* position_ptr;
        glm::vec3* rotation_ptr;
        std::function<void()> update_fn;
        bool paused = false;
    };

    std::unordered_map<std::shared_ptr<AnimatedEntityInterface>, AnimatedEntry> animated_entities{};

public:
    void animate(double dt);

    template <class AnimatedEntity>
    void start(std::shared_ptr<AnimatedEntity> animated_entity,
               CustomMotionParams params,
               glm::vec3& position,
               glm::vec3& rotation,
               std::function<void()> update_fn);

    template <class AnimatedEntity>
    void update_param(std::shared_ptr<AnimatedEntity> animated_entity,
                      CustomMotionParams params);

    template <class AnimatedEntity>
    void pause(std::shared_ptr<AnimatedEntity> animated_entity);

    template <class AnimatedEntity>
    void resume(std::shared_ptr<AnimatedEntity> animated_entity,
                CustomMotionParams params);

    template <class AnimatedEntity>
    std::optional<CustomMotionParams> is_animating(const std::shared_ptr<AnimatedEntity>& animated_entity);

    template <class AnimatedEntity>
    void stop(const std::shared_ptr<AnimatedEntity>& animated_entity);

    void stop_all();
};

template <class AnimatedEntity>
void CustomAnimator::start(std::shared_ptr<AnimatedEntity> animated_entity,
                           CustomMotionParams params,
                           glm::vec3& position,
                           glm::vec3& rotation,
                           std::function<void()> update_fn) {
    stop(animated_entity);
    auto aei = std::dynamic_pointer_cast<AnimatedEntityInterface>(animated_entity);
    animated_entities[aei] = AnimatedEntry{
        params,
        position,   // snapshot of position at start time
        rotation,   // snapshot of rotation at start time
        &position,
        &rotation,
        update_fn,
        false
    };
}

template <class AnimatedEntity>
void CustomAnimator::update_param(std::shared_ptr<AnimatedEntity> animated_entity,
                                  CustomMotionParams params) {
    auto aei = std::dynamic_pointer_cast<AnimatedEntityInterface>(animated_entity);
    auto it = animated_entities.find(aei);
    if (it != animated_entities.end()) {
        it->second.parameters = params;
    }
}

template <class AnimatedEntity>
void CustomAnimator::pause(std::shared_ptr<AnimatedEntity> animated_entity) {
    auto aei = std::dynamic_pointer_cast<AnimatedEntityInterface>(animated_entity);
    auto it = animated_entities.find(aei);
    if (it != animated_entities.end()) {
        it->second.paused = true;
    }
}

template <class AnimatedEntity>
void CustomAnimator::resume(std::shared_ptr<AnimatedEntity> animated_entity,
                            CustomMotionParams params) {
    auto aei = std::dynamic_pointer_cast<AnimatedEntityInterface>(animated_entity);
    auto it = animated_entities.find(aei);
    if (it != animated_entities.end()) {
        it->second.parameters = params;
        it->second.paused = false;
    }
}

template <class AnimatedEntity>
std::optional<CustomMotionParams>
CustomAnimator::is_animating(const std::shared_ptr<AnimatedEntity>& animated_entity) {
    auto aei = std::dynamic_pointer_cast<AnimatedEntityInterface>(animated_entity);
    auto it = animated_entities.find(aei);
    if (it != animated_entities.end()) {
        return it->second.parameters;
    }
    return std::nullopt;
}

template <class AnimatedEntity>
void CustomAnimator::stop(const std::shared_ptr<AnimatedEntity>& animated_entity) {
    auto aei = std::dynamic_pointer_cast<AnimatedEntityInterface>(animated_entity);
    auto it = animated_entities.find(aei);
    if (it != animated_entities.end()) {
        *it->second.position_ptr = it->second.initial_position;
        *it->second.rotation_ptr = it->second.initial_rotation;
        it->second.update_fn();
        animated_entities.erase(it);
    }
}

#endif // CUSTOM_ANIMATOR_H
