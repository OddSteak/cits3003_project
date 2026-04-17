#ifndef UNIFORM_BUFFER_ARRAY_H
#define UNIFORM_BUFFER_ARRAY_H

#include <array>
#include <glad/gl.h>

#include "utility/HelperTypes.h"

/// A helper class that abstracts over a Uniform Buffer Object as a type safe array of fixed size.
template<typename T, unsigned int N>
class UniformBufferArray : NonCopyable {
    uint ubo = 0;
public:
    /// The CPU side buffer that will be mirror on the GPU
    std::array<T, N> data;

    /// Construct the UBO with an initial state.
    /// is_state means that you do not intend to update it often
    explicit UniformBufferArray(std::array<T, N> data, bool is_static = true);
    /// Upload the CPU side to the GPU, if a valid index is provided then it will only upload that one element
    void upload(int index = -1);
    /// Bind the UBO to the specified binding index
    void bind(int binding);

    ~UniformBufferArray();
};

template<typename T, unsigned int N>
UniformBufferArray<T, N>::UniformBufferArray(std::array<T, N> data, bool is_static): data(data) {
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, N * sizeof(T), data.data(), is_static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

template<typename T, unsigned int N>
void UniformBufferArray<T, N>::upload(int index) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    if (index < 0 || index >= (int) N) {
        glBufferSubData(GL_UNIFORM_BUFFER, 0, N * sizeof(T), data.data());
    } else {
        glBufferSubData(GL_UNIFORM_BUFFER, index * sizeof(T), sizeof(T), &data[index]);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

template<typename T, unsigned int N>
void UniformBufferArray<T, N>::bind(int binding) {
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo);
}

template<typename T, unsigned int N>
UniformBufferArray<T, N>::~UniformBufferArray() {
    glDeleteBuffers(1, &ubo);
}

#endif //UNIFORM_BUFFER_ARRAY_H
