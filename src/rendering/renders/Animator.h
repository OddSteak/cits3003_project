#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <unordered_map>

#include "rendering/scene/RenderedEntity.h"

struct AnimationParameters {
    uint animation_id = NONE_ANIMATION;
    bool loop = false;
    bool paused = false;
    double speed = 1.0;
};

/// A class for controlling the animation for a set of animatable entities
class Animator {
    std::unordered_map<std::shared_ptr<AnimatedEntityInterface>, AnimationParameters> animated_entities{};
public:
    /// Animated each playing entity, incrementing time by dt.
    void animate(double dt);

    /// Start animating an entity with the given parameters. If it was already present then reset to t=0 and use new parameters.
    template<class AnimatedEntity>
    void start(std::shared_ptr<AnimatedEntity> animated_entity, AnimationParameters animation_parameters);

    /// Update the parameters on an animating entity with the given parameters.
    /// If it was not already present then nothing happens.
    template<class AnimatedEntity>
    void update_param(std::shared_ptr<AnimatedEntity> animated_entity, AnimationParameters animation_parameters);

    /// Pause an animating entity if it's currently play
    template<class AnimatedEntity>
    void pause(std::shared_ptr<AnimatedEntity> animated_entity);

    /// Resumes a paused entity, also updating the animation parameters.
    /// If the entity is not currently present, then start animating it with these parameters at t=0
    template<class AnimatedEntity>
    void resume(std::shared_ptr<AnimatedEntity> animated_entity, AnimationParameters animation_parameters);

    /// Checks if an entity is currently animating, if so returns it's current parameters.
    template<class AnimatedEntity>
    std::optional<AnimationParameters> is_animating(const std::shared_ptr<AnimatedEntity>& animated_entity);

    /// Stop an entity from animationg, does nothing it it was already stopped.
    template<class AnimatedEntity>
    void stop(const std::shared_ptr<AnimatedEntity>& animated_entity);
};

template<class AnimatedEntity>
void Animator::start(std::shared_ptr<AnimatedEntity> animated_entity, AnimationParameters animation_parameters) {
    stop(animated_entity);
    auto aei = std::dynamic_pointer_cast<AnimatedEntityInterface>(animated_entity);
    aei->get_animation_id() = animation_parameters.animation_id;
    aei->get_animation_time_seconds() = 0.0;
    animated_entities[aei] = animation_parameters;
}

template<class AnimatedEntity>
void Animator::update_param(std::shared_ptr<AnimatedEntity> animated_entity, AnimationParameters animation_parameters) {
    auto aei = std::dynamic_pointer_cast<AnimatedEntityInterface>(animated_entity);
    auto param = animated_entities.find(aei);
    if (param != animated_entities.end()) {
        param->second = animation_parameters;
    }
}

template<class AnimatedEntity>
void Animator::pause(std::shared_ptr<AnimatedEntity> animated_entity) {
    auto aei = std::dynamic_pointer_cast<AnimatedEntityInterface>(animated_entity);
    auto param = animated_entities.find(aei);
    if (param != animated_entities.end()) {
        param->second.paused = true;
    }
}

template<class AnimatedEntity>
void Animator::resume(std::shared_ptr<AnimatedEntity> animated_entity, AnimationParameters animation_parameters) {
    auto aei = std::dynamic_pointer_cast<AnimatedEntityInterface>(animated_entity);
    auto param = animated_entities.find(aei);
    if (param != animated_entities.end()) {
        param->second = animation_parameters;
        param->second.paused = false;
    } else {
        aei->get_animation_id() = animation_parameters.animation_id;
        animated_entities[aei] = animation_parameters;
    }
}

template<class AnimatedEntity>
std::optional<AnimationParameters> Animator::is_animating(const std::shared_ptr<AnimatedEntity>& animated_entity) {
    auto aei = std::dynamic_pointer_cast<AnimatedEntityInterface>(animated_entity);
    auto param = animated_entities.find(aei);
    if (param != animated_entities.end()) {
        return param->second;
    } else {
        return std::nullopt;
    }
}

template<class AnimatedEntity>
void Animator::stop(const std::shared_ptr<AnimatedEntity>& animated_entity) {
    auto aei = std::dynamic_pointer_cast<AnimatedEntityInterface>(animated_entity);
    aei->get_animation_id() = NONE_ANIMATION;
    aei->get_animation_time_seconds() = 0.0;
    animated_entities.erase(aei);
}

#endif //ANIMATOR_H
