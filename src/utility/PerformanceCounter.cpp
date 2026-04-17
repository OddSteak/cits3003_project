#include "PerformanceCounter.h"

#include "rendering/imgui/ImGuiManager.h"

#include <algorithm>

PerformanceCounter::PerformanceCounter() {
    frame_times.reserve(FRAME_COUNT);
    frame_times.push_back(0.0f); // To prevent / 0
}

float PerformanceCounter::values_getter(void* data, int idx) {
    auto* self = static_cast<PerformanceCounter*>(data);

    size_t display_count = std::min(self->frame_times.size(), self->FRAME_DISPLAY_COUNT);
    size_t display_offset = self->frame_times.size() - display_count;

    return self->frame_times[(self->start_index + display_offset + idx) % self->FRAME_COUNT] * 1.0e3f;
}

void PerformanceCounter::add_imgui_options_section(float frame_delta) {
    if (frame_times.size() == FRAME_COUNT) {
        frame_times[start_index] = frame_delta;
        start_index = (start_index + 1) % FRAME_COUNT;
    } else {
        frame_times.push_back(frame_delta);
    }

    if (ImGui::CollapsingHeader("Performance Metrics")) {
        size_t display_count = std::min(frame_times.size(), FRAME_DISPLAY_COUNT);

        ImGui::PlotLines("Frame Times (ms)", values_getter,
                         this, (int) display_count, 0, nullptr, FLT_MAX, FLT_MAX,
                         ImVec2(0, 40));

        float averageTime = 0.0f;
        float minTime = frame_times[0];
        float maxTime = frame_times[0];

        for (float val: frame_times) {
            averageTime += val;
            minTime = std::min(minTime, val);
            maxTime = std::max(maxTime, val);
        }

        averageTime /= (float) frame_times.size();

        ImGui::Text("Average Frame time: %.3f ms", averageTime * 1000.0f);
        ImGui::Text("Average Effective FPS: %.3f", 1.0f / averageTime);
        ImGui::Text("Min Frame time: %.3f ms", minTime * 1000.0f);
        ImGui::Text("Max Frame time: %.3f ms", maxTime * 1000.0f);
    }
}
