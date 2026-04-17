#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <iostream>
#include <unordered_set>
#include <memory>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Window.h"

/// A class that manages a set of open windows
class WindowManager {
    std::unordered_set<Window> windows;

    // Just starts of with 60hz value, auto adjust with time though
    double dt = 1.0f / 60.0f;

    static bool v_sync_enabled;
public:
    /// An initial setup step, call once at the beginning
    static void init();
    /// Create a window manager
    WindowManager() = default;

    /// Creates a window with the given name and size
    Window create_window(const std::string& name, glm::ivec2 size);
    /// Destroys the window passed in, the window MUST not be used after this.
    void destroy_window(const Window& window);

    /// Globally sets v-sync on or off
    static void set_v_sync(bool value);
    /// Gets the global status of whether v-sync is enabled or not
    static bool get_v_sync();

    // Process window/key/mouse events since last call
    void update();

    // Get the time since the last call to update
    double get_delta_time() const;

    // Returns if glfw detects any monitors, used to help prevent a crass with ImGUI
    static bool monitors_exist();

    // Cleans up the global resources, the counter to init()
    static void cleanup();
};

#endif //WINDOW_MANAGER_H
