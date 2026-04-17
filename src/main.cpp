#include "system_interfaces/WindowManager.h"
#include "rendering/imgui/ImGuiManager.h"
#include "utility/OpenGL.h"
#include "utility/PerformanceCounter.h"
#include "rendering/resources/ModelLoader.h"
#include "rendering/resources/TextureLoader.h"
#include "rendering/renders/MasterRenderer.h"
#include "scene/SceneManager.h"
#include "scene/SceneInterface.h"
#include "scene/BasicStaticScene.h"
#include "scene/EditorScene.h"
#include "scene/SceneContext.h"

#define GLFW_INCLUDE_NONE

int main() {
    // Set up the window manager, then create a window with and make it the current context.
    WindowManager::init();
    WindowManager window_manager{};

    auto window = window_manager.create_window("Main Window", {1280, 720});
    window.make_context_current();
    window_manager.set_v_sync(false);

    // Uses the OpenGL context to load all the function pointers and also set up the debug callback
    // Note that the debug callback does not work on MacOS, instead if you want to check for errors
    // you will need to place calls to GL_CHECK_ERRORS() in key places, which will report the last
    // error to the console
    OpenGL::load_functions();
    OpenGL::setup_debug_callback();

    // Scope is to ensure that MasterRenderer destructor runs before the OpenGL context is destroyed
    {
        // Initialise ImGui to be used
        ImGuiManager imgui_manager{window};

        // Create a performance counter, to measure the FPS
        PerformanceCounter performance_counter{};

        // Create an instance of the MasterRenderer which controls all the rendering
        MasterRenderer master_renderer{};

        // Set up the model and texture loads, pointing them to a relative path to look in for files.
        ModelLoader model_loader{"res/models"};
        TextureLoader texture_loader{"res/textures"};

        // Create a scene manager and give it two scene constructors, one for the editor scene,
        // and another for an example second scene, this one just being a simple static scene.
        SceneManager scene_manager{};
        auto editor_scene = scene_manager.register_scene_generator("Editor Scene", []() {
            return std::make_shared<EditorScene::EditorScene>();
        });
        scene_manager.register_scene_generator("Basic Static Scene", []() {
            return std::make_shared<BasicStaticScene>();
        });

        // Create a SceneContext object to prevent needing to pass lots of variables into functions,
        // can just pass the one.
        SceneContext scene_context{
            window,
            window_manager,
            model_loader,
            texture_loader,
            true
        };
        // Use the handle of the editor scene to switch to it, making it the starting scene
        scene_manager.switch_scene(editor_scene, scene_context);

        // The game/render loop that runs until you close the program
        while (!window.should_close()) {
            // Process window/key/mouse events that have happened since the last loop
            window_manager.update();

            // Toggle the visibility of the ImGUI ui, with the pressing of the [`] key, typically left of [1].
            if (window.was_key_pressed(GLFW_KEY_GRAVE_ACCENT)) scene_context.imgui_enabled = !scene_context.imgui_enabled;
            // This is to try to prevent a crash that can happen when all monitors go to sleep
            bool was_imgui_enabled = scene_context.imgui_enabled;
            scene_context.imgui_enabled = scene_context.imgui_enabled && WindowManager::monitors_exist();
            ImGuiManager::set_disabled(!scene_context.imgui_enabled);

            if (scene_context.imgui_enabled) {
                // Tell ImGUI that we are starting a new frame, and to handle the docked/floating windows.
                imgui_manager.new_frame();
                ImGuiManager::enable_main_window_docking();
            }
            // Tell the MasterRenderer that we are staring a new frame
            master_renderer.update(window);

            if (scene_context.imgui_enabled) {
                // Create an ImGUI window for global options, that are independent of the scene
                if (ImGui::Begin("Options & Info", nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
                    scene_manager.add_imgui_options_section(scene_context);
                    master_renderer.add_imgui_options_section(window_manager);
                    performance_counter.add_imgui_options_section((float) window_manager.get_delta_time());
                }
                ImGui::End();
            }

            // Tick the scene, so it can do per-frame logic
            scene_manager.tick_scene(scene_context);
            // Tell the MasterRenderer to use render the current scene to the window
            master_renderer.render_scene(scene_manager.get_current_scene()->get_render_scene(), scene_context);

            if (scene_context.imgui_enabled) {
                // Tell ImGUI to now render itself onto the frame
                imgui_manager.render();
            }

            // Swap the image buffers, and if needed sleep to limit the fps
            window.swap_buffers();
            master_renderer.sync();

            scene_context.imgui_enabled = was_imgui_enabled;
        }

        // Cleanup some resources now that the program is closing
        scene_manager.cleanup(scene_context);

        texture_loader.cleanup();
        model_loader.cleanup();

        ImGuiManager::cleanup();
    }

    // Lastly destroy the window which will also destroy the OpenGL context, which is why it needs to be last
    window_manager.destroy_window(window);
    WindowManager::cleanup();

    return EXIT_SUCCESS;
}