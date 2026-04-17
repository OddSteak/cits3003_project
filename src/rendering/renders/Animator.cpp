#include "Animator.h"

void Animator::animate(double dt) {
    std::vector<std::shared_ptr<AnimatedEntityInterface>> to_remove{};

    for (auto& item: animated_entities) {
        if (item.second.paused) continue;

        auto& time = item.first->get_animation_time_seconds();
        auto dur = item.first->get_animation_duration_seconds();
        time += dt * item.second.speed;
        if (time > dur) {
            if (item.second.loop && dur > 0) {
                time = std::fmod(time, dur);
            } else {
                time = dur;
                to_remove.push_back(item.first);
            }
        }
    }

    for (const auto& item: to_remove) {
        animated_entities.erase(item);
    }
}
