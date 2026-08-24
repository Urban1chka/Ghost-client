#pragma once

#include "lib/imgui/imgui.h"
#include "lib/imgui/imgui_internal.h"
#include "lib/imgui/imgui_impl_win32.h"
#include "lib/imgui/imgui_impl_dx11.h"

#include "includes.hpp"
#include "setting.hpp"
#include "../gui/managers/LangManager.hpp"

#include <map>
#include <array>
#include <thread>
#include <string>
#include <vector>
#include <mciapi.h>
#include <algorithm>
#include <xaudio2.h>
#include <windows.h>
#include <atomic>
#include <fstream>
#include <filesystem>

#include "sounds/click_sound.hpp"
#include "sounds/fatality_sound.hpp"
#include "gui/unicodes.hpp"
#include "gui/managers/FontManager.hpp"

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "Winmm.lib")

template<typename T>
static inline T Lerp(T a, T b, float t) { return (T)(a + (b - a) * t); }

inline std::string notification_text = "";
inline float notification_timer = 0.0f;
inline float notification_alpha = 0.0f;

inline std::string g_sound_path[2] = { "", "" };

namespace ui {
    inline void init_sound() {
        char exe_path[MAX_PATH];
        if (GetModuleFileNameA(NULL, exe_path, MAX_PATH)) {
            std::string path_str(exe_path);
            size_t last_slash = path_str.find_last_of("\\/");

            if (last_slash != std::string::npos) {
                std::string base_path = path_str.substr(0, last_slash + 1);

                g_sound_path[0] = base_path + "sound.wav";
                g_sound_path[1] = base_path + "fatality.wav";

                std::ofstream click_file(g_sound_path[0], std::ios::binary);
                if (click_file) {
                    click_file.write(reinterpret_cast<const char*>(clickData::click_bytes), clickData::click_size);
                    click_file.close();
                }

                std::ofstream fatality_file(g_sound_path[1], std::ios::binary);
                if (fatality_file) {
                    fatality_file.write(reinterpret_cast<const char*>(fatalitySound::fatalitySound_bytes), fatalitySound::fatalitySound_size);
                    fatality_file.close();
                }
            }
        }
    }
    inline void play_sound(int type) {
        if (type < 0 || type > 1 || g_sound_path[type].empty()) return;

        int volume = setting::misc::sound_volume;
        std::string target_path = g_sound_path[type];

        std::thread([volume, target_path]() {
            static std::atomic<int> sound_id{ 0 };
            std::string alias = "audio_" + std::to_string(sound_id++);

            std::string open_cmd = "open \"" + target_path + "\" type mpegvideo alias " + alias;
            std::string vol_cmd = "setaudio " + alias + " volume to " + std::to_string(volume);
            std::string play_cmd = "play " + alias + " wait";
            std::string close_cmd = "close " + alias;

            mciSendStringA(open_cmd.c_str(), NULL, 0, NULL);
            mciSendStringA(vol_cmd.c_str(), NULL, 0, NULL);
            mciSendStringA(play_cmd.c_str(), NULL, 0, NULL);
            mciSendStringA(close_cmd.c_str(), NULL, 0, NULL);
            }).detach();
    }

    inline const char* GetKeyNameLegacy(int vk) {
        if (vk == 0) return "None";
        if (vk >= 1 && vk <= 5) {
            const char* mouse_keys[] = { "M1", "M2", "M3", "M4", "M5" };
            return mouse_keys[vk - 1];
        }
        static char name[64] = { 0 };
        unsigned int scanCode = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);
        LONG lParam = scanCode << 16;
        if (vk >= 33 && vk <= 46) lParam |= 0x1000000;
        if (GetKeyNameTextA(lParam, name, sizeof(name)) > 0) return name;
        return "none";
    }

    inline void bind_list() {
        struct BindEntry {
            std::string mode_tag, name, key_tag;
        };
        std::vector<BindEntry> active_binds;

        float col_mode_w = 0.0f, col_name_w = 0.0f, col_key_w = 0.0f;

        for (const auto& entry : hotkey_registry) {
            if (entry.is_enabled && !*entry.is_enabled) continue;
            if (!is_active(*entry.state)) continue;

            std::string mode_tag = "[" + std::string(entry.state->mode == 2 ? LangManager::get().translate("ALWAYS") : (entry.state->mode == 1 ? LangManager::get().translate("TOGGLE") : LangManager::get().translate("HOLD"))) + "]";

            std::string name = entry.name;

            std::string raw_key;
            if (entry.state->mode == 2) {
                raw_key = "ALWAYS";
            }
            else {
                int k = entry.state->key;
                if (k == VK_LBUTTON) raw_key = "M1";
                else if (k == VK_RBUTTON) raw_key = "M2";
                else if (k == VK_MBUTTON) raw_key = "M3";
                else if (k == VK_XBUTTON1) raw_key = "M4";
                else if (k == VK_XBUTTON2) raw_key = "M5";
                else {
                    raw_key = GetKeyNameLegacy(k);
                    if (raw_key.empty()) raw_key = "UNKNOWN";
                }
            }

            std::transform(raw_key.begin(), raw_key.end(), raw_key.begin(), ::toupper);
            std::string key_tag = "[" + raw_key + "]";

            col_mode_w = max(col_mode_w, ImGui::CalcTextSize(mode_tag.c_str()).x);
            col_name_w = max(col_name_w, ImGui::CalcTextSize(name.c_str()).x);
            col_key_w = max(col_key_w, ImGui::CalcTextSize(key_tag.c_str()).x);

            active_binds.push_back({ mode_tag, name, key_tag });
        }

        if (!setting::menu::render_menu && active_binds.empty()) return;

        if (setting::menu::reset_bind_pos) {
            ImGui::SetNextWindowPos(setting::menu::bind_list_pos, ImGuiCond_Always);
            setting::menu::reset_bind_pos = false;
        }

        const float pad_x = 15.0f, pad_top = 8.0f, sep_w = ImGui::CalcTextSize(" ").x,
            header_h = 30.0f, row_h = 20.0f, row_pad = 4.0f;

        float content_w = col_mode_w + sep_w + col_name_w + sep_w + col_key_w;
        float box_width = max(160.0f, content_w + pad_x * 2.0f);
        float box_height = header_h + 1.0f +
            (active_binds.empty() ? 0.0f : (float)active_binds.size() * (row_h + row_pad) - row_pad + 12.0f) +
            pad_top;

        ImGui::SetNextWindowSize({ box_width, box_height });

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings;

        if (!setting::menu::render_menu) flags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

        if (!ImGui::Begin("##bind_list", nullptr, flags)) {
            ImGui::PopStyleVar();
            ImGui::End();
            return;
        }

        ImGui::PopStyleVar();

        setting::menu::bind_list_pos = ImGui::GetWindowPos();

        ImVec2 p = ImGui::GetWindowPos();
        ImDrawList* d = ImGui::GetWindowDrawList();
        ImGuiStyle& style = ImGui::GetStyle();

        d->AddRectFilled(p, { p.x + box_width, p.y + box_height }, ImGui::GetColorU32(style.Colors[ImGuiCol_ChildBg]), style.ChildRounding);
        d->AddRect(p, { p.x + box_width, p.y + box_height }, ImGui::GetColorU32(style.Colors[ImGuiCol_Border]), style.ChildRounding);

        float h1w = ImGui::CalcTextSize(LangManager::get().translate("Bind")).x;
        float h_off = (box_width - (h1w + ImGui::CalcTextSize(LangManager::get().translate(" List")).x)) * 0.5f;

        d->AddText({ p.x + h_off, p.y + (header_h - ImGui::GetTextLineHeight()) * 0.5f + 1.0f },
            ImGui::GetColorU32(style.Colors[ImGuiCol_ButtonActive]), LangManager::get().translate("Bind"));
        d->AddText({ p.x + h_off + h1w, p.y + (header_h - ImGui::GetTextLineHeight()) * 0.5f + 1.0f },
            ImGui::GetColorU32(ImGuiCol_Text), LangManager::get().translate(" List"));

        float row_y = p.y + header_h + pad_top + 2.0f;

        for (const auto& b : active_binds) {
            float rx = p.x + pad_x;

            d->AddText({ rx, row_y }, ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]), b.mode_tag.c_str());
            rx += col_mode_w + sep_w;

            d->AddText({ rx, row_y }, ImGui::GetColorU32(style.Colors[ImGuiCol_Text]), b.name.c_str());
            rx += col_name_w + sep_w;

            d->AddText({ rx, row_y }, ImGui::GetColorU32(style.Colors[ImGuiCol_ButtonActive]), b.key_tag.c_str());

            row_y += row_h + row_pad;
        }

        ImGui::End();
    }
    inline void watermark() {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
        if (setting::menu::reset_watermark_pos) {
            ImGui::SetNextWindowPos(setting::menu::watermark_pos, ImGuiCond_Always);
            setting::menu::reset_watermark_pos = false;
        }
        if (ImGui::Begin("##watermark", nullptr, flags)) {
            setting::menu::watermark_pos = ImGui::GetWindowPos();
            ImDrawList* d = ImGui::GetWindowDrawList();

            const char* t1 = "Ghost";
            const char* t2 = " Client";
            char fps_text[32];
            snprintf(fps_text, sizeof(fps_text), " | %.0f FPS", ImGui::GetIO().Framerate);

            ImGuiStyle& style = ImGui::GetStyle();
            ImU32 bg_col = ImGui::GetColorU32(style.Colors[ImGuiCol_ChildBg]);
            ImU32 border_col = ImGui::GetColorU32(style.Colors[ImGuiCol_Border]);
            ImU32 accent_col = ImGui::GetColorU32(style.Colors[ImGuiCol_ButtonActive]);
            ImU32 text_col = ImGui::GetColorU32(style.Colors[ImGuiCol_Text]);
            ImU32 text_muted = ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]);
            float rounding = style.ChildRounding;

            ImVec2 s1 = ImGui::CalcTextSize(t1);
            ImVec2 s2 = ImGui::CalcTextSize(t2);
            ImVec2 s3 = ImGui::CalcTextSize(fps_text);

            float pad_x = 10.0f;
            float pad_y = 6.0f;
            float box_w = s1.x + s2.x + s3.x + pad_x * 2.0f;
            float box_h = s1.y + pad_y * 2.0f;

            ImVec2 p = ImGui::GetCursorScreenPos();
            ImVec2 box_end = { p.x + box_w, p.y + box_h };

            ImGui::ItemSize({ box_w, box_h });

            d->AddRectFilled(p, box_end, bg_col, rounding);
            d->AddRect(p, box_end, border_col, rounding, 0, 1.0f);

            float text_y = p.y + (box_h - s1.y) * 0.5f;
            float cur_x = p.x + pad_x;

            d->AddText({ cur_x, text_y }, accent_col, t1);
            cur_x += s1.x;
            d->AddText({ cur_x, text_y }, text_col, t2);
            cur_x += s2.x;
            d->AddText({ cur_x, text_y }, text_muted, fps_text);

            ImGui::End();
        }
    }
}

namespace intro {
    inline bool intro_active = true;
    inline float intro_start = 0.0f;

    struct HookLogEntry {
        char text[128];
        ImColor color;
        bool initialized = false;
    };

    constexpr int MAX_LOGS = 16;
    inline HookLogEntry g_hook_logs[MAX_LOGS] = {};

    inline void SetHookLog(int index, const char* text, ImColor color) {
        if (index >= 0 && index < MAX_LOGS) {
            snprintf(g_hook_logs[index].text, sizeof(g_hook_logs[index].text), "%s", text);
            g_hook_logs[index].color = color;
            g_hook_logs[index].initialized = true;
        }
    }

    inline float EaseOutCubic(float x) { return 1.0f - powf(1.0f - x, 3.0f); }
    inline float EaseOutQuint(float x) { return 1.0f - powf(1.0f - x, 5.0f); }

    inline void draw_intro() {
        if (!intro_active) return;

        float time_now = ImGui::GetTime();
        if (intro_start == 0.0f) intro_start = time_now;

        float elapsed = time_now - intro_start;
        const float slot_duration = 0.55f;
        const float appear_time = 0.15f;
        const float hold_time = 0.30f;
        const float fade_time = 0.10f;

        int total_logs = 0;
        for (int i = 0; i < MAX_LOGS; ++i) {
            if (g_hook_logs[i].initialized)
                total_logs = i + 1;
        }

        if (total_logs == 0) return;

        float last_slot_end = total_logs * slot_duration;
        const float hold_at_100 = 0.65f;
        const float fade_out_dur = 0.45f;
        float intro_duration = last_slot_end + hold_at_100 + fade_out_dur;

        float alpha = 1.0f;

        if (elapsed < 0.30f) {
            alpha = EaseOutCubic(elapsed / 0.30f);
        }
        else if (elapsed > intro_duration - fade_out_dur) {
            alpha = 1.0f - EaseOutCubic((elapsed - (intro_duration - fade_out_dur)) / fade_out_dur);
        }

        alpha = std::clamp(alpha, 0.0f, 1.0f);

        if (alpha <= 0.001f) {
            if (elapsed >= intro_duration + 0.3f)   
            intro_active = false;
            return;
        }

        ImVec2 display = ImGui::GetIO().DisplaySize;
        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        if (!draw) return;

        ImVec4 accent = ImVec4(0.28f, 0.15f, 0.65f, 0.90f);
        ImVec4 accent_hi = ImVec4(0.43f, 0.27f, 0.75f, 1.00f);
        ImVec4 bg_main = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        ImVec4 bg_child = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        ImVec4 border = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        ImVec4 text_col = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
        ImVec4 green = ImVec4(0.18f, 0.80f, 0.44f, 1.00f);
        ImVec4 red = ImVec4(0.91f, 0.30f, 0.24f, 1.00f);
        ImVec4 yellow = ImVec4(0.95f, 0.77f, 0.06f, 1.00f);

        auto ToU32 = [](ImVec4 c, float a) -> ImU32 {
            return ImGui::ColorConvertFloat4ToU32(ImVec4(c.x, c.y, c.z, c.w * a));
        };

        const float panel_w = 320.0f;
        const float panel_h = 90.0f;
        const float radius = 4.0f;

        const float start_x = display.x - panel_w - 22.0f;
        const float start_y = 18.0f;

        ImVec2 p_min(start_x, start_y);
        ImVec2 p_max(start_x + panel_w, start_y + panel_h);

        draw->AddRectFilled(p_min, p_max, ToU32(bg_child, alpha), radius);
        draw->AddRect(p_min, p_max, ToU32(border, alpha), radius, 0, 1.0f);
        draw->AddRectFilled(ImVec2(p_min.x + 1.0f, p_min.y + 6.0f), ImVec2(p_min.x + 3.0f, p_max.y - 6.0f), ToU32(accent, alpha), 1.5f);

        draw->AddText(ImVec2(p_min.x + 14.0f, p_min.y + 8.0f), ToU32(accent_hi, alpha), "GHOST CLIENT");

        char hook_count_buf[32];
        int success = 0, errors = 0, missing = 0;
        for (int i = 0; i < total_logs; ++i) {
            if (!g_hook_logs[i].initialized) continue;
            if (g_hook_logs[i].text[0] == '[') {
                if (g_hook_logs[i].text[1] == '+') success++;
                else if (g_hook_logs[i].text[1] == '!') errors++;
                else if (g_hook_logs[i].text[1] == '-') missing++;
            }
        }
        snprintf(hook_count_buf, sizeof(hook_count_buf), "%d/%d", success, total_logs);
        ImVec2 count_size = ImGui::GetFont()->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, hook_count_buf);
        draw->AddText(ImGui::GetFont(), 11.0f, ImVec2(p_max.x - 10.0f - count_size.x, p_min.y + 10.0f), ToU32(text_col, alpha * 0.5f), hook_count_buf);

        int current_idx = -1;
        float item_local = 0.0f;
        bool is_last_hold = false;

        if (elapsed < last_slot_end) {
            current_idx = static_cast<int>(elapsed / slot_duration);
            if (current_idx >= total_logs) current_idx = total_logs - 1;
            item_local = elapsed - (current_idx * slot_duration);
        } 
        else {
            current_idx = total_logs - 1;
            item_local = slot_duration;
            is_last_hold = true;
        }

        if (current_idx >= 0 && current_idx < MAX_LOGS && g_hook_logs[current_idx].initialized) {
            float item_alpha = alpha;
            float slide = 0.0f;

            if (!is_last_hold) {
                float appear = std::clamp(item_local / appear_time, 0.0f, 1.0f);
                float disappear = 1.0f;
                if (item_local > appear_time + hold_time)
                    disappear = 1.0f - std::clamp((item_local - appear_time - hold_time) / fade_time, 0.0f, 1.0f);

                float item_anim = appear * disappear;
                float eased = EaseOutQuint(item_anim);
                item_alpha = alpha * eased;
                slide = (1.0f - eased) * 16.0f;
            }

            ImVec4 status_col;
            const char* label = g_hook_logs[current_idx].text;
            if (label[0] == '[' && label[1] == '+') {
                status_col = green;
                label += 4;
            } 
            else if (label[0] == '[' && label[1] == '!') {
                status_col = red;
                label += 4;
            } 
            else if (label[0] == '[' && label[1] == '-') {
                status_col = yellow;
                label += 4;
            } 
            else {
                status_col = ImVec4(text_col.x, text_col.y, text_col.z, 1.0f);
            }

            float text_y = p_min.y + 42.0f;
            ImVec2 text_pos(p_min.x + 16.0f + slide, text_y);

            draw->AddCircleFilled(ImVec2(text_pos.x, text_y + ImGui::GetFontSize() * 0.5f), 3.0f,
                ImGui::ColorConvertFloat4ToU32(ImVec4(status_col.x, status_col.y, status_col.z, status_col.w * item_alpha)));

            draw->AddText(ImVec2(text_pos.x + 12.0f, text_y),
                ImGui::ColorConvertFloat4ToU32(ImVec4(status_col.x, status_col.y, status_col.z, status_col.w * item_alpha)),
                label);

            ImVec4 dim_text = ImVec4(text_col.x, text_col.y, text_col.z, text_col.w * 0.35f * item_alpha);
            const char* hook_label = "hook";
            draw->AddText(ImGui::GetFont(), 10.0f,
                ImVec2(text_pos.x + 12.0f, text_y + ImGui::GetFontSize() + 1.0f),
                ImGui::ColorConvertFloat4ToU32(dim_text), hook_label);
        }

        float progress = 1.0f;
        if (elapsed < last_slot_end)
            progress = std::clamp(elapsed / last_slot_end, 0.0f, 1.0f);

        float bar_y = p_max.y - 6.0f;
        float bar_h = 2.0f;
        float bar_left = p_min.x + 8.0f;
        float bar_right = p_max.x - 8.0f;

        draw->AddRectFilled(ImVec2(bar_left, bar_y), ImVec2(bar_right, bar_y + bar_h),
            ToU32(border, alpha * 0.5f), 1.0f);

        float fill_w = (bar_right - bar_left) * progress;
        if (fill_w > 1.0f) {
            draw->AddRectFilled(ImVec2(bar_left, bar_y), ImVec2(bar_left + fill_w, bar_y + bar_h),
                ToU32(accent, alpha), 1.0f);

            draw->AddCircleFilled(ImVec2(bar_left + fill_w, bar_y + bar_h * 0.5f), 3.0f,
                ToU32(accent_hi, alpha));
        }

        float circle_r = 18.0f;
        ImVec2 circle_center(p_max.x - 26.0f, p_min.y + 22.0f);

        draw->AddCircleFilled(circle_center, circle_r, ToU32(bg_child, alpha));
        draw->AddCircle(circle_center, circle_r, ToU32(border, alpha), 0, 1.0f);

        float arc_end = progress * 6.283185f - 1.570796f;
        draw->PathClear();
        draw->PathArcTo(circle_center, circle_r - 1.0f, -1.570796f, arc_end, 28);
        draw->PathStroke(ToU32(accent, alpha), false, 2.0f);

        ImFont* icon_font = fonts[icons].get(12);
        if (icon_font) {
            const char* icon_text = i_info_hexagon;
            ImVec2 icon_size = icon_font->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, icon_text);
            ImVec2 icon_pos(circle_center.x - icon_size.x * 0.5f, circle_center.y - icon_size.y * 0.5f - 2.0f);
            draw->AddText(icon_font, 11.0f, icon_pos, ToU32(accent, alpha), icon_text);
        }

        int percent = static_cast<int>(progress * 100.0f + 0.5f);
        char pct_buf[16];

        snprintf(pct_buf, sizeof(pct_buf), "%d%%", percent);
        float pct_font_size = 7.0f;

        ImVec2 pct_size = ImGui::GetFont()->CalcTextSizeA(pct_font_size, FLT_MAX, 0.0f, pct_buf);
        ImVec2 pct_pos(circle_center.x - pct_size.x * 0.5f, circle_center.y + 5.0f);
        draw->AddText(ImGui::GetFont(), pct_font_size, pct_pos, ToU32(text_col, alpha * 0.6f), pct_buf);

        if (elapsed >= intro_duration + 0.4f) {
            intro_active = false;
        }
    }
}