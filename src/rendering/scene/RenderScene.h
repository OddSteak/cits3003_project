#ifndef RENDER_SCENE_H
#define RENDER_SCENE_H

#include <memory>
#include <unordered_set>

/// A generic RenderScene for Renderers to use
template<typename Entity, typename GlobalData>
struct RenderScene {
    std::unordered_set<std::shared_ptr<Entity>> entities{};
    GlobalData global_data{};
};

#endif //RENDER_SCENE_H
