#ifndef SHADER_INTERFACE_H
#define SHADER_INTERFACE_H

#include <string>
#include <optional>
#include <filesystem>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
#include <functional>

#include <glad/gl.h>

#include "utility/HelperTypes.h"

/// An interface for GLSL shaders with a bunch of helpers and things to make your life easier.
template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
class ShaderInterface {
    const std::string SHADER_DIR = "res/shaders";

    uint program_id;

    std::unordered_map<std::string, int> uniform_locations;
    std::unordered_map<std::string, int> uniform_block_indices;

    std::string shader_name;
    std::string vertex_code;
    std::string fragment_code;

    std::string vertex_path;
    std::string fragment_path;
    std::function<void()> setup;

    std::unordered_map<std::string, std::string> vert_defines;
    std::unordered_map<std::string, std::string> frag_defines;
public:
    /// Construct the interface, proving the name of shaders (used for error formatting), the paths to the vertex
    /// and fragment shaders, also a setup function which is called initially and when the shader is reloaded from disk (hot loaded).
    /// Also can specify some #define K V, that will be applied to the shaders
    ShaderInterface(std::string name, const std::string& vertex_path, const std::string& fragment_path,
                    std::function<void()> setup,
                    std::unordered_map<std::string, std::string> vert_defines = {},
                    std::unordered_map<std::string, std::string> frag_defines = {});

    uint id() const;

    void use() const;

    /// Fetch the newest version of the shaders from disk and try to compile them, prints errors if but keeps working if
    /// the is an issue with the new shaders.
    bool reload_files();

    /// Recompile the shaders using the stored shader code, but with new defines.
    void recompile(std::unordered_map<std::string, std::string> new_vert_defines = {},
                   std::unordered_map<std::string, std::string> new_frag_defines = {});

    /// Set an individual vert define, and by default recompile.
    void set_vert_define(std::string key, std::string value, bool defer_recompile = false);
    /// Set an individual frag define, and by default recompile.
    void set_frag_define(std::string key, std::string value, bool defer_recompile = false);

    /// Free up resources.
    void cleanup();

    virtual ~ShaderInterface();
private:
    static std::optional<std::string> load_shader_file(const std::string& shader_path);

    static std::optional<std::string> apply_defines_and_includes(const std::string& code, const std::string& shader_path, const std::unordered_map<std::string, std::string>& defines);
    static std::optional<std::string> apply_includes(const std::string& code, const std::string& shader_path);

    static std::optional<uint> compile_shader_code(const std::string& shader_code, uint shader_type, const std::string& shader_name);

    static std::optional<uint> link_program(uint vertex_shader, uint fragment_shader, const std::string& shader_name);

protected:
    [[nodiscard]] int get_uniform_location(const std::string& name);
    [[nodiscard]] uint get_uniform_block_index(const std::string& name);

    void set_binding(const std::string& sampler_name, uint binding);
    void set_block_binding(const std::string& block_name, uint binding);
};


template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::ShaderInterface(std::string name, const std::string& vertex_path,
                                                                                   const std::string& fragment_path,
                                                                                   std::function<void()> setup,
                                                                                   std::unordered_map<std::string, std::string> vert_defines,
                                                                                   std::unordered_map<std::string, std::string> frag_defines)
    : uniform_locations(), uniform_block_indices(), shader_name(std::move(name)), vertex_path(vertex_path), fragment_path(fragment_path), setup(std::move(setup)), vert_defines(std::move(vert_defines)), frag_defines(std::move(frag_defines)) {

    vertex_code = load_shader_file(SHADER_DIR + "/" + vertex_path).value(); // Will throw exception on failure
    fragment_code = load_shader_file(SHADER_DIR + "/" + fragment_path).value(); // Will throw exception on failure

    std::string realisedVertexCode = apply_defines_and_includes(vertex_code, SHADER_DIR + "/" + vertex_path, this->vert_defines).value(); // Will throw exception on failure
    std::string realisedFragmentCode = apply_defines_and_includes(fragment_code, SHADER_DIR + "/" + fragment_path, this->frag_defines).value(); // Will throw exception on failure

    auto vertexShader = compile_shader_code(realisedVertexCode, GL_VERTEX_SHADER, shader_name).value(); // Will throw exception on failure
    auto fragmentShader = compile_shader_code(realisedFragmentCode, GL_FRAGMENT_SHADER, shader_name).value(); // Will throw exception on failure

    program_id = link_program(vertexShader, fragmentShader, shader_name).value(); // Will throw exception on failure

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
uint ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::id() const {
    return program_id;
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
void ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::use() const {
    glUseProgram(program_id);
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
bool ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::reload_files() {
    auto old_vertex_code = vertex_code;
    auto old_fragment_code = fragment_code;

    try {
        vertex_code = load_shader_file(SHADER_DIR + "/" + vertex_path).value(); // Will throw exception on failure
        fragment_code = load_shader_file(SHADER_DIR + "/" + fragment_path).value(); // Will throw exception on failure

        recompile(std::move(vert_defines), std::move(frag_defines));
        std::cout << "Successfully reloaded shader files for: [" << shader_name << "]" << std::endl;
        return true;
    } catch (const std::bad_optional_access&) {
        vertex_code = std::move(old_vertex_code);
        fragment_code = std::move(old_fragment_code);
        recompile(std::move(vert_defines), std::move(frag_defines));
        std::cerr << "Failed to reload shader files for: [" << shader_name << "]" << std::endl;
        return false;
    }
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
void ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::recompile(
    std::unordered_map<std::string, std::string> new_vert_defines,
    std::unordered_map<std::string, std::string> new_frag_defines) {
    cleanup();

    vert_defines = std::move(new_vert_defines);
    frag_defines = std::move(new_frag_defines);

    std::string realised_vertex_code = apply_defines_and_includes(vertex_code, SHADER_DIR + "/" + vertex_path, vert_defines).value(); // Will throw exception on failure;
    std::string realised_fragment_code = apply_defines_and_includes(fragment_code, SHADER_DIR + "/" + fragment_path, frag_defines).value(); // Will throw exception on failure;

    uint vertex_shader = compile_shader_code(realised_vertex_code, GL_VERTEX_SHADER, shader_name).value(); // Will throw exception on failure
    uint fragment_shader = compile_shader_code(realised_fragment_code, GL_FRAGMENT_SHADER, shader_name).value(); // Will throw exception on failure

    program_id = link_program(vertex_shader, fragment_shader, shader_name).value(); // Will throw exception on failure

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    uniform_locations.clear();
    uniform_block_indices.clear();

    this->setup();
    this->use();
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
void ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::set_vert_define(std::string key, std::string value, bool defer_recompile) {
    auto existing = vert_defines.find(key);
    if (existing != vert_defines.end() && existing->second == value) return; // No work to do

    vert_defines[std::move(key)] = std::move(value);
    if (!defer_recompile) {
        recompile(vert_defines, frag_defines);
    }
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
void ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::set_frag_define(std::string key, std::string value, bool defer_recompile) {
    auto existing = frag_defines.find(key);
    if (existing != frag_defines.end() && existing->second == value) return; // No work to do

    frag_defines[std::move(key)] = std::move(value);
    if (!defer_recompile) {
        recompile(vert_defines, frag_defines);
    }
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
std::optional<std::string> ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::load_shader_file(const std::string& shader_path) {
    std::string shader_code;
    std::ifstream shader_file;

    shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        shader_file.open(shader_path);
        std::stringstream shader_stream;

        shader_stream << shader_file.rdbuf();

        shader_file.close();

        shader_code = shader_stream.str();
    } catch (std::ifstream::failure&) {
        std::cout << "Failed to load shader file: " << shader_path << std::endl;
        return {};
    }

    return shader_code;
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
std::optional<std::string> ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::apply_defines_and_includes(const std::string& code,
                                                                                                                         const std::string& shader_path,
                                                                                                                         const std::unordered_map<std::string, std::string>& defines) {
    std::stringstream output;

    auto version_start = code.find("#version", 0);
    if (version_start == std::string::npos) {
        std::cerr << "GLSL shader must specify version using #version" << std::endl;
        return {};
    }
    auto version_line_end = code.find('\n', version_start);

    output << code.substr(0, version_line_end + 1);

    for (const auto& def: defines) {
        output << "#define " << def.first << " " << def.second << "\n";
    }

    output << code.substr(version_line_end + 1);

    return apply_includes(output.str(), shader_path);
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
std::optional<std::string> ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::apply_includes(const std::string& code, const std::string& shader_path) {
    std::string shader_path_base = std::filesystem::path(shader_path).parent_path().string();
    shader_path_base += "/";

    std::istringstream input(code);
    std::stringstream output;

    bool include_happened = false;
    for (std::string line; std::getline(input, line);) {
        auto inc = line.find("#include");
        if (inc != std::string::npos) {
            auto start = line.find('"', inc);
            auto end = line.rfind('"');
            if (start != std::string::npos && end != std::string::npos && start != end) {
                auto path = line.substr(start + 1, end - start - 1);

                auto included_code = load_shader_file(shader_path_base + path);
                if (!included_code.has_value()) return {};

                include_happened = true;

                output << included_code.value() << '\n';
                continue;
            }
        }

        output << line << '\n';
    }

    if (include_happened) {
        return apply_includes(output.str(), shader_path);
    }

    return output.str();
}

static inline std::string format_info_log(const std::string& shader_code, const std::string& info_log) {
    std::vector<std::string> lines;
    std::istringstream code_line_reader(shader_code);
    for (std::string line; std::getline(code_line_reader, line);) {
        lines.push_back(line);
    }

    std::stringstream formatted_info_log;
    std::istringstream info_log_reader(info_log);
    for (std::string line; std::getline(info_log_reader, line);) {
        formatted_info_log << line << '\n';
        if (line.substr(0, 2) == "0(") {
            auto end = line.find(')');
            if (end != std::string::npos) {
                auto num = line.substr(2, end - 2);
                try {
                    auto i = std::stoi(num) - 1;
                    if (i >= 0 && i < (int) lines.size()) {
                        if (i - 1 >= 0) {
                            formatted_info_log << "\t[ ]: " << lines[i - 1] << '\n';
                        }
                        formatted_info_log << "\t[>]: " << lines[i] << '\n';
                        if (i + 1 < (int) lines.size()) {
                            formatted_info_log << "\t[ ]: " << lines[i + 1] << '\n';
                        }
                    }
                } catch (const std::exception&) {}
            }
        }
    }

    return formatted_info_log.str();
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
std::optional<uint>
ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::compile_shader_code(const std::string& shader_code,
                                                                                       uint shader_type, const std::string& shader_name) {
    uint shader = glCreateShader(shader_type);

    const char* source_c_str = shader_code.c_str();
    glShaderSource(shader, 1, &source_c_str, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        int msg_len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &msg_len);
        std::string info_log;
        info_log.resize(msg_len - 1);

        glGetShaderInfoLog(shader, msg_len, nullptr, info_log.data());
        std::cerr << "Failed to compile '" << shader_name << "' " << (shader_type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << " shader\n"
                  << format_info_log(shader_code, info_log) << std::endl;
        return {};
    }

    return shader;
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
std::optional<uint> ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::link_program(uint vertex_shader, uint fragment_shader, const std::string& shader_name) {
    uint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // print linking errors if any
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        int msg_len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &msg_len);
        std::string info_log;
        info_log.resize(msg_len - 1);


        glGetProgramInfoLog(program, msg_len, nullptr, info_log.data());
        // TODO: To improved error logging: Need to try figure out if the info_log is referencing vertex or fragment shader (or both) and use corresponding shader_code(s)
        std::cerr << "Failed to link shader program '" << shader_name << "'" << "\n" << format_info_log("", info_log) << std::endl;
        return {};
    }

    return program;
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
int ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::get_uniform_location(const std::string& name) {
    auto search = uniform_locations.find(name);

    if (search != uniform_locations.end()) {
        return search->second;
    } else {
        int location = glGetUniformLocation(program_id, name.c_str());

        uniform_locations.insert({name, location});

        return location;
    }
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
uint ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::get_uniform_block_index(const std::string& name) {
    auto search = uniform_block_indices.find(name);

    if (search != uniform_block_indices.end()) {
        return search->second;
    } else {
        int location = glGetUniformBlockIndex(program_id, name.c_str());

        uniform_block_indices.insert({name, location});

        return location;
    }
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
void ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::set_binding(const std::string& sampler_name, uint binding) {
    glProgramUniform1i(id(), get_uniform_location(sampler_name), binding);
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
void ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::set_block_binding(const std::string& block_name, uint binding) {
    uint index = get_uniform_block_index(block_name);
    // TODO: Print a warning? Might be wanted, but also might silence an issue
    if (index == GL_INVALID_INDEX) return; // Block name doesn't exist, just ignore setting anything
    glUniformBlockBinding(id(), index, binding);
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
void ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::cleanup() {
    if (program_id != GL_INVALID_INDEX) {
        glDeleteProgram(program_id);
        program_id = GL_INVALID_INDEX;
    }
}

template<typename VertexData, typename InstanceData, typename GlobalData, typename RenderData>
ShaderInterface<VertexData, InstanceData, GlobalData, RenderData>::~ShaderInterface() {
    cleanup();
}

#endif //SHADER_INTERFACE_H
