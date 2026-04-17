#version 410 core

// Per vertex data
layout(location = 0) in vec3 vertex_position;
layout(location = 2) in vec2 texture_coordinate;

out VertexOut {
    vec3 ws_position;
    vec2 texture_coordinate;
} vertex_out;

// Per instance data
uniform mat4 model_matrix;

// Global data
uniform mat4 projection_view_matrix;

void main() {
    vertex_out.ws_position = (model_matrix * vec4(vertex_position, 1.0f)).xyz;
    vertex_out.texture_coordinate = texture_coordinate;

    gl_Position = projection_view_matrix * vec4(vertex_out.ws_position, 1.0f);
}
