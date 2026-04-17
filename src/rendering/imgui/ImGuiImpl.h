#ifndef IMGUI_IMPL_H
#define IMGUI_IMPL_H

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE

#include <imgui/imgui.h>

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM (<glad/gl.h>)

#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>

#endif //IMGUI_IMPL_H
