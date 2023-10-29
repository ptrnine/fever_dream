/*
 * Based on https://github.com/ocornut/imgui/issues/786
 */

#pragma once
#include <string>
#include <vector>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "core/math.hpp"
#include "types.hpp"

namespace ui
{
namespace imgui = ImGui;

struct bezier_preset {
    std::string_view name;
    std::array<float, 4> values;
};

static std::vector<bezier_preset> bezier_presets = {
    {"linear", {0, 0, 1, 1}},
    {"ease", core::interpolation::params::ease<float>},
    {"ease_in", core::interpolation::params::ease_in<float>},
    {"ease_out", core::interpolation::params::ease_out<float>},
};

struct bezier_state {
    int                  current_preset   = 0;
    std::array<float, 4> values           = {0, 0, 1, 1};
    float                area_width       = 220;
    unsigned             smoothness       = 64;
    float                grab_radius      = 8;
    float                grab_outline     = 2;
    float                line_width       = 1;
    float                curve_width      = 4;
    bool                 grab[2]          = {false, false};
    bool                 area_constrained = false;

    float test_size = 30.f;
    float test_t = 0.f;
    float test_vel = 1.5f;
    float test_dir = 1.f;
    float test_cur_vel = 0.f;

    static inline constexpr auto no_grab = std::numeric_limits<unsigned>::max();
    unsigned grab_index() const {
        for (unsigned i = 0; i < std::size(grab); ++i)
            if (grab[i])
                return i;
        return no_grab;
    }

    vec2f p1() const {
        return {values[0], values[1]};
    }

    vec2f p1_inv() const {
        return {values[0], 1.f - values[1]};
    }

    vec2f p2() const {
        return {values[2], values[3]};
    }

    vec2f p2_inv() const {
        return {values[2], 1.f - values[3]};
    }

    vec2f grab_values(unsigned i) const {
        return {values[i * 2], values[i * 2 + 1]};
    }

    vec2f grab_values() const {
        return grab_values(grab_index());
    }

    void set_grab_values(vec2f v) {
        auto i = grab_index();
        values[i * 2] = v.x();
        values[i * 2 + 1] = v.y();
    }
};

class bezier_editor {
public:
    bezier_editor(std::string iname): name(std::move(iname)) {}

    bool update(float timestep) {
        update_header();

        imgui::SameLine();

        const ImGuiStyle& Style = imgui::GetStyle();
        const ImGuiIO&    IO = imgui::GetIO();
        ImDrawList*       DrawList = imgui::GetWindowDrawList();
        ImGuiWindow*      Window = imgui::GetCurrentWindow();
        if (Window->SkipItems)
            return false;

        // header and spacing
        int changed = imgui::SliderFloat4("", state.values.data(), 0, 1, "%.3f", 1.0f);
        imgui::Dummy(ImVec2(0, 3));

        // prepare canvas
        vec2f avail = imgui::GetContentRegionAvail();
        float dim = state.area_width > 0 ? state.area_width : avail.x();
        auto  canvas = vec2f::filled_with(dim);

        constexpr float test_rect_gap = 25.f;

        vec2f push = {0, 0};
        if (avail.x() > dim)
            push.x() = (avail.x() - dim) * 0.5f;
        if (avail.y() > dim + (test_rect_gap + state.test_size))
            push.y() = (avail.y() - (dim + test_rect_gap + state.test_size)) * 0.5f;

        vec2f bb_min = Window->DC.CursorPos + push;
        vec2f bb_max = bb_min + canvas;
        imgui::ItemSize({bb_min, bb_max});
        if (!imgui::ItemAdd({bb_min, bb_max}, 0))
            return changed;

        const ImGuiID id = Window->GetID(name.data());
        imgui::ItemHoverable(ImRect(bb_min - push, bb_min + vec2f{avail.x(), dim} + push), id, 0);

        imgui::RenderFrame(bb_min, bb_max, imgui::GetColorU32(ImGuiCol_FrameBg, 1), true, Style.FrameRounding);

        // background grid
        for (float i = 0; i <= dim; i += dim * 0.25f) {
            DrawList->AddLine(vec2f{bb_min.x() + i, bb_min.y()},
                              vec2f{bb_min.x() + i, bb_max.y()},
                              imgui::GetColorU32(ImGuiCol_TextDisabled));
            DrawList->AddLine(vec2f{bb_min.x(), bb_min.y() + i},
                              vec2f{bb_max.x(), bb_min.y() + i},
                              imgui::GetColorU32(ImGuiCol_TextDisabled));
        }

        // control points: 2 lines and 2 circles
        {
            // handle grabbers
            vec2f mouse = imgui::GetIO().MousePos;
            vec2f pos[] = {
                lerp(bb_min, bb_max, state.p1_inv()),
                lerp(bb_min, bb_max, state.p2_inv()),
            };
            float distance[] = {
                (pos[0] - mouse).magnitude_2(),
                (pos[1] - mouse).magnitude_2(),
            };

            auto grab_index = state.grab_index();
            if (grab_index == state.no_grab) {
                auto closest = distance[0] < distance[1] ? 0 : 1;
                if (distance[closest] < (16 * state.grab_radius * state.grab_radius)) {
                    auto grab_values = state.grab_values(closest);
                    imgui::SetTooltip("(%4.3f, %4.3f)", grab_values.x(), grab_values.y());

                    if (imgui::IsMouseClicked(0))
                        state.grab[closest] = true;
                }
            }
            else {
                auto grab_values = state.grab_values();
                imgui::SetTooltip("(%4.3f, %4.3f)", grab_values.x(), grab_values.y());

                if (imgui::IsMouseDragging(0)) {
                    auto cnv = vec2f{canvas.x(), -canvas.y()};
                    auto p = grab_values + vec2f(imgui::GetIO().MouseDelta / cnv);

                    if (state.area_constrained)
                        for (auto& v : p.v) v = core::clamp(v, 0.f, 1.f);

                    state.set_grab_values(p);

                    changed = true;
                }

                if (state.grab[grab_index] && imgui::IsMouseReleased(0)) {
                    state.grab[grab_index] = false;
                }
            }
        }

        // draw curve
        {
            ImColor color(imgui::GetStyle().Colors[ImGuiCol_PlotLines]);
            for (float t = 0; t < 1.0f; t += 1.f / float(state.smoothness)) {
                auto next = t + 1.f / float(state.smoothness);
                auto t1 = core::bezier3(vec2f{0, 1}, state.p1_inv(), state.p2_inv(), vec2f{1, 0}, t);
                auto t2 = core::bezier3(vec2f{0, 1}, state.p1_inv(), state.p2_inv(), vec2f{1, 0}, next);
                t1 = core::lerp(bb_min, bb_max, t1);
                t2 = core::lerp(bb_min, bb_max, t2);
                DrawList->AddLine(t1, t2, color, state.curve_width);
            }
        }

        // draw lines and grabbers
        float  luma = imgui::IsItemActive() || imgui::IsItemHovered() ? 0.5f : 1.0f;
        ImVec4 pink(1.f, 0.f, 0.75f, luma), cyan(0.f, 0.75f, 1.f, luma);
        ImVec2 p1 = ImVec2(state.values[0], 1 - state.values[1]) * (bb_max - bb_min) + bb_min;
        ImVec2 p2 = ImVec2(state.values[2], 1 - state.values[3]) * (bb_max - bb_min) + bb_min;

        ImVec4 white(imgui::GetStyle().Colors[ImGuiCol_Text]);
        DrawList->AddLine(vec2f{bb_min.x(), bb_max.y()}, p1, ImColor(white), state.line_width);
        DrawList->AddLine(vec2f{bb_max.x(), bb_min.y()}, p2, ImColor(white), state.line_width);
        DrawList->AddCircleFilled(p1, state.grab_radius, ImColor(white));
        DrawList->AddCircleFilled(p1, state.grab_radius - state.grab_outline, ImColor(pink));
        DrawList->AddCircleFilled(p2, state.grab_radius, ImColor(white));
        DrawList->AddCircleFilled(p2, state.grab_radius - state.grab_outline, ImColor(cyan));

        /* Draw test rect */
        auto cubic_bezier = core::interpolation::cubic_bezier<float>(state.values);
        auto x_max = bb_max.x() - state.test_size;
        auto pos = state.test_dir > 0.f ? cubic_bezier(bb_min.x(), x_max, state.test_t)
                                        : cubic_bezier(x_max, bb_min.x(), 1.f - state.test_t);

        vec2f test_bb_min = {pos, bb_max.y() + test_rect_gap};
        vec2f test_bb_max = test_bb_min + vec2f::filled_with(state.test_size);
        DrawList->AddRectFilled(test_bb_min, test_bb_max, ImColor(1.f, 0.f, 0.75f), 4.f);

        if (imgui::IsMouseClicked(0)) {
            ImRect test_bb_rect{test_bb_min, test_bb_max};
            if (test_bb_rect.Contains(imgui::GetIO().MousePos))
                state.test_cur_vel = state.test_vel;
        }

        state.test_t += timestep * state.test_cur_vel * state.test_dir;
        if (state.test_t > 1.f) {
            state.test_t = 1.f;
            state.test_dir = -1.f;
            state.test_cur_vel = 0.f;
        }
        else if (state.test_t < 0.f) {
            state.test_t = 0.f;
            state.test_dir = 1.f;
            state.test_cur_vel = 0.f;
        }

        return true;
    }

    auto& values() const {
        return state.values;
    }

private:
    void update_header() {
        imgui::PushID(name.data());
        //imgui::SameLine();

        if (imgui::Button("Presets")) {
            imgui::OpenPopup("!Presets");
        }
        if (imgui::BeginPopup("!Presets")) {
            for (size_t i = 0; i < bezier_presets.size(); ++i) {
                auto& preset = bezier_presets[i];
                if (imgui::MenuItem(preset.name.data(), nullptr, i == state.current_preset)) {
                    state.current_preset = i;
                    state.values         = bezier_presets[i].values;
                }
            }
            imgui::EndPopup();
        }
        imgui::PopID();
    }

private:
    std::string name;
    bezier_state state;
};
} // namespace ui
