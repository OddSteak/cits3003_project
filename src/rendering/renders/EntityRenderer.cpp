#include "EntityRenderer.h"

EntityRenderer::EntityShader::EntityShader() :
    BaseLitEntityShader("Entity", "entity/vert.glsl", "entity/frag.glsl") {

    get_uniforms_set_bindings();
}

void EntityRenderer::EntityShader::get_uniforms_set_bindings() {
    BaseLitEntityShader::get_uniforms_set_bindings(); // Call the base implementation to load all the common uniforms
    // Pass normal matrix from cpu to not need to compute the same cofactor for every vertex
    normal_matrix_location = get_uniform_location("normal_matrix");
}

void EntityRenderer::EntityShader::set_instance_data(const BaseLitEntityInstanceData& instance_data) {
    BaseLitEntityShader::set_instance_data(instance_data); // Call the base implementation to set all the common instance data

    // Calculate a normal matrix so that non-uniform scale transformations properly transform normals
    // See: https://github.com/graphitemaster/normals_revisited
    // and: https://gist.github.com/shakesoda/8485880f71010b79bc8fed0f166dabac
    glm::mat3 normal_matrix = glm::mat3(
        glm::cross(glm::vec3(instance_data.model_matrix[1]), glm::vec3(instance_data.model_matrix[2])),
        glm::cross(glm::vec3(instance_data.model_matrix[2]), glm::vec3(instance_data.model_matrix[0])),
        glm::cross(glm::vec3(instance_data.model_matrix[0]), glm::vec3(instance_data.model_matrix[1]))
    );
    glProgramUniformMatrix3fv(id(), normal_matrix_location, 1, GL_FALSE, &normal_matrix[0][0]);
}

EntityRenderer::EntityRenderer::EntityRenderer() : shader() {}

void EntityRenderer::EntityRenderer::render(const RenderScene& render_scene, const LightScene& light_scene) {
    shader.use();
    shader.set_global_data(render_scene.global_data);

    for (const auto& entity: render_scene.entities) {
        shader.set_instance_data(entity->instance_data);

        glm::vec3 position = entity->instance_data.model_matrix[3];
        // IMPORTANT NOTE:
        // This call has the potential to recompile the shader if the value for "NUM_PL" changes.
        // If this where to happen for every entity, it would MASSIVELY kill performance (and possibly just not even work at all).
        // However, in this case, consecutive get_nearest_point_lights calls WILL return the same number of items,
        // so that issue won't happen since it only recompiles on a change.
        // Just make sure to be careful of this kind of thing.
        shader.set_point_lights(light_scene.get_nearest_point_lights(position, BaseLitEntityShader::MAX_PL, 1));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, entity->render_data.diffuse_texture->get_texture_id());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, entity->render_data.specular_map_texture->get_texture_id());

        glBindVertexArray(entity->model->get_vao());
        glDrawElementsBaseVertex(GL_TRIANGLES, entity->model->get_index_count(), GL_UNSIGNED_INT, nullptr, entity->model->get_vertex_offset());
    }
}

bool EntityRenderer::EntityRenderer::refresh_shaders() {
    return shader.reload_files();
}

void EntityRenderer::VertexData::from_mesh(const VertexCollection& vertex_collection, std::vector<VertexData>& out_vertices) {
    out_vertices.reserve(out_vertices.size() + vertex_collection.positions.size());

    if (vertex_collection.normals.empty() || vertex_collection.normals.size() != vertex_collection.positions.size()) {
        throw std::runtime_error("EntityRenderer::VertexData requires normals");
    }

    if (vertex_collection.tex_coords.empty() || vertex_collection.tex_coords.size() != vertex_collection.positions.size()) {
        throw std::runtime_error("EntityRenderer::VertexData requires texture coordinates");
    }

    for (auto i = 0u; i < vertex_collection.positions.size(); i++) {
        out_vertices.push_back(VertexData{
            vertex_collection.positions[i],
            vertex_collection.normals[i],
            vertex_collection.tex_coords[i]
        });
    }
}


void EntityRenderer::VertexData::setup_attrib_pointers() {
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*) offsetof(VertexData, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*) offsetof(VertexData, normal));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*) offsetof(VertexData, texture_coordinate));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}