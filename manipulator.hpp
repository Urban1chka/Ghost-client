#pragma once
#include "Getter.hpp"
#include "server_side.hpp"
#include "Hooks.hpp"
#include "manipulator.hpp"
#include <vector>
#include <cmath>
#include <imgui.h>

inline Vector3 current_manipulate_angle = { 0, 0, 0 };
inline SDK::BasePlayer* manipulator_locked_target = nullptr;
inline float manipulator_scan_rotation = 0.0f;
inline bool g_in_camera = false;

struct ScanPoint {
    Vector3 world_pos;
    bool valid;
};

inline std::vector<ScanPoint> manipulator_scan_points;
inline std::vector<ScanPoint> manipulator_draw_points;

inline float manip_get_eye_height() {
    Vector3 eyeOff = SDK::PlayerEyes::EyeOffset();
    if (!eyeOff.Empty() && eyeOff.y > 0.1f) {
        return eyeOff.y;
    }
    return 1.5f;
}

inline bool manip_is_target_alive(SDK::BasePlayer* players) {
    if (!players) return false;

    for (const auto& player : entity_data::players_list) {
        if ((SDK::BasePlayer*)(player.pawn) == players) {
            if (players->lifeState() == LifeState::Dead) return false;
            if (players->health() <= 0.f) return false;
            return true;
        }
    }

    return false;
}

inline SDK::BasePlayer* manip_find_best_target() {
    if (!entity_data::local_player) return nullptr;

    matrix4x4 v_matrix = Get::view_matrix();
    if (!v_matrix.matrix) return nullptr;

    float max_angle = setting::combat::total_points;
    if (max_angle <= 0.0f) max_angle = 1.0f;

    SDK::BasePlayer* best = nullptr;

    for (const auto& player : entity_data::players_list) {
        auto* player_pawn = (SDK::BasePlayer*)(player.pawn);
        if (!player_pawn) continue;

        if (setting::combat::ignore_team_combat_type && Get::team(player_pawn, entity_data::local_player)) continue;
        if (setting::combat::ignore_npc_combat_type && player.is_npc) continue;
        if (setting::combat::ignore_sleepers_combat_type && Get::is_sleeper(player_pawn)) continue;

        auto* bone = Get::bone_transform(player_pawn, BoneList::head);
        if (!bone) continue;

        Vector3 head = Get::transform_pos(bone);
        if (head.Empty()) continue;

        Vector2 screen_pos{};
        if (!head.world_to_screen(screen_pos, v_matrix.matrix)) continue;

        best = player_pawn;
    }

    return best;
}

inline void find_manipulate_angle() {
    if (!setting::combat::enable_manipulated || !is_active(setting::combat::manipulated_key)) {
        current_manipulate_angle = { 0, 0, 0 };
        manipulator_scan_points.clear();
        manipulator_draw_points.clear();
        return;
    }

    manipulator_scan_points.clear();

    if (!entity_data::local_player || !entity_data::local_player->GetTransform() || !entity_data::local_player->eyes()) {
        current_manipulate_angle = { 0, 0, 0 };
        manipulator_draw_points.clear();
        return;
    }

    SDK::BasePlayer* target_player = manipulator_locked_target ? manipulator_locked_target : manip_find_best_target();
    if (!target_player) {
        current_manipulate_angle = { 0, 0, 0 };
        manipulator_draw_points.clear();
        return;
    }

    auto* t_bone = Get::bone_transform(target_player, BoneList::head);
    if (!t_bone) {
        current_manipulate_angle = { 0, 0, 0 };
        manipulator_draw_points.clear();
        return;
    }

    Vector3 target_pos = Get::transform_pos(t_bone);

    Vector3 re_p = entity_data::local_player->GetTransform()->GetPosition() +
        entity_data::local_player->GetTransform()->up() *
        (SDK::PlayerEyes::EyeOffset().y + entity_data::local_player->eyes()->viewOffset().y);

    if (SDK::UnityEngine::LineOfSight(re_p, target_pos)) {
        current_manipulate_angle = { 0, 0, 0 };
        manipulator_draw_points.clear();
        return;
    }

    float max_radius = setting::combat::manip_distance;
    if (max_radius < 0.1f) max_radius = 0.1f;

    float total_points = setting::combat::total_points;
    if (total_points < 1) total_points = 1;

    Vector3 choice = { 0, 0, 0 };
    const float golden_ratio = 1.61803398875f;

    for (float i = 0; i < total_points; ++i) {
        float u = i / total_points;
        float phi = acosf(1.0f - u);
        float theta = 2.0f * M_PI * i / golden_ratio;

        float x = sinf(phi) * cosf(theta);
        float y = cosf(phi) * 0.3f;
        float z = sinf(phi) * sinf(theta);

        Vector3 offset = Vector3(x * max_radius, y * max_radius, z * max_radius);
        Vector3 point = re_p + offset;

        ScanPoint sp;
        sp.world_pos = point;
        sp.valid = false;

        if (SDK::UnityEngine::LineOfSight(re_p, point) && SDK::UnityEngine::LineOfSight(target_pos, point)) {
            Vector3 test_eye = entity_data::local_player->eyes()->position() + offset;
            if (check_eye(test_eye)) {
                sp.valid = true;
                if (choice.Empty()) {
                    choice = offset;
                }
            }
        }

        manipulator_scan_points.push_back(sp);
    }

    current_manipulate_angle = choice;
    manipulator_draw_points = manipulator_scan_points;
}

inline void draw_manipulator_esp() {
    if (setting::combat::enable_manipulated && setting::combat::draw_manipulated && is_active(setting::combat::manipulated_key)) {
        if (manipulator_draw_points.empty()) return;

        matrix4x4 v_matrix = Get::view_matrix();
        if (!v_matrix.matrix) return;

        auto draw_list = ImGui::GetBackgroundDrawList();
        if (!draw_list) return;

        for (const auto& sp : manipulator_draw_points) {
            Vector2 screen_pos{};
            if (sp.world_pos.world_to_screen(screen_pos, v_matrix.matrix)) {
                ImU32 color = sp.valid ? IM_COL32(0, 255, 0, 220) : IM_COL32(255, 0, 0, 80);
                float radius = sp.valid ? 4.5f : 2.5f;
                draw_list->AddCircleFilled(ImVec2(screen_pos.x, screen_pos.y), radius, color);
            }
        }
    }
}

struct HitscanResult {
    bool found;
    Vector3 hit_point;
    float damage;
};

inline HitscanResult hitscan_from_eye(Vector3 target_pos) {
    HitscanResult result{};
    result.found = false;
    result.damage = 0.0f;

    if (!entity_data::local_player) return result;

    Vector3 eye_pos = entity_data::local_player->eyes()->position();
    Vector3 dir = (target_pos - eye_pos).Normalized();
    float dist = eye_pos.Distance(target_pos);

    if (dist > setting::combat::hitscan_range) return result;

    if (SDK::UnityEngine::LineOfSight(eye_pos, target_pos)) {
        result.found = true;
        result.hit_point = target_pos;
        result.damage = 1.0f;
        return result;
    }

    float max_radius = 1.0f;
    float scan_points = 12;

    for (float i = 0; i < scan_points; ++i) {
        float u = i / scan_points;
        float phi = acosf(1.0f - u);
        float theta = 2.0f * M_PI * i * 0.61803398875f;

        float x = sinf(phi) * cosf(theta);
        float y = sinf(phi) * sinf(theta) * 0.3f;
        float z = cosf(phi);

        Vector3 offset = Vector3(x, y, z) * max_radius;
        Vector3 test_point = eye_pos + offset;

        if (SDK::UnityEngine::LineOfSight(test_point, target_pos)) {
            if (SDK::UnityEngine::LineOfSight(eye_pos, test_point)) {
                result.found = true;
                result.hit_point = test_point;
                result.damage = 1.0f;
                return result;
            }
        }
    }

    return result;
}
