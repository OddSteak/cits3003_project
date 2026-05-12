#ifndef CUSTOM_ANIMATOR_H
#define CUSTOM_ANIMATOR_H

#include <unordered_map>
#include "rendering/scene/RenderedEntity.h"

struct CustomMotionParams {
  glm::vec3 translation_velocity = {0.0f, 0.0f, 0.0f};
  glm::vec3 rotation_velocity = {0.0f, 0.0f, 0.0f};
  bool enabled = false;
};

class CustomAnimator {
  struct AnimatedEntry {
    CustomMotionParams parameters;
    glm::vec3 position_velocity;
    glm::vec3 rotation_velocity;
  };

  std::unordered_map<std::shared_ptr<AnimatedEntityInterface>, AnimatedEntry>
      animated_entities{};

public:

  void animate(double dt);

  /// Start animating an entity with the given parameters. If it was already
  /// present then reset to t=0 and use new parameters.
  template <class AnimatedEntity>
  void start(std::shared_ptr<AnimatedEntity> animated_entity,
             CustomMotionParams animation_parameters);

  /// Update the parameters on an animating entity with the given parameters.
  /// If it was not already present then nothing happens.
  template <class AnimatedEntity>
  void update_param(std::shared_ptr<AnimatedEntity> animated_entity,
                    CustomMotionParams parameters);

  /// Pause an animating entity if it's currently play
  template <class AnimatedEntity>
  void pause(std::shared_ptr<AnimatedEntity> animated_entity);

  /// Resumes a paused entity, also updating the animation parameters.
  /// If the entity is not currently present, then start animating it with these
  /// parameters at t=0
  template <class AnimatedEntity>
  void resume(std::shared_ptr<AnimatedEntity> animated_entity,
              CustomMotionParams animation_parameters);

  /// Checks if an entity is currently animating, if so returns it's current
  /// parameters.
  template <class AnimatedEntity>
  std::optional<CustomMotionParams()>
  is_animating(const std::shared_ptr<AnimatedEntity> &animated_entity);

  /// Stop an entity from animationg, does nothing it it was already stopped.
  template <class AnimatedEntity>
  void stop(const std::shared_ptr<AnimatedEntity> &animated_entity);
};

template <class AnimatedEntity>
void CustomAnimator::start(std::shared_ptr<AnimatedEntity> animated_entity,
                     CustomMotionParams animation_parameters) {
  stop(animated_entity);
  auto aei =
      std::dynamic_pointer_cast<AnimatedEntityInterface>(animated_entity);
  aei->get_animation_id() = animation_parameters.animation_id;
  aei->get_animation_time_seconds() = 0.0;
  animated_entities[aei] = animation_parameters;
}

#endif // CUSTOM_ANIMATOR_H
