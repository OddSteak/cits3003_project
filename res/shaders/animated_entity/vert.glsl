#version 410 core
#include "../common/lights.glsl"
#include "../common/maths.glsl"

// Per vertex data
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coordinate;
layout(location = 3) in vec4 bone_weights;
layout(location = 4) in uvec4 bone_indices;

out VertexOut {
    LightingResult lighting_result;
    vec2 texture_coordinate;
} vertex_out;

// Per instance data
uniform mat4 model_matrix;

// Material properties
uniform vec3 diffuse_tint;
uniform vec3 specular_tint;
uniform vec3 ambient_tint;
uniform float shininess;

// Light Data
#if NUM_PL > 0
layout (std140) uniform PointLightArray {
    PointLightData point_lights[NUM_PL];
};
#endif

// Animation Data
uniform mat4 bone_transforms[BONE_TRANSFORMS];

// Global data
uniform vec3 ws_view_position;
uniform mat4 projection_view_matrix;

uniform sampler2D specular_map_texture;

void main() {
    // Transform vertices
    float sum = dot(bone_weights, vec4(1.0f));

    mat4 bone_transform =
        bone_weights[0] * bone_transforms[bone_indices[0]]
        + bone_weights[1] * bone_transforms[bone_indices[1]]
        + bone_weights[2] * bone_transforms[bone_indices[2]]
        + bone_weights[3] * bone_transforms[bone_indices[3]]
        + (1.0f - sum) * mat4(1.0f);

    mat4 animation_matrix = model_matrix * bone_transform;
    mat3 normal_matrix = cofactor(animation_matrix);

    vec3 ws_position = (animation_matrix * vec4(vertex_position, 1.0f)).xyz;
    vec3 ws_normal = normalize(normal_matrix * normal);
    vertex_out.texture_coordinate = texture_coordinate;

    gl_Position = projection_view_matrix * vec4(ws_position, 1.0f);

    // Per vertex light calcs are below this point
    vec3 ws_view_dir = normalize(ws_view_position - ws_position);
    LightCalculatioData light_calculation_data = LightCalculatioData(ws_position, ws_view_dir, ws_normal);
    Material material = Material(diffuse_tint, specular_tint, ambient_tint, shininess);

    vertex_out.lighting_result = total_light_calculation(light_calculation_data, material
        #if NUM_PL > 0
        ,point_lights
        #endif
    );
}
