#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <functional>
#include <memory>
#include <map>

#include "SceneInterface.h"
#include "scene/SceneContext.h"

class SceneManager;

/// A handle to a SceneGenerator that has been added to the scene manager,
/// can then be used to tell the SceneManager to switch to that scene.
struct SceneGeneratorHandle {
private:
    explicit SceneGeneratorHandle(int handle) : handle(handle) {}

    int handle;

    friend class SceneManager;
};

class SceneManager {
    struct SceneGenerator {
        std::string name;
        std::function<std::shared_ptr<SceneInterface>()> generator;
    };

    int next_handle = 1;
    std::unordered_map<int, SceneGenerator> scene_generators{};
    std::vector<int> ordered_scene_generators{};

    std::shared_ptr<SceneInterface> current_scene = nullptr;

    // ImGUI state variables
    int selected_scene_generator = 0;
public:
    SceneManager() = default;

    /// Add a scene generator function to the SceneManager, names can be duplicated, the returned handle uniquely references the added generator.
    SceneGeneratorHandle register_scene_generator(std::string name, std::function<std::shared_ptr<SceneInterface>()> scene_generator);
    /// Generate a scene using the specified generator and switch to that scene
    void switch_scene(const SceneGeneratorHandle& generator_handle, const SceneContext& scene_context);
    /// Switch to the scene passed in
    void switch_scene(std::shared_ptr<SceneInterface> scene, const SceneContext& scene_context);

    /// Adds the ImGUI controls owned by the SceneManager, including the scene generator selector to switch between all provided generators
    void add_imgui_options_section(const SceneContext& scene_context);
    /// Ticks the current scene
    void tick_scene(const SceneContext& scene_context);

    /// Returns a ref to the current scene
    const std::shared_ptr<SceneInterface>& get_current_scene() const;

    /// Cleans up the SceneManager, doing thing like calling close() of the current Scene
    void cleanup(const SceneContext& scene_context);
};

#endif //SCENE_MANAGER_H
