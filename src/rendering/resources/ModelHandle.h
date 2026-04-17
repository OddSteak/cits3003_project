#ifndef MODEL_HANDLE_H
#define MODEL_HANDLE_H

#include <string>
#include <optional>

#include <glad/gl.h>
#include "utility/HelperTypes.h"

/// A type-erased version of ModelHandle for polymorphic usages
class BaseModelHandle : private NonCopyable {
public:
    virtual ~BaseModelHandle() = default;
};

/// A class representing a handle to a loaded model, also storing some of its configuration data.
template<typename VertexData>
class ModelHandle : public BaseModelHandle {
    uint vertex_vbo;
    uint index_vbo;
    uint vao;
    int index_count;
    int vertex_offset;

    std::optional<std::string> filename{};
public:
    ModelHandle(uint vertex_vbo, uint index_vbo, uint vao, int index_count, int vertex_offset, std::optional<std::string> filename = {});

    [[nodiscard]] uint get_vertex_vbo() const;
    [[nodiscard]] uint get_index_vbo() const;
    [[nodiscard]] uint get_vao() const;
    [[nodiscard]] int get_index_count() const;
    [[nodiscard]] int get_vertex_offset() const;
    [[nodiscard]] const std::optional<std::string>& get_filename() const;

    ~ModelHandle() override;
};

template<typename VertexData>
ModelHandle<VertexData>::ModelHandle(uint vertex_vbo, uint index_vbo, uint vao, int index_count, int vertex_offset, std::optional<std::string> filename)
    : BaseModelHandle(), vertex_vbo(vertex_vbo), index_vbo(index_vbo), vao(vao), index_count(index_count), vertex_offset(vertex_offset), filename(std::move(filename)) {}

template<typename VertexData>
uint ModelHandle<VertexData>::get_vertex_vbo() const {
    return vertex_vbo;
}

template<typename VertexData>
uint ModelHandle<VertexData>::get_index_vbo() const {
    return index_vbo;
}

template<typename VertexData>
uint ModelHandle<VertexData>::get_vao() const {
    return vao;
}

template<typename VertexData>
int ModelHandle<VertexData>::get_index_count() const {
    return index_count;
}

template<typename VertexData>
int ModelHandle<VertexData>::get_vertex_offset() const {
    return vertex_offset;
}

template<typename VertexData>
const std::optional<std::string>& ModelHandle<VertexData>::get_filename() const {
    return filename;
}

template<typename VertexData>
ModelHandle<VertexData>::~ModelHandle() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertex_vbo);
    glDeleteBuffers(1, &index_vbo);
}

#endif //MODEL_HANDLE_H
