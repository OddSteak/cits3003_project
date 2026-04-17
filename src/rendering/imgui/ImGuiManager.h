#ifndef IMGUI_MANAGER_H
#define IMGUI_MANAGER_H

#include "system_interfaces/WindowManager.h"
#include "ImGuiImpl.h"

#include "../../scene/SceneInterface.h"

/// A helper class to make using ImGUI a bit easier
class ImGuiManager {
    Window window;

    static bool disabled;
    static bool was_cursor_disabled;
public:
    /// Construct the manager, targeting a main window.
    explicit ImGuiManager(Window window);

    /// Start a new ImGUI frame
    void new_frame();
    /// Render the last ImGUI frame
    void render();
    /// Cleanup the manager
    static void cleanup();
    /// Enable and configure the docking feature, needs to be called every ImGUI frame
    static void enable_main_window_docking();

    /// Set the global disabled flag
    static void set_disabled(bool set_disabled);

    /// Set the global cursor was disabled flag
    static void set_cursor_was_disabled();

    /// Returns true if ImGUI is trying to capture the keyboard, e.g. when typing in an input box,
    /// in which things like the camera should ignore keyboard input.
    static bool want_capture_keyboard();
    /// Returns true if ImGUI is trying to capture the mouse, e.g. when trying to interact with the GUI,
    /// in which things like the camera should ignore mouse input.
    static bool want_capture_mouse();
};

namespace ImGui {
    /// A helper that takes a std::string for convenience
    bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags);

    /// Add a helper marker with the provide tooltip on hover.
    void HelpMarker(const char* desc);

    /// Call this after a ImGui::Drag* function to disable the cursor while dragging it, allowing infinite dragging
    void DragDisableCursor(Window& window);
}

#endif //IMGUI_MANAGER_H
