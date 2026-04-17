#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

using namespace nlohmann;

/// Some helpers to make saving and loading glm vectors to and from json
namespace glm {
    template<typename T, qualifier Q = defaultp>
    void to_json(json& j, const vec<1, T, Q>& v) {
        j = json::array({v.x});
    }

    template<typename T, qualifier Q = defaultp>
    void to_json(json& j, const vec<2, T, Q>& v) {
        j = json::array({v.x, v.y});
    }

    template<typename T, qualifier Q = defaultp>
    void to_json(json& j, const vec<3, T, Q>& v) {
        j = json::array({v.x, v.y, v.z});
    }

    template<typename T, qualifier Q = defaultp>
    void to_json(json& j, const vec<4, T, Q>& v) {
        j = json::array({v.x, v.y, v.z, v.w});
    }

    template<typename T, qualifier Q = defaultp>
    void from_json(const json& j, vec<1, T, Q>& v) {
        v.x = j[0];
    }

    template<typename T, qualifier Q = defaultp>
    void from_json(const json& j, vec<2, T, Q>& v) {
        v.x = j[0];
        v.y = j[1];
    }

    template<typename T, qualifier Q = defaultp>
    void from_json(const json& j, vec<3, T, Q>& v) {
        v.x = j[0];
        v.y = j[1];
        v.z = j[2];
    }

    template<typename T, qualifier Q = defaultp>
    void from_json(const json& j, vec<4, T, Q>& v) {
        v.x = j[0];
        v.y = j[1];
        v.z = j[2];
        v.w = j[3];
    }
}


#endif //JSON_HELPER_H
