#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>
#include <imgui_internal.h>
#include <vector>
#include <string>
#include <functional>
#include <animations.hpp>

#include <compbuilder/CompBuilder.hpp>
#include "WidgetsManager.hpp"
#include "ColorPickerManager.hpp"
#include "FontManager.hpp"
#include <unicodes.hpp>
#include "ChildManager.hpp"
#include "TabsManager.hpp"
#include "LangManager.hpp"
#include "SearchManager.hpp"

using namespace ImGui;

bool WidgetsManager::Checkbox(const char* label, bool* v, int* key, float* col, std::function< void() > options, bool warning) {
    SearchManager::get().additem(label, [=]() { Checkbox(label, v, key, col, options, warning); });

    float square_sz = 16;

    ImRect total_bb{ GetCurrentWindow()->DC.CursorPos, GetCurrentWindow()->DC.CursorPos + ImVec2{ CalcTextSize(label, 0, 1).x + GImGui->Style.ItemSpacing.x + square_sz, square_sz } };

    if (warning) {
        total_bb.Min.x += 24;
        total_bb.Max.x += 24;
    }

    ImRect bb{ total_bb.Min, total_bb.Min + ImVec2{ square_sz, square_sz } };
    ImVec2 options_pos{ bb.Min.x + CalcItemWidth() - 24.f * warning, bb.Min.y };

    bool prev_v = *v;
    bool res = CompBuilder::get().Checkbox(label, v, key, col, options, warning, total_bb, bb, options_pos, [&](CompBuilder::CheckboxEnv env) {
        ImColor col = col_anim(col_anim(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_TextDisabled, 0.6f), env.anim.hover), GetColorU32(ImGuiCol_Text), env.anim.enabled);

        GetWindowDrawList()->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), 2);
        GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_Scheme, env.anim.enabled), 2);
        RenderCheckMark(GetWindowDrawList(), bb.GetCenter() - ImVec2{ 4, 4 }, GetColorU32(ImGuiCol_ChildBg, env.anim.enabled), 8);

        GetWindowDrawList()->AddText({ total_bb.Min.x + GImGui->Style.ItemInnerSpacing.x + square_sz, total_bb.GetCenter().y - GImGui->FontSize / 2 - 0.5f }, col, env.label, FindRenderedTextEnd(env.label));
        });

    if (prev_v != *v) {
        ui::play_sound(0);
    }

    return res;
}

template < typename T >
bool WidgetsManager::Slider(const char* label, T* v, T min, T max, const char* format) {
    SearchManager::get().additem(label, [=]() { Slider(label, v, min, max, format); });

    ImRect total_bb{ GetCurrentWindow()->DC.CursorPos, GetCurrentWindow()->DC.CursorPos + ImVec2{ CalcItemWidth(), GImGui->FontSize + GImGui->Style.ItemInnerSpacing.y + 5 } };
    ImRect bb{ total_bb.Max - ImVec2{ total_bb.GetWidth(), 5 }, total_bb.Max };
    return CompBuilder::get().Slider(label, v, min, max, format,
        total_bb,
        bb,
        [&](const CompBuilder::SliderEnv& env) {
            ImColor col = col_anim(col_anim(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_TextDisabled, 0.6f), env.anim.hover), GetColorU32(ImGuiCol_Text), env.anim.held);

            GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), 2);
            GetWindowDrawList()->AddRectFilled(bb.Min, bb.Min + ImVec2{ env.anim.val_anim, bb.GetHeight() }, GetColorU32(ImGuiCol_Scheme), 2);
            GetWindowDrawList()->AddCircleFilled({ bb.Min.x + env.anim.val_anim, bb.GetCenter().y }, 5.5f + env.anim.anim - 2.f * env.anim.held, GetColorU32(ImGuiCol_Text), 36);

            GetWindowDrawList()->AddText(total_bb.Min, GetColorU32(ImGuiCol_Text), env.label, FindRenderedTextEnd(env.label));
            GetWindowDrawList()->AddText({ total_bb.Max.x - CalcTextSize(env.buf).x, total_bb.Min.y }, col, env.buf);
        });
}

bool WidgetsManager::SliderInt(const char* label, int* v, int min, int max, const char* format) {
    return Slider(label, v, min, max, format);
}

bool WidgetsManager::SliderFloat(const char* label, float* v, float min, float max, const char* format) {
    return Slider(label, v, min, max, format);
}

bool WidgetsManager::ComboEx(const char* label, const char* preview_value, std::function< void(CompBuilder::ComboEnv env) > code) {
    ImRect total_bb{ GetCurrentWindow()->DC.CursorPos, GetCurrentWindow()->DC.CursorPos + ImVec2{ CalcItemWidth(), GetFrameHeight() + GImGui->FontSize + GImGui->Style.ItemInnerSpacing.y } };
    ImRect bb{ total_bb.Max - ImVec2{ CalcItemWidth(), GetFrameHeight() }, total_bb.Max };

    CompBuilder::get().Combo(label, total_bb, bb, [&](const CompBuilder::ComboEnv& env) {
        ImColor col = col_anim(col_anim(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_TextDisabled, 0.6f), env.anim.hover), GetColorU32(ImGuiCol_Scheme), env.anim.open);

        GetWindowDrawList()->AddText(total_bb.Min, GetColorU32(ImGuiCol_Text), env.label, FindRenderedTextEnd(env.label));

        GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), GImGui->Style.FrameRounding, env.open ? ImDrawFlags_RoundCornersTop : ImDrawFlags_RoundCornersAll);
        GetWindowDrawList()->AddText(bb.Min + GImGui->Style.FramePadding, GetColorU32(ImGuiCol_Text), LangManager::get().translate(preview_value), FindRenderedTextEnd(LangManager::get().translate(preview_value)));
        GetWindowDrawList()->AddText(fonts[icons].get(14), 14, { bb.Max.x - GImGui->Style.FramePadding.x - 14, bb.GetCenter().y - 7.5f }, col, i_chevron_selector_vertical);

        if (env.anim.open > 0.05f) {
            SetNextWindowPos({ bb.Min.x, bb.Max.y });
            PushStyleVar(ImGuiStyleVar_Alpha, env.anim.open);
            PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
            PushStyleVar(ImGuiStyleVar_WindowRounding, GImGui->Style.FrameRounding);
            PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
            PushStyleColor(ImGuiCol_WindowBg, GetColorU32(ImGuiCol_FrameBg));
            Begin(label, 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
            {
                SetWindowSize({ bb.GetWidth(), (GetCurrentWindow()->ContentSize.y + GImGui->Style.WindowRounding) * env.anim.open });

                BringWindowToDisplayFront(GetCurrentWindow());
                BringWindowToFocusFront(GetCurrentWindow());

                GetWindowDrawList()->AddRectFilled(GetWindowPos(), GetWindowPos() + GetWindowSize(), GetColorU32(ImGuiCol_WindowBg), GImGui->Style.WindowRounding, ImDrawFlags_RoundCornersBottom);
                GetWindowDrawList()->AddRect(GetWindowPos(), GetWindowPos() + GetWindowSize(), GetColorU32(ImGuiCol_Border), GImGui->Style.WindowRounding, ImDrawFlags_RoundCornersBottom);

                if (!IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && IsMouseClicked(0) && !env.hovered) {
                    env.open = false;
                }

                SetCursorPosY(GImGui->Style.WindowRounding);
                PushStyleColor(ImGuiCol_FrameBg, GetColorU32(ImGuiCol_FrameBgHovered));

                code(env);

                PopStyleColor();
            }
            End();
            PopStyleColor();
            PopStyleVar(4);
        }
        });

    return false;
}

bool WidgetsManager::Combo(const char* label, int* v, std::vector< const char* > items) {
    SearchManager::get().additem(label, [=]() { Combo(label, v, items); });

    int prev_v = *v;

    ComboEx(label, items[*v], [&](CompBuilder::ComboEnv env) {
        for (int i = 0; i < items.size(); ++i) {
            if (Selectable(items[i], *v == i)) {
                *v = i;
                env.open = !env.open;
            }
        }
        });

    return prev_v != *v;
}

bool WidgetsManager::MultiCombo(const char* label, bool* v, std::vector< const char* > items) {
    SearchManager::get().additem(label, [=]() { MultiCombo(label, v, items); });

    auto& style = GetStyle();

    std::string buf;

    buf.clear();
    for (size_t i = 0; i < items.size(); ++i) {
        if (v[i]) {
            buf += LangManager::get().translate(items[i]);
            buf += ", ";
        }
    }

    if (!buf.empty()) {
        buf.resize(buf.size() - 2);
    }

    if (CalcTextSize(buf.c_str()).x > 160 - style.FramePadding.x * 3 - 10) {
        for (int i = 0; i < buf.size() - 1; ++i) {
            if (CalcTextSize(buf.substr(0, i + 1).c_str()).x > 160 - style.FramePadding.x - 10) {
                buf.resize(i);
                if (buf[buf.size() - 1] == ',') {
                    buf.resize(buf.size() - 1);
                }
                buf.append("..");
            }
        }
    }

    ComboEx(label, buf.c_str(), [&](CompBuilder::ComboEnv env) {
        for (int i = 0; i < items.size(); ++i) {
            if (Selectable(items[i], v[i])) {
                v[i] = !v[i];
            }
        }
        });

    return false;
}

bool WidgetsManager::Bind(const char* label, HotkeyState* state, bool* is_enabled) {
    SearchManager::get().additem(label, [=]() { Bind(label, state, is_enabled); });

    auto it = std::find_if(hotkey_registry.begin(), hotkey_registry.end(), [state](const HotkeyEntry& e) { return e.state == state; });
    if (it == hotkey_registry.end()) {
        hotkey_registry.push_back({ label, state, is_enabled });
    }

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    float avail_x = ImGui::GetContentRegionAvail().x;
    ImVec2 start_pos = window->DC.CursorPos;

    bool val_changed = Checkbox(label, is_enabled);
    ImVec2 next_pos = window->DC.CursorPos;

    float btn_w = 62.0f;
    float btn_h = 25.0f;

    ImVec2 btn_pos_min = ImVec2(start_pos.x + avail_x - btn_w, start_pos.y - (btn_h - 16.0f) / 2.0f);
    ImRect btn_bb(btn_pos_min, ImVec2(btn_pos_min.x + btn_w, btn_pos_min.y + btn_h));

    ImGui::SetCursorScreenPos(btn_pos_min);

    ImGui::PushID(state);
    ImGuiID id = window->GetID("binder");

    if (!ImGui::ItemAdd(btn_bb, id)) {
        ImGui::PopID();
        ImGui::SetCursorScreenPos(next_pos);
        return val_changed;
    }

    bool btn_hovered = ImGui::ItemHoverable(btn_bb, id, (ImGuiItemFlags)0);

    bool* waiting = window->DC.StateStorage->GetBoolRef(id, false);
    float* anim = window->DC.StateStorage->GetFloatRef(id + 1, 0.0f);
    bool* just_activated = window->DC.StateStorage->GetBoolRef(id + 2, false);

    bool is_always = (state->mode == 2);

    if (!*is_enabled) {
        *waiting = false;
        *just_activated = false;
        if (g.ActiveId == id) {
            ImGui::ClearActiveID();
        }
    }

    if (*is_enabled && !is_always) {
        if (btn_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !*waiting) {
            *waiting = true;
            *just_activated = true;
            ImGui::SetActiveID(id, window);
        }
    }

    *anim = ImLerp(*anim, ((*is_enabled && btn_hovered && !is_always) || *waiting) ? 1.0f : 0.0f, g.IO.DeltaTime * 12.0f);

    if (*waiting && *is_enabled && !is_always) {
        if (g.ActiveId != id) {
            ImGui::SetActiveID(id, window);
        }

        g.IO.WantCaptureMouse = true;
        g.IO.WantCaptureKeyboard = true;

        bool was_activated = *just_activated;

        if (was_activated) {
            if (!GetAsyncKeyState(VK_LBUTTON)) {
                *just_activated = false;
            }
        }

        if (!was_activated) {
            bool input_received = false;

            for (int i = 1; i < 255; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    if (i == VK_ESCAPE || i == VK_DELETE) {
                        state->key = 0;
                    }
                    if (i != VK_ESCAPE && i != VK_DELETE) {
                        state->key = i;
                    }
                    input_received = true;
                    val_changed = true;
                    break;
                }
            }

            if (input_received) {
                *waiting = false;
                *just_activated = false;
                ImGui::ClearActiveID();
            }
        }
    }

    ImU32 bg_col = ImGui::GetColorU32(ImLerp(style.Colors[ImGuiCol_FrameBg], style.Colors[ImGuiCol_FrameBgActive], *anim));
    window->DrawList->AddRectFilled(btn_bb.Min, btn_bb.Max, bg_col, 2);

    char k_buf[64];
    if (is_always) {
        ImFormatString(k_buf, sizeof(k_buf), "ALWAYS");
    }
    else {
        ImFormatString(k_buf, sizeof(k_buf), "NONE");

        if (*waiting) {
            ImFormatString(k_buf, sizeof(k_buf), "...");
        }
        else if (state->key != 0) {
            const char* mouse_name = nullptr;
            switch (state->key) {
            case VK_LBUTTON:  mouse_name = "M1"; break;
            case VK_RBUTTON:  mouse_name = "M2"; break;
            case VK_MBUTTON:  mouse_name = "M3"; break;
            case VK_XBUTTON1: mouse_name = "M4"; break;
            case VK_XBUTTON2: mouse_name = "M5"; break;
            }

            if (mouse_name) {
                ImFormatString(k_buf, sizeof(k_buf), "%s", mouse_name);
            }
            if (!mouse_name) {
                ImFormatString(k_buf, sizeof(k_buf), "%s", ui::GetKeyNameLegacy(state->key));
            }
        }
    }

    ImFont* font_ptr = fonts[font].get(12);
    ImFont* icon_ptr = fonts[icons].get(12);

    ImVec2 text_sz = font_ptr->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, k_buf);

    if (is_always) {
        float text_x = btn_bb.Min.x + (btn_bb.GetWidth() - text_sz.x) / 2.0f;
        float text_y = btn_bb.GetCenter().y - text_sz.y / 2.0f;
        window->DrawList->AddText(font_ptr, 12.0f, ImVec2(text_x, text_y), GetColorU32(ImGuiCol_Text), k_buf);
    }
    else {
        ImVec2 icon_sz = icon_ptr->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, i_keyboard_02);

        float text_x = btn_bb.Min.x + ((btn_bb.GetWidth() - 20.0f) - text_sz.x) / 2.0f;
        float text_y = btn_bb.GetCenter().y - text_sz.y / 2.0f;

        float icon_x = btn_bb.Max.x - 18.0f;
        float icon_y = btn_bb.GetCenter().y - icon_sz.y / 2.0f;

        window->DrawList->AddText(font_ptr, 12.0f, ImVec2(text_x, text_y), GetColorU32(ImGuiCol_Text), k_buf);
        window->DrawList->AddText(icon_ptr, 12.0f, ImVec2(icon_x, icon_y), GetColorU32(ImGuiCol_Scheme), i_keyboard_02);
    }

    if (*is_enabled && btn_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("##popup");
    }

	ImGui::SetNextWindowSize({ 105.0f, 0.0f });
	if (ImGui::BeginPopup("##popup")) {
		if (ImGui::MenuItem(LangManager::get().translate("Hold"), nullptr, state->mode == 0)) state->mode = 0;
		if (ImGui::MenuItem(LangManager::get().translate("Toggle"), nullptr, state->mode == 1)) state->mode = 1;
		if (ImGui::MenuItem(LangManager::get().translate("Always On"), nullptr, state->mode == 2)) state->mode = 2;
		ImGui::EndPopup();
	}

    ImGui::PopID();
    ImGui::SetCursorScreenPos(next_pos);

    return val_changed;
}

bool WidgetsManager::Binder(const char* label, int* key) {
    SearchManager::get().additem(label, [=]() { Binder(label, key); });

    CompBuilder::get().Binder(label, key, [&](const CompBuilder::BinderEnv& env) {
        GetWindowDrawList()->AddText({ env.total_bb.Min.x, env.total_bb.GetCenter().y - GImGui->FontSize / 2 }, GetColorU32(ImGuiCol_Text), env.label, FindRenderedTextEnd(env.label));

        GetWindowDrawList()->AddRectFilled(env.bb.Min, env.bb.Max, GetColorU32(ImGuiCol_FrameBg), 2);
        GetWindowDrawList()->AddText(fonts[font].get(12), 12, env.bb.Min + ImVec2{ 6, env.bb.GetHeight() / 2 - 6 }, GetColorU32(ImGuiCol_Text), env.keys[*key]);
        GetWindowDrawList()->AddText(fonts[icons].get(12), 12, env.bb.Min + ImVec2{ env.bb.GetWidth() - 18, env.bb.GetHeight() / 2 - 6 }, GetColorU32(ImGuiCol_Scheme), i_keyboard_02);
        });

    return false;
}

bool WidgetsManager::TextField(const char* label, char* buf, size_t buf_size, ImVec2 size, const char* hint, const char* icon) {
    SearchManager::get().additem(label, [=]() { TextField(label, buf, buf_size, size, hint, icon); });

    char str_id[64];
    ImFormatString(str_id, sizeof(str_id), "##%s", label);

    ImVec2 pos = GetCursorPos();

    bool value_changed = false;

    if (CalcTextSize(label, 0, 1).x > 0) {
        PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, GImGui->Style.ItemInnerSpacing.y });
        TextEx(label, FindRenderedTextEnd(label));
        PopStyleVar();
    }

    if (icon) {
        ImRect bb{ GetCurrentWindow()->DC.CursorPos, GetCurrentWindow()->DC.CursorPos + CalcItemSize(size, CalcItemWidth(), GetFrameHeight()) };
        GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), GImGui->Style.FrameRounding);

        if (GImGui->Style.FrameBorderSize != 0) {
            GetWindowDrawList()->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), GImGui->Style.FrameRounding);
        }

        GetWindowDrawList()->AddText(FontManager::get().get_fonts().at(icons).get(14), 14, bb.Min + GImGui->Style.FramePadding - ImVec2{ 0, 1 }, GetColorU32(ImGuiCol_TextDisabled), icon);

        PushStyleColor(ImGuiCol_FrameBg, GetColorU32(ImGuiCol_FrameBg, 0));
        PushStyleColor(ImGuiCol_Border, GetColorU32(ImGuiCol_Border, 0));
        SetCursorPosX(pos.x + 24);
        SetCursorPosY(pos.y - 1);
        value_changed = InputTextEx(str_id, LangManager::get().translate(hint), buf, buf_size, size - ImVec2{ 24, 0 }, 0);
        PopStyleColor(2);
    }
    else {
        value_changed = InputTextEx(str_id, LangManager::get().translate(hint), buf, buf_size, size, 0);
    }

    return value_changed;
}

bool WidgetsManager::Button(const char* label, ImVec2 size) {
    return CompBuilder::get().Button(label, size, [&](const CompBuilder::ButtonEnv& env) {
        auto col = col_anim(col_anim(GetColorU32(ImGuiCol_Button), GetColorU32(ImGuiCol_ButtonHovered), env.anim.hover), GetColorU32(ImGuiCol_ButtonActive), env.anim.held);

        GetWindowDrawList()->AddRectFilled(env.bb.Min, env.bb.Max, col, GImGui->Style.FrameRounding);
        GetWindowDrawList()->AddText(env.bb.GetCenter() - CalcTextSize(env.label, 0, 1) / 2, GetColorU32(ImGuiCol_TextButton), env.label, FindRenderedTextEnd(env.label));
        });
}

bool WidgetsManager::ColorEdit(const char* label, float col[4]) {
    SearchManager::get().additem(label, [=]() { ColorEdit(label, col); });

    float square_sz = 14;

    ImRect total_bb{ GetCurrentWindow()->DC.CursorPos, GetCurrentWindow()->DC.CursorPos + ImVec2{ CalcTextSize(label, 0, 1).x > 0 ? CalcItemWidth() : square_sz, square_sz } };
    ImRect bb{ total_bb.Max - ImVec2{ square_sz, square_sz }, total_bb.Max };

    bool value_changed = false;

    CompBuilder::get().ColorEdit(label, total_bb, bb, col, [&](CompBuilder::ColorEditEnv env) {
        GetWindowDrawList()->AddText({ total_bb.Min.x, total_bb.GetCenter().y - GImGui->FontSize / 2 }, GetColorU32(ImGuiCol_Text), env.label, FindRenderedTextEnd(env.label));

        GetWindowDrawList()->AddCircleFilled(bb.GetCenter(), square_sz / 2, ImColor{ col[0], col[1], col[2], GImGui->Style.Alpha }, 36);

        if (env.anim.open > 0.05f) {
            SetNextWindowPos({ bb.Min.x, bb.Max.y + 5 });
            PushStyleVar(ImGuiStyleVar_Alpha, env.anim.open);
            PushStyleVar(ImGuiStyleVar_ItemSpacing, { 10, 10 });
            PushStyleVar(ImGuiStyleVar_WindowRounding, GImGui->Style.FrameRounding);
            PushStyleVar(ImGuiStyleVar_WindowPadding, { 10, 10 });
            PushStyleColor(ImGuiCol_WindowBg, GetColorU32(ImGuiCol_FrameBg));
            Begin(label, 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
            {
                BringWindowToDisplayFront(GetCurrentWindow());
                BringWindowToFocusFront(GetCurrentWindow());

                if (!IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && IsMouseClicked(0) && !env.hovered) {
                    env.open = false;
                }

                ColorPickerManager::get().draw(env.label, col);
            }
            End();
            PopStyleColor();
            PopStyleVar(4);
        }
        });

    return value_changed;
}

bool WidgetsManager::Selectable(const char* label, bool selected, ImVec2 size) {
    ImRect bb{ GetCurrentWindow()->DC.CursorPos, GetCurrentWindow()->DC.CursorPos + CalcItemSize(size, GetWindowWidth(), GetFrameHeight()) };
    return CompBuilder::get().Selectable(label, selected, bb, [&](const CompBuilder::SelectableEnv& env) {
        ImColor col = col_anim(col_anim(GetColorU32(ImGuiCol_Text), GetColorU32(ImGuiCol_Text, 0.6f), env.anim.hover), GetColorU32(ImGuiCol_Scheme), env.anim.selected);

        GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, env.anim.selected));
        GetWindowDrawList()->AddText({ bb.Min.x + GImGui->Style.FramePadding.x, bb.GetCenter().y - GImGui->FontSize / 2 }, col, env.label, FindRenderedTextEnd(env.label));
        });
}

void WidgetsManager::Separator() {
    GetWindowDrawList()->AddRectFilled({ GetWindowPos().x, GetCurrentWindow()->DC.CursorPos.y }, { GetWindowPos().x + GetWindowWidth(), GetCurrentWindow()->DC.CursorPos.y + 1 }, GetColorU32(ImGuiCol_Separator));
    Dummy({ GetWindowWidth(), 1 });
}