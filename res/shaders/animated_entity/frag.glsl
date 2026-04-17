#version 410 core
#include "../common/lights.glsl"

in VertexOut {
    LightingResult lighting_result;
    vec2 texture_coordinate;
} frag_in;

layout(location = 0) out vec4 out_colour;

// Global Data
uniform float inverse_gamma;

uniform sampler2D diffuse_texture;
uniform sampler2D specular_map_texture;

void main() {
    // Resolve the per vertex lighting with per fragment texture sampling.
    vec3 resolved_lighting = resolve_textured_light_calculation(frag_in.lighting_result, diffuse_texture, specular_map_texture, frag_in.texture_coordinate);

    out_colour = vec4(resolved_lighting, 1.0f);
    out_colour.rgb = pow(out_colour.rgb, vec3(inverse_gamma));
}
