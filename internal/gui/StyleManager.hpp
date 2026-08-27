#pragma once

#include <algorithm>

class StyleManager {
public:
    void Styles() {
        auto& style = GImGui->Style;

        style.WindowRounding = 4;
        style.WindowPadding = ImVec2{ 0, 0 };
        style.WindowBorderSize = 0;

        style.FrameRounding = 3;
        style.FramePadding = ImVec2{ 14, 12 };
        style.FrameBorderSize = 0;

        style.PopupRounding = 3;
        style.PopupBorderSize = 0;

        style.ChildRounding = 4;
        style.ChildBorderSize = 1;

        style.ItemSpacing = ImVec2{ 14, 14 };
        style.ItemInnerSpacing = ImVec2{ 10, 10 };

        style.ScrollbarRounding = 4;
        style.ScrollbarSize = 4;
        style.WindowMinSize = ImVec2{ 1, 1 };
    }

    void Colors() {
        ImVec4* colors = GImGui->Style.Colors;

        ImVec4 bg_main = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        ImVec4 bg_child = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        ImVec4 accent = ImVec4(0.28f, 0.15f, 0.65f, 0.90f);
        ImVec4 text = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
        ImVec4 border = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        ImVec4 frame_bg = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

        ImVec4 accent_active = ImVec4((std::min)(accent.x + 0.15f, 1.0f), (std::min)(accent.y + 0.12f, 1.0f), (std::min)(accent.z + 0.10f, 1.0f), 1.0f);
        ImVec4 accent_hovered = ImVec4((std::min)(accent.x + 0.05f, 1.0f), (std::min)(accent.y + 0.05f, 1.0f), (std::min)(accent.z + 0.05f, 1.0f), 1.0f);

        colors[ImGuiCol_Scheme] = accent;

        colors[ImGuiCol_TextButton] = text;
        colors[ImGuiCol_TextDisabled] = ImVec4(text.x, text.y, text.z, 0.6f);

        colors[ImGuiCol_WindowBg] = bg_main;
        colors[ImGuiCol_ChildBg] = bg_child;
        colors[ImGuiCol_PopupBg] = bg_main;
        colors[ImGuiCol_Border] = border;
        colors[ImGuiCol_Text] = text;
        colors[ImGuiCol_TextHovered] = ImVec4(text.x, text.y, text.z, 0.8f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.07f);

        colors[ImGuiCol_FrameBg] = frame_bg;
        colors[ImGuiCol_FrameBgHovered] = ImVec4((std::min)(frame_bg.x + 0.02f, 1.0f), (std::min)(frame_bg.y + 0.02f, 1.0f), (std::min)(frame_bg.z + 0.02f, 1.0f), 1.0f);
        colors[ImGuiCol_FrameBgActive] = ImVec4((std::min)(frame_bg.x + 0.04f, 1.0f), (std::min)(frame_bg.y + 0.04f, 1.0f), (std::min)(frame_bg.z + 0.04f, 1.0f), 1.0f);

        colors[ImGuiCol_Button] = frame_bg;
        colors[ImGuiCol_ButtonHovered] = accent;
        colors[ImGuiCol_ButtonActive] = accent_active;

        colors[ImGuiCol_CheckMark] = accent;
        colors[ImGuiCol_SliderGrab] = accent;
        colors[ImGuiCol_SliderGrabActive] = accent_active;

        colors[ImGuiCol_Header] = ImVec4(accent.x, accent.y, accent.z, 0.3f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(accent.x, accent.y, accent.z, 0.5f);
        colors[ImGuiCol_HeaderActive] = accent;

        colors[ImGuiCol_Tab] = frame_bg;
        colors[ImGuiCol_TabHovered] = accent_hovered;
        colors[ImGuiCol_TabActive] = accent;
        colors[ImGuiCol_TabUnfocused] = frame_bg;
        colors[ImGuiCol_TabUnfocusedActive] = accent;

        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.0f);
        colors[ImGuiCol_ScrollbarGrab] = accent;
        colors[ImGuiCol_ScrollbarGrabHovered] = accent;
        colors[ImGuiCol_ScrollbarGrabActive] = accent;
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_Separator] = border;
        colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    static StyleManager& get() {
        static StyleManager s{ };
        return s;
    }
};