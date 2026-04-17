#include "WindowManager.h"

#include <memory.h>
#include <imgui/imgui.h>

#include "utility/OpenGL.h"


// These are defined in ImGuiManager.cpp, but ImGuiManager.h includes WindowManager.h,
// so this is done as a "dependency inversion".
extern bool want_capture_mouse();
extern bool want_capture_keyboard();

bool WindowManager::v_sync_enabled = false;

void glfwErrorCallback(int code, const char* msg) {
    std::cout << "GLFW Error (" << code << ")\n\t" << "msg: " << msg << std::endl;
}

void WindowManager::init() {
    glfwSetErrorCallback(glfwErrorCallback);
    glfwInit();
}

Window WindowManager::create_window(const std::string& name, glm::ivec2 size) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OpenGL::VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OpenGL::VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif // __APPLE__
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    auto glfwWindow = glfwCreateWindow(size.x, size.y, name.c_str(), nullptr, nullptr);

    glfwSetCursorPosCallback(glfwWindow, [](GLFWwindow* window, double x, double y) {
        static glm::dvec2 lastPos = {x, y};
        if (!want_capture_mouse()) {
            glm::dvec2 delta = glm::dvec2(x, y) - lastPos;

            auto windowData = reinterpret_cast<Window::WindowData*>(glfwGetWindowUserPointer(window));

            windowData->current_motion_deltas[0] += delta;

            for (size_t i = 1; i < Window::WindowData::DELTA_ARRAY_COUNT; ++i) {
                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1 + (int) i - 1) == GLFW_PRESS) {
                    windowData->current_motion_deltas[i] += delta;
                }
            }
        }
        lastPos = {x, y};
    });

    glfwSetKeyCallback(glfwWindow, [](GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
        auto windowData = reinterpret_cast<Window::WindowData*>(glfwGetWindowUserPointer(window));

        if (!want_capture_keyboard()) {
            if (action == GLFW_PRESS) {
                windowData->current_pressed_keys[key] = true;
            }
        }
    });

    glfwSetScrollCallback(glfwWindow, [](GLFWwindow* window, double /*dx*/, double dy) {
        auto windowData = reinterpret_cast<Window::WindowData*>(glfwGetWindowUserPointer(window));

        if (!want_capture_mouse()) {
            windowData->current_scroll_delta += (float) dy;
        }
    });

    Window window{};
    window.window = glfwWindow;
    window.window_data = std::make_shared<Window::WindowData>();
    window.base_title = name;

    glfwSetWindowUserPointer(glfwWindow, &*window.window_data);

    windows.insert(window);
    return window;
}

void WindowManager::destroy_window(const Window& window) {
    glfwDestroyWindow(window.window);
    windows.erase(window);
}

void WindowManager::set_v_sync(bool value) {
    glfwSwapInterval(value ? 1 : 0);
    v_sync_enabled = value;
}

bool WindowManager::get_v_sync() {
    return v_sync_enabled;
}

void WindowManager::update() {
    static double lastTime = 0.0;
    double time = glfwGetTime();
    if (lastTime != 0.0) {
        dt = time - lastTime;
    }
    lastTime = time;

    for (const auto& window: windows) {
        for (size_t i = 0; i < Window::WindowData::DELTA_ARRAY_COUNT; ++i) {
            window.window_data->motion_deltas[i] = window.window_data->current_motion_deltas[i];
            window.window_data->current_motion_deltas[i] = {0.0, 0.0};
        }

        memcpy(window.window_data->pressed_keys, window.window_data->current_pressed_keys, sizeof(bool) * Window::WindowData::PRESSED_KEY_COUNT);
        memset(window.window_data->current_pressed_keys, 0, sizeof(bool) * Window::WindowData::PRESSED_KEY_COUNT);

        window.window_data->scroll_delta = window.window_data->current_scroll_delta;
        window.window_data->current_scroll_delta = 0.0f;
    }

    glfwPollEvents();
}

double WindowManager::get_delta_time() const {
    return dt;
}

bool WindowManager::monitors_exist() {
    int count = 0;
    return glfwGetMonitors(&count) != nullptr && count > 0;
}

void WindowManager::cleanup() {
    glfwTerminate();
}
