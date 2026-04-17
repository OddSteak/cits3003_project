#ifndef PERFORMANCE_COUNTER_H
#define PERFORMANCE_COUNTER_H

#include <vector>
#include <cstddef>

/// A performance counter class,
/// It is used to measure the FPS and plot it with ImGUI
class PerformanceCounter {
    std::vector<float> frame_times{};
    size_t start_index{};

    const size_t FRAME_COUNT = 200;
    const size_t FRAME_DISPLAY_COUNT = 200;

    static float values_getter(void* data, int idx);
public:
    PerformanceCounter();

    /// Adds the ImGUI control to the current ImGUI window
    void add_imgui_options_section(float frame_delta);
};

#endif //PERFORMANCE_COUNTER_H
