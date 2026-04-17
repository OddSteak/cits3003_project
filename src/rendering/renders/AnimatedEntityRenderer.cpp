#include "AnimatedEntityRenderer.h"

AnimatedEntityRenderer::AnimatedEntityShader::AnimatedEntityShader() :
    BaseLitEntityShader("Animated Entity", "animated_entity/vert.glsl", "animated_entity/frag.glsl", {{"BONE_TRANSFORMS", BONE_TRANSFORMS_STR}}) {

    get_uniforms_set_bindings();
}

void AnimatedEntityRenderer::AnimatedEntityShader::get_uniforms_set_bindings() {
    BaseLitEntityShader::get_uniforms_set_bindings(); // Call the base implementation to load all the common uniforms
    bone_transforms_location = get_uniform_location("bone_transforms");
}

void AnimatedEntityRenderer::AnimatedEntityShader::set_model_matrix(const glm::mat4& model_matrix) {
    glProgramUniformMatrix4fv(id(), model_matrix_location, 1, GL_FALSE, &model_matrix[0][0]);
}

void AnimatedEntityRenderer::AnimatedEntityShader::set_bone_transforms(const std::vector<glm::mat4>& bone_transforms) {
    glProgramUniformMatrix4fv(id(), bone_transforms_location, std::min(BONE_TRANSFORMS, (int) bone_transforms.size()), GL_FALSE, &bone_transforms[0][0][0]);
}

AnimatedEntityRenderer::AnimatedEntityRenderer::AnimatedEntityRenderer() : shader() {}

void AnimatedEntityRenderer::AnimatedEntityRenderer::render(const RenderScene& render_scene, const LightScene& light_scene) {
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

        entity->mesh_hierarchy->calculate_animation(entity->animation_id, entity->animation_time_seconds);
        entity->mesh_hierarchy->visit_nodes([this, &entity](const MeshHierarchyNode& node, glm::mat4 accumulated_transformation) {
            for (const auto& mesh_id: node.meshes) {
                const auto& mesh = entity->mesh_hierarchy->meshes[mesh_id];

                shader.set_model_matrix(entity->instance_data.model_matrix * accumulated_transformation);
                if (!mesh.bone_transforms.empty()) shader.set_bone_transforms(mesh.bone_transforms);

                glBindVertexArray(mesh.model->get_vao());
                glDrawElementsBaseVertex(GL_TRIANGLES, mesh.model->get_index_count(), GL_UNSIGNED_INT, nullptr, mesh.model->get_vertex_offset());
            }
        });
    }
}

bool AnimatedEntityRenderer::AnimatedEntityRenderer::refresh_shaders() {
    return shader.reload_files();
}

void AnimatedEntityRenderer::VertexData::from_mesh(const VertexCollection& vertex_collection, std::vector<VertexData>& out_vertices) {
    out_vertices.reserve(out_vertices.size() + vertex_collection.positions.size());

    if (vertex_collection.bones.empty() || vertex_collection.bones.size() != vertex_collection.positions.size()) {
        throw std::runtime_error("AnimatedEntityRenderer::VertexData requires bones");
    }
    if (vertex_collection.normals.empty() || vertex_collection.normals.size() != vertex_collection.positions.size()) {
        throw std::runtime_error("AnimatedEntityRenderer::VertexData requires normals");
    }

    if (vertex_collection.tex_coords.empty() || vertex_collection.tex_coords.size() != vertex_collection.positions.size()) {
//        throw std::runtime_error("AnimatedEntityRenderer::VertexData requires texture coordinates");
    }

    for (auto i = 0u; i < vertex_collection.positions.size(); i++) {
        out_vertices.push_back(VertexData{
            vertex_collection.positions[i],
            vertex_collection.normals[i],
            i < vertex_collection.tex_coords.size() ? vertex_collection.tex_coords[i] : glm::vec2{0.0f},
            vertex_collection.bones[i].first,
            vertex_collection.bones[i].second
        });
    }
}


void AnimatedEntityRenderer::VertexData::setup_attrib_pointers() {
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*) offsetof(VertexData, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*) offsetof(VertexData, normal));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*) offsetof(VertexData, texture_coordinate));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*) offsetof(VertexData, bone_weights));
    glVertexAttribIPointer(4, 4, GL_UNSIGNED_INT, sizeof(VertexData), (void*) offsetof(VertexData, bone_indices)); // Note the `I` in the function name, needed to have ints work as expected
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
}