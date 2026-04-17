#include "MeshHierarchy.h"

glm::mat4 AnimationData::sample(double time) const {
    glm::vec3 position{0.0f};
    if (!positions.empty()) {
        auto next_key = positions.lower_bound(time);
        if (next_key == positions.end()) {
            position = positions.rbegin()->second;
        } else if (next_key->first == time || next_key == positions.begin()) {
            position = next_key->second;
        } else {
            auto next = *next_key;
            auto prev = *(--next_key);

            position = glm::mix(prev.second, next.second, (float) ((time - prev.first) / (next.first - prev.first)));
        }
    }

    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    if (!rotations.empty()) {
        auto next_key = rotations.lower_bound(time);
        if (next_key == rotations.end()) {
            rotation = rotations.rbegin()->second;
        } else if (next_key->first == time || next_key == rotations.begin()) {
            rotation = next_key->second;
        } else {
            auto next = *next_key;
            auto prev = *(--next_key);

            rotation = glm::slerp(prev.second, next.second, (float) ((time - prev.first) / (next.first - prev.first)));
        }
    }

    glm::vec3 scaling{1.0f};
    if (!scalings.empty()) {
        auto next_key = scalings.lower_bound(time);
        if (next_key == scalings.end()) {
            scaling = scalings.rbegin()->second;
        } else if (next_key->first == time || next_key == scalings.begin()) {
            scaling = next_key->second;
        } else {
            auto next = *next_key;
            auto prev = *(--next_key);

            scaling = glm::mix(prev.second, next.second, (float) ((time - prev.first) / (next.first - prev.first)));
        }
    }

    return glm::translate(position) * glm::toMat4(rotation) * glm::scale(scaling);
}