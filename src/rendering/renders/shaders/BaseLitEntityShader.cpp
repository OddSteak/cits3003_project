#include "BaseLitEntityShader.h"

#include <utility>

BaseLitEntityShader::BaseLitEntityShader(std::string name, const std::string& vertex_path, const std::string& fragment_path,
                                         std::unordered_map<std::string, std::string> vert_defines,
                                         std::unordered_map<std::string, std::string> frag_defines) :
    BaseEntityShader(std::move(name), vertex_path, fragment_path, std::move(vert_defines), std::move(frag_defines)),
    point_lights_ubo({}, false) {

    get_uniforms_set_bindings();
}

void BaseLitEntityShader::get_uniforms_set_bindings() {
    BaseEntityShader::get_uniforms_set_bindings(); // Call the base implementation to load all the common uniforms
    // Material
    diffuse_tint_location = get_uniform_location("diffuse_tint");
    specular_tint_location = get_uniform_location("specular_tint");
    ambient_tint_location = get_uniform_location("ambient_tint");
    shininess_location = get_uniform_location("shininess");
    // Texture sampler bindings
    set_binding("diffuse_texture", 0);
    set_binding("specular_map_texture", 1);
    // Uniform block bindings
    set_block_binding("PointLightArray", POINT_LIGHT_BINDING);
}

void BaseLitEntityShader::set_instance_data(const BaseLitEntityInstanceData& instance_data) {
    BaseEntityShader::set_instance_data(instance_data);

    // Calculate and set material properties
    const auto& entity_material = instance_data.material;

    glm::vec3 scaled_diffuse_tint = glm::vec3(entity_material.diffuse_tint) * entity_material.diffuse_tint.a;
    glm::vec3 scaled_specular_tint = glm::vec3(entity_material.specular_tint) * entity_material.specular_tint.a;
    glm::vec3 scaled_ambient_tint = glm::vec3(entity_material.ambient_tint) * entity_material.ambient_tint.a;

    glProgramUniform3fv(id(), diffuse_tint_location, 1, &scaled_diffuse_tint[0]);
    glProgramUniform3fv(id(), specular_tint_location, 1, &scaled_specular_tint[0]);
    glProgramUniform3fv(id(), ambient_tint_location, 1, &scaled_ambient_tint[0]);
    glProgramUniform1fv(id(), shininess_location, 1, &entity_material.shininess);
}

void BaseLitEntityShader::set_point_lights(const std::vector<PointLight>& point_lights) {
    uint count = std::min(MAX_PL, (uint) point_lights.size());

    for (uint i = 0; i < count; i++) {
        const PointLight& point_light = point_lights[i];

        glm::vec3 scaled_colour = glm::vec3(point_light.colour) * point_light.colour.a;

        point_lights_ubo.data[i].position = point_light.position;
        point_lights_ubo.data[i].colour = scaled_colour;
    }

    set_vert_define("NUM_PL", Formatter() << count);
    point_lights_ubo.bind(POINT_LIGHT_BINDING);
    point_lights_ubo.upload();
}