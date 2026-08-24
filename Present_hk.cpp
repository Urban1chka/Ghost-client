#include "Hooks.hpp"
#include "gui/gui.hpp"
#include "manipulator.hpp"
#include "IconManager.hpp"

void Visuals() {
    if (!entity_data::local_player) {
        if (!entity_data::players_list.empty()) {
            entity_data::players_list.clear();
        }

        return;
    }

    auto draw_list = ImGui::GetBackgroundDrawList();

    matrix4x4 view_matrix = Get::view_matrix();
    if (!view_matrix.matrix) return;

    float screen_width = (float)SDK::UnityEngine::Screen::GetWidth();
    float screen_height = (float)SDK::UnityEngine::Screen::GetHeight();

    auto get_u32_color = [](float color[3], float alpha = 1.0f) {
        return ImGui::ColorConvertFloat4ToU32({ color[0], color[1], color[2], alpha });
        };

    const ImU32 box_color = get_u32_color(setting::visuals::box_color);
    const ImU32 name_color = get_u32_color(setting::visuals::name_color);
    const ImU32 bullet_color = get_u32_color(setting::visuals::bullet_tracers_color);

    const ImU32 dist_color = get_u32_color(setting::visuals::distance_color);
    const ImU32 skeleton_color = get_u32_color(setting::visuals::skeleton_color);
    const ImU32 aim_target_color = get_u32_color(setting::combat::aim_target_color);

    const ImU32 npc_color = get_u32_color(setting::visuals::npc_color);
    const ImU32 team_color = get_u32_color(setting::visuals::team_color);
    const ImU32 sleepers_color = get_u32_color(setting::visuals::sleepers_color);

    ImU32 map_org;
    ImU32 box_org;
    ImU32 name_org;
    ImU32 dist_org;
    ImU32 skeleton_org;

    auto color_black = ImColor(0, 0, 0);
    auto color_green = ImColor(0, 255, 0);
    auto color_red = ImColor(255, 0, 0);

    if (setting::visuals::enable_esp) {
        for (const auto& player : entity_data::players_list) {
            if (!player.pawn) continue;

            SDK::BasePlayer* player_pawn = (SDK::BasePlayer*)player.pawn;
            if (!player_pawn) continue;

            float health = player_pawn->health();
            if (health < 0.0f || health > 1000.0f) continue;

            auto is_sleeper = Get::is_sleeper(player_pawn);
            auto is_team = Get::team(player_pawn, entity_data::local_player);

            if (setting::visuals::ignore_team_visuals_type && is_team) continue;
            if (setting::visuals::ignore_npc_visuals_type && player.is_npc) continue;
            if (setting::visuals::ignore_sleepers_visuals_type && is_sleeper) continue;

            ImColor box_org, name_org, dist_org, skeleton_org;
            if (is_team != 0) {
                box_org = team_color;
                name_org = team_color;
                dist_org = team_color;
                skeleton_org = team_color;
            }
            else if (is_sleeper != 0) {
                box_org = sleepers_color;
                name_org = sleepers_color;
                dist_org = sleepers_color;
                skeleton_org = sleepers_color;
            }
            else if (player.is_npc) {
                box_org = npc_color;
                name_org = npc_color;
                dist_org = npc_color;
                skeleton_org = npc_color;
            }
            else {
                box_org = box_color;
                name_org = name_color;
                dist_org = dist_color;
                skeleton_org = skeleton_color;
            }

            float min_x = FLT_MAX, min_y = FLT_MAX;
            float max_x = -FLT_MAX, max_y = -FLT_MAX;
            bool bone_visible = false;

            for (int bone_id : bounds_bones) {
                SDK::UnityEngine::Transform* bone = Get::bone_transform(player.pawn, bone_id);
                if (!bone) continue;

                Vector3 world_position = Get::transform_pos(bone);
                if (world_position.Empty()) continue;

                for (int i = 0; i < 8; i++) {
                    Vector3 corner = {
                        world_position.x + ((i & 1) ? 0.15f : -0.15f),
                        world_position.y + ((i & 2) ? 0.20f : -0.20f),
                        world_position.z + ((i & 4) ? 0.15f : -0.15f)
                    };

                    Vector2 screen_position;
                    if (!corner.world_to_screen(screen_position, view_matrix.matrix)) continue;

                    bone_visible = true;

                    min_x = min(min_x, screen_position.x);
                    max_x = max(max_x, screen_position.x);
                    min_y = min(min_y, screen_position.y);
                    max_y = max(max_y, screen_position.y);
                }
            }
            if (!bone_visible) continue;

            if (setting::visuals::box_esp) {
                draw_list->AddRect({ min_x, min_y }, { max_x, max_y }, box_org, 1.0f);
            }
            if (setting::visuals::name_esp) {
                SDK::System::String* displayName = player_pawn->displayName();

                if (displayName && displayName->len > 0 && displayName->buffer) {
                    char name_buffer[256] = { 0 };

                    int safe_len = (displayName->len > 120) ? 120 : displayName->len;

                    int converted_size = WideCharToMultiByte(
                        CP_UTF8,
                        0,
                        displayName->buffer,
                        safe_len,
                        name_buffer,
                        sizeof(name_buffer) - 1,
                        nullptr,
                        nullptr
                    );

                    if (converted_size > 0) {
                        name_buffer[converted_size] = '\0';

                        ImVec2 text_size = ImGui::CalcTextSize(name_buffer);

                        float text_x = min_x + (max_x - min_x) * 0.5f - text_size.x * 0.5f;
                        float text_y = min_y - text_size.y - 2.0f;

                        draw_list->AddText({ text_x, text_y }, name_org, name_buffer);
                    }
                }
            }

            if (setting::visuals::health_esp) {
                float clamped_health = std::clamp(health, 0.0f, 100.0f);
                float hp_pc = clamped_health / 100.0f;

                float bar_width = 3.0f;
                float bx = min_x - bar_width - 4.0f;

                float bar_height = max_y - min_y;
                draw_list->AddRectFilled({ bx - 1, min_y }, { bx + bar_width + 1, max_y }, ImColor(0, 0, 0, 255));

                ImColor health_color((int)(255 * (1.0f - hp_pc)), (int)(255 * hp_pc), 0, 255);
                draw_list->AddRectFilled({ bx, max_y - (bar_height * hp_pc) }, { bx + bar_width, max_y }, health_color);
            }
            if (setting::visuals::distance_esp) {
                SDK::UnityEngine::Transform* local_eyes = Get::bone_transform(entity_data::local_player, BoneList::head);
                SDK::UnityEngine::Transform* target_eyes = Get::bone_transform(player_pawn, BoneList::head);

                if (local_eyes && target_eyes) {
                    Vector3 local_pos = Get::transform_pos(local_eyes);
                    Vector3 target_pos = Get::transform_pos(target_eyes);

                    int distance = (int)(local_pos - target_pos).Length();
                    char dist_str[16];

                    sprintf_s(dist_str, "%dm", distance);

                    float dist_x = max_x + 3.0f;
                    float dist_y = min_y;

                    draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.85f, { dist_x, dist_y }, dist_org, dist_str);
                }
            }
            if (setting::visuals::skeleton_esp) {
                for (const auto& connection : skeleton_connections) {
                    SDK::UnityEngine::Transform* bone_start = Get::bone_transform(player.pawn, connection.first);
                    SDK::UnityEngine::Transform* bone_end = Get::bone_transform(player.pawn, connection.second);

                    if (!bone_start || !bone_end) continue;
                    Vector2 point_start, point_end;

                    if (Get::transform_pos(bone_start).world_to_screen(point_start, view_matrix.matrix) &&
                        Get::transform_pos(bone_end).world_to_screen(point_end, view_matrix.matrix)) {
                        draw_list->AddLine({ point_start.x, point_start.y }, { point_end.x, point_end.y }, skeleton_org, 1.0f);
                    }
                }
            }
            if (setting::visuals::inventory_esp || setting::menu::render_menu) {
                SDK::BasePlayer* nearest = nullptr;
                float best_dist = FLT_MAX;
                Vector2 center = { screen_width * 0.5f, screen_height * 0.5f };

                if (setting::visuals::inventory_esp) {
                    for (const auto& player : entity_data::players_list) {
                        if (!player.pawn) continue;

                        SDK::BasePlayer* pp = (SDK::BasePlayer*)player.pawn;
                        if (!pp) continue;

                        auto bone = Get::bone_transform(pp, BoneList::head);
                        if (!bone) continue;

                        Vector3 world = Get::transform_pos(bone);
                        if (world.Empty()) continue;

                        Vector2 screen;
                        if (!world.world_to_screen(screen, view_matrix.matrix)) continue;

                        float dx = screen.x - center.x;
                        float dy = screen.y - center.y;
                        float d = dx * dx + dy * dy;

                        if (d < best_dist) {
                            best_dist = d;
                            nearest = pp;
                        }
                    }
                }

                auto wear = nearest ? Get::wear_items(nearest) : std::vector<SDK::Item*>{};
                auto belt = nearest ? Get::belt_items(nearest) : std::vector<SDK::Item*>{};

                float icon_sz = setting::visuals::inventory_icon_size;
                if (icon_sz < 16.f) icon_sz = 16.f;
                if (icon_sz > 64.f) icon_sz = 64.f;

                float pad = 4.f;
                float slot_gap = 3.f;
                float wear_w = 7 * icon_sz + 6 * slot_gap + pad * 2;
                float belt_w = 6 * icon_sz + 5 * slot_gap + pad * 2;
                float total_w = max(wear_w, belt_w);
                float total_h = icon_sz * 2 + pad * 3 + slot_gap;

                bool movable = setting::menu::render_menu;
                ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground |
                    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;

                if (!movable)
                    flags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs;

                if (setting::visuals::reset_inventory_pos) {
                    ImGui::SetNextWindowPos(setting::visuals::inventory_pos, ImGuiCond_Always);
                    setting::visuals::reset_inventory_pos = false;
                }
                else {
                    ImGui::SetNextWindowPos(setting::visuals::inventory_pos, ImGuiCond_Once);
                }

                ImGui::SetNextWindowSize({ total_w, total_h });

                if (ImGui::Begin("##inventory_esp", nullptr, flags)) {
                    setting::visuals::inventory_pos = ImGui::GetWindowPos();

                    ImDrawList* d = ImGui::GetWindowDrawList();
                    ImVec2 p = ImGui::GetCursorScreenPos();
                    ImGuiStyle& style = ImGui::GetStyle();

                    ImU32 slot_bg = ImGui::GetColorU32(ImVec4(
                        style.Colors[ImGuiCol_FrameBg].x,
                        style.Colors[ImGuiCol_FrameBg].y,
                        style.Colors[ImGuiCol_FrameBg].z,
                        0.45f
                    ));

                    ImU32 border_col = ImGui::GetColorU32(ImVec4(
                        style.Colors[ImGuiCol_Border].x,
                        style.Colors[ImGuiCol_Border].y,
                        style.Colors[ImGuiCol_Border].z,
                        0.6f
                    ));

                    float start_x_wear = p.x + (total_w - wear_w) * 0.5f + pad;
                    float y_wear = p.y + pad;

                    for (int i = 0; i < 7; i++) {
                        float sx = start_x_wear + i * (icon_sz + slot_gap);
                        float sy = y_wear;

                        d->AddRectFilled({ sx, sy }, { sx + icon_sz, sy + icon_sz }, slot_bg, 3.f);
                        d->AddRect({ sx, sy }, { sx + icon_sz, sy + icon_sz }, border_col, 3.f, 0, 1.0f);

                        if (i < (int)wear.size() && wear[i]) {
                            std::string sn = Get::item_shortname(wear[i]);
                            if (!sn.empty()) {
                                ImTextureID tex = IconManager::Get().GetTextureID(sn);
                                if (tex)
                                    d->AddImage(tex, { sx + 1, sy + 1 }, { sx + icon_sz - 1, sy + icon_sz - 1 });
                            }
                        }
                    }

                    float start_x_belt = p.x + (total_w - belt_w) * 0.5f + pad;
                    float y_belt = y_wear + icon_sz + pad;

                    for (int i = 0; i < 6; i++) {
                        float sx = start_x_belt + i * (icon_sz + slot_gap);
                        float sy = y_belt;

                        d->AddRectFilled({ sx, sy }, { sx + icon_sz, sy + icon_sz }, slot_bg, 3.f);
                        d->AddRect({ sx, sy }, { sx + icon_sz, sy + icon_sz }, border_col, 3.f, 0, 1.0f);

                        if (i < (int)belt.size() && belt[i]) {
                            std::string sn = Get::item_shortname(belt[i]);
                            if (!sn.empty()) {
                                ImTextureID tex = IconManager::Get().GetTextureID(sn);
                                if (tex)
                                    d->AddImage(tex, { sx + 1, sy + 1 }, { sx + icon_sz - 1, sy + icon_sz - 1 });
                            }
                        }
                    }

                    ImGui::End();
                }
            }
        }
    }
    if (setting::visuals::resource_esp) {
        for (const auto& res : entity_data::resource_list) {
            if (!res.pawn) continue;

            bool should_draw = false;
            ImU32 ore_color = 0;

            if (res.is_hemp && setting::visuals::draw_hemp) {
                should_draw = true;
                ore_color = get_u32_color(setting::visuals::hemp_color);
            }
            else if (!res.is_hemp) {
                if (res.type == OreType::stone && setting::visuals::draw_stone) {
                    should_draw = true;
                    ore_color = get_u32_color(setting::visuals::stone_color);
                }
                else if (res.type == OreType::metal && setting::visuals::draw_metal) {
                    should_draw = true;
                    ore_color = get_u32_color(setting::visuals::metal_color);
                }
                else if (res.type == OreType::sulfur && setting::visuals::draw_sulfur) {
                    should_draw = true;
                    ore_color = get_u32_color(setting::visuals::sulfur_color);
                }
            }

            if (!should_draw) continue;

            Vector3 draw_pos = res.position;
            draw_pos.y += 1.0f;

            Vector2 screen_pos;
            if (!draw_pos.world_to_screen(screen_pos, view_matrix.matrix)) continue;

            const char* label = nullptr;
            if (res.is_hemp) {
                label = "Hemp";
            }
            else if (res.type == OreType::stone) {
                label = "Stone";
            }
            else if (res.type == OreType::metal) {
                label = "Metal";
            }
            else if (res.type == OreType::sulfur) {
                label = "Sulfur";
            }

            if (!label) continue;

            auto font = ImGui::GetFont();
            float text_height = 0.2;

            ImVec2 text_size = ImGui::CalcTextSize(label);

            float text_x = screen_pos.x - text_size.x * 0.5f;
            float text_y = screen_pos.y - text_height * 0.5f;

            draw_list->AddText({ text_x - 1, text_y }, IM_COL32(0, 0, 0, 200), label);
            draw_list->AddText({ text_x + 1, text_y }, IM_COL32(0, 0, 0, 200), label);
            draw_list->AddText({ text_x, text_y - 1 }, IM_COL32(0, 0, 0, 200), label);
            draw_list->AddText({ text_x, text_y + 1 }, IM_COL32(0, 0, 0, 200), label);
            draw_list->AddText({ text_x, text_y }, ore_color, label);
        }
    }

    if (setting::visuals::crosshair) {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

        auto* draw_list = ImGui::GetForegroundDrawList();
        float sz = setting::visuals::crosshair_size;
        float thin = 0.5f;

        ImU32 color;
        if (setting::visuals::rainbow_crosshair) {
            float speed = 0.25f;
            float hue = fmodf((float)ImGui::GetTime() * speed, 1.0f);
            ImVec4 col_rainbow;
            ImGui::ColorConvertHSVtoRGB(hue, 1.0f, 1.0f, col_rainbow.x, col_rainbow.y, col_rainbow.z);
            color = ImGui::ColorConvertFloat4ToU32(ImVec4(col_rainbow.x, col_rainbow.y, col_rainbow.z, 1.0f));
        }
        else {
            color = ImGui::ColorConvertFloat4ToU32(ImVec4(setting::visuals::crosshair_color[0], setting::visuals::crosshair_color[1], setting::visuals::crosshair_color[2], 1.0f));
        }

        switch (setting::visuals::crosshair_type) {
        case 0: {
            float sz = setting::visuals::crosshair_size;
            float thin = 1.0f;

            draw_list->AddLine(ImVec2(center.x - sz, center.y), ImVec2(center.x + sz + 1.0f, center.y), color, thin);
            draw_list->AddLine(ImVec2(center.x, center.y - sz), ImVec2(center.x, center.y + sz + 1.0f), color, thin);

            break;
        }
        case 1: {
            float sz = setting::visuals::crosshair_size;
            float radius = sz * 0.4f;

            draw_list->AddCircleFilled(center, radius, color, 64);
            break;
        }
        case 2: {
            float rot_speed = 2.0f;
            float angle = (float)ImGui::GetTime() * rot_speed;
            float cos_a = cosf(angle);
            float sin_a = sinf(angle);

            auto rotate = [&](ImVec2 p) {
                float x = p.x - center.x;
                float y = p.y - center.y;
                return ImVec2(center.x + x * cos_a - y * sin_a, center.y + x * sin_a + y * cos_a);
                };

            ImVec2 p1 = rotate(ImVec2(center.x, center.y - sz));
            ImVec2 p2 = rotate(ImVec2(center.x, center.y + sz));
            ImVec2 p3 = rotate(ImVec2(center.x - sz, center.y));
            ImVec2 p4 = rotate(ImVec2(center.x + sz, center.y));

            draw_list->AddLine(p1, p2, color, thin);
            draw_list->AddLine(p3, p4, color, thin);
            draw_list->AddLine(p1, rotate(ImVec2(center.x + sz, center.y - sz)), color, thin);
            draw_list->AddLine(p2, rotate(ImVec2(center.x - sz, center.y + sz)), color, thin);
            draw_list->AddLine(p3, rotate(ImVec2(center.x - sz, center.y - sz)), color, thin);
            draw_list->AddLine(p4, rotate(ImVec2(center.x + sz, center.y + sz)), color, thin);
        }
              break;
        }
    }
    if (setting::combat::draw_manipulated) {
        draw_manipulator_esp();
    }
    if (setting::visuals::draw_bullet_tracers) {
        const float now = SDK::UnityEngine::Time::GetTime();
        const float max_life = setting::visuals::max_bullet_lifetime;

        for (auto it = last_bulletPosition.begin(); it != last_bulletPosition.end();) {
            if (now - it->second.time > 1.2f)
                it = last_bulletPosition.erase(it);
            else
                ++it;
        }

        for (auto it = tracers.begin(); it != tracers.end();) {
            float age = now - it->time;
            if (age > max_life) {
                it = tracers.erase(it);
                continue;
            }

            Vector2 s, e;
            if (it->start.world_to_screen(s, view_matrix.matrix) &&
                it->end.world_to_screen(e, view_matrix.matrix)) {
                float alpha = 1.0f - (age / max_life);
                alpha *= alpha;

                ImU32 col = ImGui::ColorConvertFloat4ToU32({
                    setting::visuals::bullet_tracers_color[0],
                    setting::visuals::bullet_tracers_color[1],
                    setting::visuals::bullet_tracers_color[2],
                    alpha
                    });

                draw_list->AddLine(
                    ImVec2(s.x, s.y),
                    ImVec2(e.x, e.y),
                    col,
                    setting::visuals::bullets_tracers_thickness
                );
            }
            ++it;
        }

        if (tracers.size() > 350)
            tracers.erase(tracers.begin(), tracers.begin() + (tracers.size() - 350));
    }

    if (setting::combat::aim_enable && setting::combat::draw_aim_fov) {
        float current_fov = SDK::Graphics::_fov();
        float fov_radius = setting::combat::aim_fov * (90.0f / current_fov);

        draw_list->AddCircle({ screen_width / 2.0f, screen_height / 2.0f }, fov_radius, ImColor(setting::combat::aim_fov_color[0], setting::combat::aim_fov_color[1], setting::combat::aim_fov_color[2]), 120, 1.0f);
    }
    if (setting::misc::reload_indicator && entity_data::local_player) {
        float fraction = 0.0f;
        bool has_reload = false;

        auto active_item = Get::active_item(entity_data::local_player);
        if (!active_item) return;

        auto held_raw = active_item->heldEntity();
        if (!held_raw) return;

        auto* held = reinterpret_cast<SDK::BaseProjectile*>(held_raw);
        if (!held) return;

        if (held->HasReloadCooldown()) {
            float time_left = held->nextReloadTime() - SDK::UnityEngine::Time::GetTime();
            float time_full = held->CalculateCooldownTime(held->nextReloadTime(), held->reloadTime()) - SDK::UnityEngine::Time::GetTime();
            if (time_full > 0.0f)
                fraction = clamp(time_left / time_full, 0.0f, 1.0f);
            fraction = 1.0f - fraction;
            has_reload = true;
        }
        else if (!did_reload && time_since_last_shot > 0.0f && time_since_last_shot <= (held->reloadTime() - (held->reloadTime() / 10))) {
            float time_full = held->reloadTime() - (held->reloadTime() / 10);
            fraction = clamp(time_since_last_shot / time_full, 0.0f, 1.0f);
            has_reload = true;
        }
        bool show_indicator = has_reload || setting::menu::render_menu;
        if (show_indicator) {
            float bar_width = 120.0f;

            float bar_height = 6.0f;
            float rounding = 3.0f;

            ImGuiStyle& style = ImGui::GetStyle();
            ImU32 bg_col = ImGui::GetColorU32(style.Colors[ImGuiCol_FrameBg]);

            ImU32 accent_col = ImGui::GetColorU32(style.Colors[ImGuiCol_Scheme]);
            bool movable = setting::menu::render_menu;

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
            if (!movable) flags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs;

            if (setting::misc::reset_reload_indicator_pos) {
                ImGui::SetNextWindowPos(setting::misc::reload_indicator_pos, ImGuiCond_Always);
                setting::misc::reset_reload_indicator_pos = false;
            }
            ImGui::SetNextWindowSize({ bar_width + 4, bar_height + 4 });
            if (ImGui::Begin("##reload_indicator", nullptr, flags)) {

                setting::misc::reload_indicator_pos = ImGui::GetWindowPos();
                ImDrawList* d = ImGui::GetWindowDrawList();

                ImVec2 p = ImGui::GetCursorScreenPos();
                ImVec2 bar_p = ImVec2(p.x, p.y);

                d->AddRectFilled({ bar_p.x, bar_p.y }, { bar_p.x + bar_width, bar_p.y + bar_height }, bg_col, rounding);
                if (fraction > 0.0f) d->AddRectFilled({ bar_p.x, bar_p.y }, { bar_p.x + bar_width * fraction, bar_p.y + bar_height }, accent_col, rounding);

                ImGui::End();
            }
        }
    }
    if (setting::combat::aim_enable && is_active(setting::combat::aim_key)) {
        if (setting::combat::draw_aim_target && target_info.has_target) {
            Vector2 screen_pos;

            if (target_info.predicted_pos.world_to_screen(screen_pos, view_matrix.matrix)) {
                float point_size = 2.5f;

                ImU32 dot_color = get_u32_color(setting::combat::aim_target_color, 1.0f);
                draw_list->AddCircleFilled({ screen_pos.x, screen_pos.y }, point_size, dot_color);
            }
        }
    }
}

HRESULT hooks::methods::hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!oPresent) return S_OK;
    ImGuiIO& io = ImGui::GetIO();

    if (GetAsyncKeyState(VK_HOME) & 1 || GetAsyncKeyState(VK_INSERT) & 1) {
        setting::menu::render_menu = !setting::menu::render_menu;
    }

    if (!imgui_init) {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice))) {
            pDevice->GetImmediateContext(&pContext);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            window = sd.OutputWindow;

            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
            pBackBuffer->Release();

            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
            
            InitImGui();
            GUI::get().initialize(pDevice);

            imgui_init = true;
        }
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    intro::draw_intro();

    if (!intro::intro_active) {

        io.MouseDrawCursor = setting::menu::render_menu;
        if (setting::menu::render_menu) {
            ClipCursor(NULL);

            POINT m_pos;
            GetCursorPos(&m_pos);
            ScreenToClient(window, &m_pos);
            io.MousePos.x = (float)m_pos.x;
            io.MousePos.y = (float)m_pos.y;

            GUI::get().draw();
        }

        ui::bind_list();
        ui::watermark();
        Visuals();
    }

    ImGui::Render();
    pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);
}