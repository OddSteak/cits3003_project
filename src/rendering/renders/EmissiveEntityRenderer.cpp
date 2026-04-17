#include "EmissiveEntityRenderer.h"

EmissiveEntityRenderer::EmissiveEntityShader::EmissiveEntityShader() :
    BaseEntityShader("Emissive Entity", "emissive_entity/vert.glsl", "emissive_entity/frag.glsl") {
    get_uniforms_set_bindings();
}

void EmissiveEntityRenderer::EmissiveEntityShader::get_uniforms_set_bindings() {
    // Material
    emission_tint_location = get_uniform_location("emissive_tint");
    // Texture sampler bindings
    set_binding("emissive_texture", 0);
}

void EmissiveEntityRenderer::EmissiveEntityShader::set_instance_data(const InstanceData& instance_data) {
    BaseEntityShader::set_instance_data(instance_data); // Call the base implementation to load all the common uniforms

    const auto& entity_material = instance_data.material;

    glm::vec3 scaled_diffuse_tint = glm::vec3(entity_material.emission_tint) * entity_material.emission_tint.a;

    glProgramUniform3fv(id(), emission_tint_location, 1, &scaled_diffuse_tint[0]);
}

EmissiveEntityRenderer::EmissiveEntityRenderer::EmissiveEntityRenderer() : shader() {}

void EmissiveEntityRenderer::EmissiveEntityRenderer::render(const RenderScene& render_scene) {
    shader.use();
    shader.set_global_data(render_scene.global_data);

    for (const auto& entity: render_scene.entities) {
        shader.set_instance_data(entity->instance_data);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, entity->render_data.emission_texture->get_texture_id());

        glBindVertexArray(entity->model->get_vao());
        glDrawElementsBaseVertex(GL_TRIANGLES, entity->model->get_index_count(), GL_UNSIGNED_INT, nullptr, entity->model->get_vertex_offset());
    }
}

bool EmissiveEntityRenderer::EmissiveEntityRenderer::refresh_shaders() {
    return shader.reload_files();
}
