#include "CustomAnimator.h"

#include <glm/gtc/type_ptr.hpp>

void CustomAnimator::animate(double dt) {
    for (auto& [entity, entry] : animated_entities) {
        if (entry.paused) continue;

        if (entry.parameters.motion_type == CustomMotionType::Move) {
            if (!entry.reached_final) {
                glm::vec3 to_final = entry.final_position - *entry.position_ptr;
                // Stop when we've reached or overshot the final position
                if (glm::dot(to_final, entry.translation_velocity) <= 0.0f) {
                    *entry.position_ptr = entry.final_position;
                    entry.reached_final = true;
                } else {
                    *entry.position_ptr += entry.translation_velocity * (float)dt;
                }
            }
        } else if (entry.parameters.motion_type == CustomMotionType::Rotate) {
            // rotation_velocity is degrees/second; euler_rotation is stored in radians
            *entry.rotation_ptr += glm::radians(entry.parameters.rotation_velocity) * (float)dt;
        }

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
