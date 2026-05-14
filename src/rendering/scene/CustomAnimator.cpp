#include "CustomAnimator.h"

#include <glm/gtc/type_ptr.hpp>

void CustomAnimator::animate(double dt) {
    for (auto& [entity, entry] : animated_entities) {
        if (entry.paused) continue;
        *entry.position_ptr += entry.parameters.translation_velocity * (float)dt;
        // rotation_velocity is degrees/second; euler_rotation is stored in radians
        *entry.rotation_ptr += glm::radians(entry.parameters.rotation_velocity) * (float)dt;
        entry.update_fn();
    }
}

void CustomAnimator::stop_all() {
    for (auto& [entity, entry] : animated_entities) {
        *entry.position_ptr = entry.initial_position;
        *entry.rotation_ptr = entry.initial_rotation;
        entry.update_fn();
    }
    animated_entities.clear();
}
