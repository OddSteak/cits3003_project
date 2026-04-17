#include "BaseEntityShader.h"

#include <utility>

BaseEntityShader::BaseEntityShader(std::string name, const std::string& vertex_path, const std::string& fragment_path,
                                   std::unordered_map<std::string, std::string> vert_defines,
                                   std::unordered_map<std::string, std::string> frag_defines) :
    ShaderInterface(std::move(name), vertex_path, fragment_path, [&]() { get_uniforms_set_bindings(); }, std::move(vert_defines), std::move(frag_defines)) {

    get_uniforms_set_bindings();
}

void BaseEntityShader::get_uniforms_set_bindings() {
    // Acquire and store the uniform locations now
    model_matrix_location = get_uniform_location("model_matrix");
    projection_view_matrix_location = get_uniform_location("projection_view_matrix");
    // Global
    ws_view_position_location = get_uniform_location("ws_view_position");
    inverse_gamma_location = get_uniform_location("inverse_gamma");
}

void BaseEntityShader::set_instance_data(const BaseEntityInstanceData& instance_data) {
    // Set model matrix
    glProgramUniformMatrix4fv(id(), model_matrix_location, 1, GL_FALSE, &instance_data.model_matrix[0][0]);
}

void BaseEntityShader::set_global_data(const BaseEntityGlobalData& global_data) {
    glProgramUniformMatrix4fv(id(), projection_view_matrix_location, 1, GL_FALSE, &global_data.projection_view_matrix[0][0]);
    glProgramUniform3fv(id(), ws_view_position_location, 1, &global_data.camera_position[0]);
    glProgramUniform1f(id(), inverse_gamma_location, 1.0f / global_data.gamma);
}