#ifndef SHADER_INTERFACE_H
#define SHADER_INTERFACE_H

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
#include <functional>

#include "glad/gl.h"

#include "utility/HelperTypes.h"

/// An interface for GLSL shaders with a bunch of helpers and things to make your life easier.
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

    [[nodiscard]] uint id() const;

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

#endif //SHADER_INTERFACE_H
