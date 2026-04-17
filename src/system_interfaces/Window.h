#ifndef PROJECT_WINDOW_H
#define PROJECT_WINDOW_H

#include <string>
#include <memory>
#include <optional>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

/// A class representing a Window created by the WindowManager
class Window {
    friend class WindowManager;

public:
    bool operator==(const Window& rhs) const;
    bool operator<(const Window& rhs) const;
    bool operator>(const Window& rhs) const;
    bool operator<=(const Window& rhs) const;
    bool operator>=(const Window& rhs) const;
    bool operator!=(const Window& rhs) const;

private:
    std::string base_title{};
    GLFWwindow* window = nullptr;

    struct WindowData {
        static const size_t DELTA_ARRAY_COUNT = (GLFW_MOUSE_BUTTON_LAST - GLFW_MOUSE_BUTTON_1 + 2);

        glm::dvec2 current_motion_deltas[DELTA_ARRAY_COUNT];
        glm::dvec2 motion_deltas[DELTA_ARRAY_COUNT];

        static const size_t PRESSED_KEY_COUNT = GLFW_KEY_LAST + 1;

        bool current_pressed_keys[PRESSED_KEY_COUNT];
        bool pressed_keys[PRESSED_KEY_COUNT];

        float current_scroll_delta = 0.0f;
        float scroll_delta;
    };

    std::shared_ptr<WindowData> window_data = nullptr;
public:
    /// Makes the OpenGL context associated with this window, current in the calling thread()
    void make_context_current();
    /// Causes the window to swap buffers
    void swap_buffers();

    /// Returns true if the user has clicked the close button (or something else like it)
    [[nodiscard]] bool should_close() const;
    /// Manually signal that the window should close
    void set_should_close() const;
    /// Checks if the window is currently in focus
    [[nodiscard]] bool is_focused() const;
    /// Brings the window into focus if it is not currently
    void focus();

    /// Returns the <dx, dy> of the mouse since the last call to WindowManager.update()
    /// If button != 0, it returns the <dx, dy> of the mouse while the mouse button was pressed.
    /// Valid values are those like [GLFW_MOUSE_BUTTON_LEFT]
    [[nodiscard]] glm::dvec2 get_mouse_delta(int button = 0) const;
    /// Returns the current mouse position in windows content area, in pixels, with (0, 0) in top left, and (w, h) in bottom right
    [[nodiscard]] glm::dvec2 get_mouse_pos() const;
    /// Returns the current mouse position in NDC, that is (-1, -1) in bottom left, and (1, 1) in top right
    [[nodiscard]] glm::vec2 get_mouse_pos_ndc() const;

    /// Returns the amount of scroll (signed) since the last call to WindowManager.update()
    [[nodiscard]] float get_scroll_delta() const;

    /// Returns true if the given key (e.g. GLFW_KEY_W) is currently pressed
    [[nodiscard]] bool is_key_pressed(int key) const;
    /// Returns true if the key (e.g. GLFW_KEY_W) was first pressed (so not repeat) since the last call to WindowManager.update()
    [[nodiscard]] bool was_key_pressed(int key) const;

    /// Returns if the given mouse button (e.g. GLFW_MOUSE_BUTTON_LEFT) is currently pressed
    [[nodiscard]] bool is_mouse_pressed(int button) const;

    void set_cursor_disabled(bool disabled) const;

    /// Returns the size of the window (this differs from framebuffer size if DPI scaling is used)
    [[nodiscard]] glm::ivec2 get_window_size() const;
    /// Returns the height of the window (this differs from framebuffer size if DPI scaling is used)
    [[nodiscard]] uint32_t get_window_width() const;
    /// Returns the width of the window (this differs from framebuffer size if DPI scaling is used)
    [[nodiscard]] uint32_t get_window_height() const;

    /// Returns the size of the framebuffer backing the window (this differs from normal size if DPI scaling is used)
    [[nodiscard]] glm::ivec2 get_framebuffer_size() const;
    /// Returns the aspect ratio of the framebuffer backing the window (this differs from normal size if DPI scaling is used)
    [[nodiscard]] float get_framebuffer_aspect_ratio() const;
    /// Returns the width of the framebuffer backing the window (this differs from normal size if DPI scaling is used)
    [[nodiscard]] uint32_t get_framebuffer_width() const;
    /// Returns the height of the framebuffer backing the window (this differs from normal size if DPI scaling is used)
    [[nodiscard]] uint32_t get_framebuffer_height() const;

    /// Sets a suffix to the title of the window, use std::nullopt to denote no suffix
    void set_title_suffix(std::optional<std::string> suffix);

    /// Retrieve the internal GLFWwindow handle in case you want to do something not exposed by the API
    [[nodiscard]] GLFWwindow* internal() const;
};

template<>
struct std::hash<Window> {
    std::size_t operator()(Window const& s) const noexcept {
        // size_t is by definition the same size as a pointer, and each
        // Window has a unique pointer to a GLFW window
        return reinterpret_cast<std::size_t>(s.internal());
    }
};


#endif //PROJECT_WINDOW_H
