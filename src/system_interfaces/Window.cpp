#include "Window.h"
#include "rendering/imgui/ImGuiManager.h"

void Window::make_context_current() {
    glfwMakeContextCurrent(window);
}

void Window::swap_buffers() {
    glfwSwapBuffers(window);
}

bool Window::should_close() const {
    return glfwWindowShouldClose(window);
}

void Window::set_should_close() const {
    glfwSetWindowShouldClose(window, GL_TRUE);
}

bool Window::is_focused() const {
    return glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;
}

void Window::focus() {
    glfwFocusWindow(window);
}

glm::dvec2 Window::get_mouse_delta(int button) const {
    return reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window))->motion_deltas[button + 1];
}

glm::dvec2 Window::get_mouse_pos() const {
    glm::dvec2 position{};
    glfwGetCursorPos(window, &position.x, &position.y);
    return position;
}

glm::vec2 Window::get_mouse_pos_ndc() const {
    glm::dvec2 screenPos = get_mouse_pos();

    glm::vec2 NDC = {
        ((float) screenPos.x / (float) get_window_width()) * 2.0f - 1.0f,
        -(((float) screenPos.y / (float) get_window_height()) * 2.0f - 1.0f),
    };

    return NDC;
}


float Window::get_scroll_delta() const {
    return reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window))->scroll_delta;
}

bool Window::is_key_pressed(int key) const {
    return !ImGuiManager::want_capture_keyboard() && glfwGetKey(window, key) == GLFW_PRESS;
}

bool Window::was_key_pressed(int key) const {
    return reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window))->pressed_keys[key];
}

bool Window::is_mouse_pressed(int button) const {
    return !ImGuiManager::want_capture_mouse() && glfwGetMouseButton(window, button) == GLFW_PRESS;
}

void Window::set_cursor_disabled(bool disabled) const {
    if (disabled) {
        ImGuiManager::set_cursor_was_disabled();
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

glm::ivec2 Window::get_window_size() const {
    glm::ivec2 size;
    glfwGetWindowSize(window, &size.x, &size.y);
    return size;
}

uint32_t Window::get_window_width() const {
    return get_window_size().x;
}

uint32_t Window::get_window_height() const {
    return get_window_size().y;
}

glm::ivec2 Window::get_framebuffer_size() const {
    glm::ivec2 size;
    glfwGetFramebufferSize(window, &size.x, &size.y);
    return size;
}

float Window::get_framebuffer_aspect_ratio() const {
    return (float) get_framebuffer_width() / (float) get_framebuffer_height();
}

uint32_t Window::get_framebuffer_width() const {
    return get_framebuffer_size().x;
}

uint32_t Window::get_framebuffer_height() const {
    return get_framebuffer_size().y;
}

void Window::set_title_suffix(std::optional<std::string> suffix) {
    std::string title = base_title;
    if (suffix.has_value()) {
        title += " - ";
        title += suffix.value();
    }
    glfwSetWindowTitle(window, title.c_str());
}

GLFWwindow* Window::internal() const {
    return window;
}

bool Window::operator==(const Window& rhs) const {
    return window == rhs.window;
}

bool Window::operator!=(const Window& rhs) const {
    return !(rhs == *this);
}

bool Window::operator<(const Window& rhs) const {
    return window < rhs.window;
}

bool Window::operator>(const Window& rhs) const {
    return rhs < *this;
}

bool Window::operator<=(const Window& rhs) const {
    return !(rhs < *this);
}

bool Window::operator>=(const Window& rhs) const {
    return !(*this < rhs);
}
