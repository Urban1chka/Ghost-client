#pragma once
#include "lib/imgui/imgui.h"
#include <string>
#include <vector>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <filesystem>

struct HotkeyState {
    int key = 0;
    int mode = 0;
    bool enabled = false;
};

struct HotkeyEntry {
    std::string name;
    HotkeyState* state;
    bool* is_enabled;
};

inline std::vector<HotkeyEntry> hotkey_registry;

inline bool is_active(HotkeyState state) {
    if (state.mode == 2) return true;
    if (state.key == 0) return false;
    if (state.mode == 0) {
        return GetAsyncKeyState(state.key) & 0x8000;
    }
    if (state.mode == 1) {
        static bool pressed[256];
        static bool active[256];
        if (GetAsyncKeyState(state.key) & 0x8000) {
            if (!pressed[state.key]) {
                active[state.key] = !active[state.key];
                pressed[state.key] = true;
            }
        }
        else {
            pressed[state.key] = false;
        }
        return active[state.key];
    }

    return false;
}

namespace setting {
    namespace visuals {
        namespace chams {
            inline bool enabled = false;
            inline float players_visible_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

            inline int bundle_index = 0;
            inline char bundle_path[MAX_PATH] = "";
            inline const char* load_status = "";

        }
        namespace custommodel {
            inline bool enabled = false;
            inline char path[MAX_PATH] = "";

            inline uintptr_t mesh_cache = 0;
            inline bool loaded = false;

            inline const char* status = "no model";
            inline uintptr_t orig_mesh = 0;

            namespace rainbow {
                inline bool enabled = false;
                inline float opacity = 0.8f;
            }
        }

        inline bool enable_esp = false;

        inline bool box_esp = false;
        inline bool name_esp = false;
        inline bool health_esp = false;
        inline bool distance_esp = false;
        inline bool skeleton_esp = false;
        inline bool inventory_esp = false;

        inline ImVec2 inventory_pos = { 100.f, 100.f };
        inline float inventory_icon_size = 32.f;
        inline bool reset_inventory_pos = true;

        inline bool fov_changer = false;
        inline float fov_value = 90.0f;

        inline HotkeyState zoom_key;
        inline float zoom_value = 30.0f;
        inline bool zoom_enable = false;

        inline bool ignore_npc_visuals_type = false;
        inline bool ignore_team_visuals_type = false;
        inline bool ignore_sleepers_visuals_type = false;

        inline float box_color[3] = { 1.0f, 1.0f, 1.0f };
        inline float name_color[3] = { 1.0f, 1.0f, 1.0f };
        inline float distance_color[3] = { 1.0f, 1.0f, 1.0f };
        inline float skeleton_color[3] = { 1.0f, 1.0f, 1.0f };
        inline float npc_color[3] = { 0.0f, 0.0f, 1.0f };
        inline float team_color[3] = { 0.0f, 1.0f, 0.0f };
        inline float sleepers_color[3] = { 1.0f, 0.0f, 1.0f };

        inline bool draw_bullet_tracers = false;
        inline float max_bullet_lifetime = 0.5f;
        inline float bullets_tracers_thickness = 0.5f;
        inline float bullet_tracers_color[3] = { 1.0f, 1.0f, 1.0f };

        inline bool crosshair = false;
        inline int crosshair_type = 0;
        inline float crosshair_size = 2.0f;
        inline bool rainbow_crosshair = false;
        inline float crosshair_color[3] = { 1.0f, 1.0f, 1.0f };

        inline bool bright_night = false;
        inline float night_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        inline bool time_changer = false;
        inline float time_value = 12.0f;

        inline bool resource_esp = false;
        inline bool draw_stone = false;
        inline bool draw_sulfur = false;
        inline bool draw_metal = false;
        inline bool draw_hemp = false;
        inline float ore_distance = 100.0f;
        inline float stone_color[3] = { 1.0f, 1.0f, 1.0f };
        inline float sulfur_color[3] = { 1.0f, 1.0f, 0.0f };
        inline float metal_color[3] = { 0.5f, 0.5f, 0.5f };
        inline float hemp_color[3] = { 0.0f, 1.0f, 0.0f };

        inline bool star_changer_enabled = false;
        inline float star_size = 1.0f;
        inline float star_brightness = 1.0f;
    }

    namespace combat {
        inline bool aim_enable = false;
        inline bool use_pSilent = false;
        inline bool auto_fire = false;
        inline bool draw_aim_fov = false;
        inline bool draw_aim_target = false;

        inline bool visible_check_combat_type = false;
        inline bool ignore_npc_combat_type = false;
        inline bool ignore_team_combat_type = false;
        inline bool ignore_sleepers_combat_type = false;

        inline float aim_fov = 100.0f;
        inline float aim_smooth = 5.0f;
        inline int target_bone = 0;
        inline HotkeyState aim_key;

        inline float aim_target_color[3] = { 1.0f, 0.0f, 0.0f };
        inline float aim_fov_color[3] = { 1.0f, 1.0f, 1.0f };

        inline bool enable_manipulated;
        inline bool draw_manipulated;
        inline HotkeyState manipulated_key;
        inline float total_points = 50;
        inline float manip_distance = 1.3f;
        inline bool enable_hitscan = false;
        inline float hitscan_range = 100.0f;
    }

    namespace movement {
        inline bool enable_speedHack = false;
        inline bool enable_airStack = false;
        inline bool enable_tp_to_head = false;
        inline bool enable_spider = false;
        inline bool infinite_jump = false;
        inline bool no_mini_sprint = false;
        inline bool insta_pickup_player = false;

        inline bool can_wield_items = false;
        inline bool anti_aim = false;
        inline bool fast_loot = false;

        inline HotkeyState spider_key;
        inline HotkeyState airStack_key;
        inline HotkeyState tp_to_head_key;
        inline HotkeyState speed_key;

        inline float speed_value = 6.0f;
    }

    namespace misc {
        inline bool no_sway = false;
        inline bool no_recoil = false;
        inline bool no_spread = false;

        inline bool insta_eoka = false;
        inline bool wall_shot = false;
        inline bool bullet_tp = false;

        inline bool silent_reload = false;
        inline bool reload_indicator = false;
        inline bool reset_reload_indicator_pos = true;
        inline ImVec2 reload_indicator_pos = { 884, 906 };

        inline int sound_volume = 500;
    }

    namespace menu {
        inline bool exit = false;
        inline bool render_menu = true;
        inline bool hit_sound = false;

        inline bool reset_bind_pos = true;
        inline bool reset_watermark_pos = true;

        inline ImVec2 watermark_pos = { 911, 35 };
        inline ImVec2 bind_list_pos = { 15, 15 };
    }

    namespace config {
        inline std::filesystem::path path_segment_utf8(const std::string& s) {
            if (s.empty()) return {};
            int nw = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
            if (nw <= 0) return {};
            std::wstring w(static_cast<size_t>(nw - 1), L'\0');
            MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), nw);
            return std::filesystem::path(w);
        }

        inline std::filesystem::path get_path(const std::string& name) {
            return std::filesystem::temp_directory_path() / L"data" / path_segment_utf8(name + ".cfg");
        }

        inline std::string color_to_string4(const float col[4]) {
            return std::to_string(col[0]) + "," + std::to_string(col[1]) + "," + std::to_string(col[2]) + "," + std::to_string(col[3]);
        }

        inline std::string color_to_string(const float col[3]) {
            return std::to_string(col[0]) + "," + std::to_string(col[1]) + "," + std::to_string(col[2]);
        }

        inline void string_to_color(const std::string& val, float col[3]) {
            try {
                size_t p1 = val.find(',');
                size_t p2 = val.find(',', p1 + 1);
                if (p1 != std::string::npos && p2 != std::string::npos) {
                    col[0] = std::stof(val.substr(0, p1));
                    col[1] = std::stof(val.substr(p1 + 1, p2 - p1 - 1));
                    col[2] = std::stof(val.substr(p2 + 1));
                }
            }
            catch (...) {}
        }

        inline void string_to_color4(const std::string& val, float col[4]) {
            try {
                size_t p1 = val.find(",");
                size_t p2 = val.find(",", p1 + 1);
                size_t p3 = val.find(",", p2 + 1);
                if (p1 != std::string::npos && p2 != std::string::npos && p3 != std::string::npos) {
                    col[0] = std::stof(val.substr(0, p1));
                    col[1] = std::stof(val.substr(p1 + 1, p2 - p1 - 1));
                    col[2] = std::stof(val.substr(p2 + 1, p3 - p2 - 1));
                    col[3] = std::stof(val.substr(p3 + 1));
                }
            }
            catch (...) {}
        }

        inline HotkeyState hotkey_from_int(int val) {
            HotkeyState state;
            state.key = val & 0xFF;
            state.mode = (val >> 8) & 0xFF;
            state.enabled = (val >> 16) & 1;
            return state;
        }

        inline int hotkey_to_int(HotkeyState state) {
            return (state.key & 0xFF) | ((state.mode & 0xFF) << 8) | ((state.enabled ? 1 : 0) << 16);
        }

        inline void save(const std::string& name) {
            std::filesystem::path full_path = get_path(name);
            std::filesystem::create_directories(full_path.parent_path());
            std::ostringstream oss;

            oss << "[Chams]\n";
            oss << "enabled=" << visuals::chams::enabled << "\n";
            oss << "players_visible_color=" << color_to_string4(visuals::chams::players_visible_color) << "\n";
            oss << "bundle_index=" << visuals::chams::bundle_index << "\n";
            oss << "bundle_path=" << visuals::chams::bundle_path << "\n";

            oss << "[Visuals]\n";
            oss << "enable_esp=" << visuals::enable_esp << "\n";
            oss << "box_esp=" << visuals::box_esp << "\n";
            oss << "name_esp=" << visuals::name_esp << "\n";
            oss << "health_esp=" << visuals::health_esp << "\n";
            oss << "distance_esp=" << visuals::distance_esp << "\n";
            oss << "skeleton_esp=" << visuals::skeleton_esp << "\n";
            oss << "inventory_esp=" << visuals::inventory_esp << "\n";
            oss << "inventory_icon_size=" << visuals::inventory_icon_size << "\n";
            oss << "inventory_pos=" << visuals::inventory_pos.x << "," << visuals::inventory_pos.y << "\n";
            oss << "fov_changer=" << visuals::fov_changer << "\n";
            oss << "fov_value=" << visuals::fov_value << "\n";
            oss << "zoom_enable=" << visuals::zoom_enable << "\n";
            oss << "zoom_value=" << visuals::zoom_value << "\n";
            oss << "zoom_key=" << hotkey_to_int(visuals::zoom_key) << "\n";
            oss << "ignore_npc_visuals_type=" << visuals::ignore_npc_visuals_type << "\n";
            oss << "ignore_team_visuals_type=" << visuals::ignore_team_visuals_type << "\n";
            oss << "ignore_sleepers_visuals_type=" << visuals::ignore_sleepers_visuals_type << "\n";
            oss << "box_color=" << color_to_string(visuals::box_color) << "\n";
            oss << "name_color=" << color_to_string(visuals::name_color) << "\n";
            oss << "distance_color=" << color_to_string(visuals::distance_color) << "\n";
            oss << "skeleton_color=" << color_to_string(visuals::skeleton_color) << "\n";
            oss << "npc_color=" << color_to_string(visuals::npc_color) << "\n";
            oss << "team_color=" << color_to_string(visuals::team_color) << "\n";
            oss << "sleepers_color=" << color_to_string(visuals::sleepers_color) << "\n";
            oss << "draw_bullet_tracers=" << visuals::draw_bullet_tracers << "\n";
            oss << "max_bullet_lifetime=" << visuals::max_bullet_lifetime << "\n";
            oss << "bullets_tracers_thickness=" << visuals::bullets_tracers_thickness << "\n";
            oss << "bullet_tracers_color=" << color_to_string(visuals::bullet_tracers_color) << "\n";
            oss << "bright_night=" << visuals::bright_night << "\n";
            oss << "night_color=" << color_to_string4(visuals::night_color) << "\n";

            oss << "[Aim]\n";
            oss << "aim_enable=" << combat::aim_enable << "\n";
            oss << "use_pSilent=" << combat::use_pSilent << "\n";
            oss << "auto_fire=" << combat::auto_fire << "\n";
            oss << "aim_key=" << hotkey_to_int(combat::aim_key) << "\n";
            oss << "aim_fov=" << combat::aim_fov << "\n";
            oss << "aim_smooth=" << combat::aim_smooth << "\n";
            oss << "draw_aim_fov=" << combat::draw_aim_fov << "\n";
            oss << "draw_aim_target=" << combat::draw_aim_target << "\n";
            oss << "visible_check_combat_type=" << combat::visible_check_combat_type << "\n";
            oss << "ignore_npc_combat_type=" << combat::ignore_npc_combat_type << "\n";
            oss << "ignore_team_combat_type=" << combat::ignore_team_combat_type << "\n";
            oss << "ignore_sleepers_combat_type=" << combat::ignore_sleepers_combat_type << "\n";
            oss << "target_bone=" << combat::target_bone << "\n";
            oss << "fov_color=" << color_to_string(combat::aim_fov_color) << "\n";
            oss << "aim_target_color=" << color_to_string(combat::aim_target_color) << "\n";
            oss << "enable_manipulated=" << combat::enable_manipulated << "\n";
            oss << "draw_manipulated=" << combat::draw_manipulated << "\n";
            oss << "manipulated_key=" << hotkey_to_int(combat::manipulated_key) << "\n";
            oss << "total_points=" << combat::total_points << "\n";
            oss << "manip_distance=" << combat::manip_distance << "\n";
            oss << "enable_hitscan=" << combat::enable_hitscan << "\n";
            oss << "hitscan_range=" << combat::hitscan_range << "\n";

            oss << "[Movement]\n";
            oss << "enable_speedHack=" << movement::enable_speedHack << "\n";
            oss << "enable_airStack=" << movement::enable_airStack << "\n";
            oss << "enable_tp_to_head=" << movement::enable_tp_to_head << "\n";
            oss << "enable_spider=" << movement::enable_spider << "\n";
            oss << "infinite_jump=" << movement::infinite_jump << "\n";
            oss << "no_mini_sprint=" << movement::no_mini_sprint << "\n";
            oss << "insta_pickup_player=" << movement::insta_pickup_player << "\n";
            oss << "can_wield_items=" << movement::can_wield_items << "\n";
            oss << "anti_aim=" << movement::anti_aim << "\n";
            oss << "fast_loot=" << movement::fast_loot << "\n";
            oss << "spider_key=" << hotkey_to_int(movement::spider_key) << "\n";
            oss << "airStack_key=" << hotkey_to_int(movement::airStack_key) << "\n";
            oss << "tp_to_head_key=" << hotkey_to_int(movement::tp_to_head_key) << "\n";
            oss << "speed_key=" << hotkey_to_int(movement::speed_key) << "\n";
            oss << "speed_value=" << movement::speed_value << "\n";

            oss << "[World]\n";
            oss << "time_changer=" << visuals::time_changer << "\n";
            oss << "time_value=" << visuals::time_value << "\n";
            oss << "resource_esp=" << visuals::resource_esp << "\n";
            oss << "draw_stone=" << visuals::draw_stone << "\n";
            oss << "draw_sulfur=" << visuals::draw_sulfur << "\n";
            oss << "draw_metal=" << visuals::draw_metal << "\n";
            oss << "draw_hemp=" << visuals::draw_hemp << "\n";
            oss << "ore_distance=" << visuals::ore_distance << "\n";
            oss << "stone_color=" << color_to_string(visuals::stone_color) << "\n";
            oss << "sulfur_color=" << color_to_string(visuals::sulfur_color) << "\n";
            oss << "metal_color=" << color_to_string(visuals::metal_color) << "\n";
            oss << "hemp_color=" << color_to_string(visuals::hemp_color) << "\n";

            oss << "[Misc]\n";
            oss << "no_sway=" << misc::no_sway << "\n";
            oss << "no_recoil=" << misc::no_recoil << "\n";
            oss << "no_spread=" << misc::no_spread << "\n";
            oss << "insta_eoka=" << misc::insta_eoka << "\n";
            oss << "wall_shot=" << misc::wall_shot << "\n";
            oss << "bullet_tp=" << misc::bullet_tp << "\n";
            oss << "crosshair=" << visuals::crosshair << "\n";
            oss << "crosshair_type=" << visuals::crosshair_type << "\n";
            oss << "crosshair_size=" << visuals::crosshair_size << "\n";
            oss << "rainbow_crosshair=" << visuals::rainbow_crosshair << "\n";
            oss << "crosshair_color=" << color_to_string(visuals::crosshair_color) << "\n";
            oss << "star_changer_enabled=" << visuals::star_changer_enabled << "\n";
            oss << "star_changer_size=" << visuals::star_size << "\n";
            oss << "star_changer_brightness=" << visuals::star_brightness << "\n";
            oss << "sound_volume=" << misc::sound_volume << "\n";
            oss << "silent_reload=" << misc::silent_reload << "\n";
            oss << "reload_indicator=" << misc::reload_indicator << "\n";
            oss << "custommodel_enabled=" << visuals::custommodel::enabled << "\n";
            oss << "custommodel_path=" << visuals::custommodel::path << "\n";
            oss << "custommodel_rainbow=" << visuals::custommodel::rainbow::enabled << "\n";
            oss << "custommodel_opacity=" << visuals::custommodel::rainbow::opacity << "\n";

            oss << "exit=" << menu::exit << "\n";

            oss << "[Menu]\n";
            oss << "render_menu=" << menu::render_menu << "\n";
            oss << "hit_sound=" << menu::hit_sound << "\n";
            oss << "watermark_pos=" << menu::watermark_pos.x << "," << menu::watermark_pos.y << "\n";
            oss << "bind_list_pos=" << menu::bind_list_pos.x << "," << menu::bind_list_pos.y << "\n";
            oss << "reload_indicator_pos=" << misc::reload_indicator_pos.x << "," << misc::reload_indicator_pos.y << "\n";

            std::string body = oss.str();
            std::ofstream f(full_path, std::ios::binary | std::ios::trunc);
            if (!f.is_open()) return;

            static const unsigned char utf8_bom[] = { 0xEF, 0xBB, 0xBF };
            f.write(reinterpret_cast<const char*>(utf8_bom), 3);
            f.write(body.data(), static_cast<std::streamsize>(body.size()));
        }

        inline void apply_key(const std::string& key, const std::string& val) {
            try {
                if (key == "enabled") visuals::chams::enabled = std::stoi(val);
                else if (key == "players_visible_color") string_to_color4(val, visuals::chams::players_visible_color);
                else if (key == "bundle_index") visuals::chams::bundle_index = std::stoi(val);
                else if (key == "bundle_path") strncpy_s(visuals::chams::bundle_path, val.c_str(), _TRUNCATE);

                else if (key == "enable_esp") visuals::enable_esp = std::stoi(val);
                else if (key == "box_esp") visuals::box_esp = std::stoi(val);
                else if (key == "name_esp") visuals::name_esp = std::stoi(val);
                else if (key == "health_esp") visuals::health_esp = std::stoi(val);
                else if (key == "distance_esp") visuals::distance_esp = std::stoi(val);
                else if (key == "skeleton_esp") visuals::skeleton_esp = std::stoi(val);
                else if (key == "inventory_esp") visuals::inventory_esp = std::stoi(val);
                else if (key == "fov_changer") visuals::fov_changer = std::stoi(val);
                else if (key == "fov_value") visuals::fov_value = std::stof(val);
                else if (key == "zoom_enable") visuals::zoom_enable = std::stoi(val);
                else if (key == "zoom_value") visuals::zoom_value = std::stof(val);
                else if (key == "zoom_key") visuals::zoom_key = hotkey_from_int(std::stoi(val));
                else if (key == "ignore_npc_visuals_type") visuals::ignore_npc_visuals_type = std::stoi(val);
                else if (key == "ignore_team_visuals_type") visuals::ignore_team_visuals_type = std::stoi(val);
                else if (key == "ignore_sleepers_visuals_type") visuals::ignore_sleepers_visuals_type = std::stoi(val);
                else if (key == "box_color") string_to_color(val, visuals::box_color);
                else if (key == "name_color") string_to_color(val, visuals::name_color);
                else if (key == "distance_color") string_to_color(val, visuals::distance_color);
                else if (key == "skeleton_color") string_to_color(val, visuals::skeleton_color);
                else if (key == "npc_color") string_to_color(val, visuals::npc_color);
                else if (key == "team_color") string_to_color(val, visuals::team_color);
                else if (key == "sleepers_color") string_to_color(val, visuals::sleepers_color);
                else if (key == "draw_bullet_tracers") visuals::draw_bullet_tracers = std::stoi(val);
                else if (key == "max_bullet_lifetime") visuals::max_bullet_lifetime = std::stof(val);
                else if (key == "bullets_tracers_thickness") visuals::bullets_tracers_thickness = std::stof(val);
                else if (key == "bullet_tracers_color") string_to_color(val, visuals::bullet_tracers_color);
                else if (key == "bright_night") visuals::bright_night = std::stoi(val);
                else if (key == "night_color") string_to_color4(val, visuals::night_color);
                else if (key == "inventory_icon_size") visuals::inventory_icon_size = std::stof(val);
                else if (key == "inventory_pos") {
                    size_t p = val.find(',');
                    if (p != std::string::npos) {
                        visuals::inventory_pos.x = std::stof(val.substr(0, p));
                        visuals::inventory_pos.y = std::stof(val.substr(p + 1));
                        visuals::reset_inventory_pos = true;
                    }
                }
            }
            catch (...) {}
        }

        inline void apply_key2(const std::string& key, const std::string& val) {
            try {
                if (key == "aim_enable") combat::aim_enable = std::stoi(val);
                else if (key == "use_pSilent") combat::use_pSilent = std::stoi(val);
                else if (key == "auto_fire") combat::auto_fire = std::stoi(val);
                else if (key == "aim_key") combat::aim_key = hotkey_from_int(std::stoi(val));
                else if (key == "aim_fov") combat::aim_fov = std::stof(val);
                else if (key == "aim_smooth") combat::aim_smooth = std::stof(val);
                else if (key == "draw_aim_fov") combat::draw_aim_fov = std::stoi(val);
                else if (key == "draw_aim_target") combat::draw_aim_target = std::stoi(val);
                else if (key == "visible_check_combat_type") combat::visible_check_combat_type = std::stoi(val);
                else if (key == "ignore_npc_combat_type") combat::ignore_npc_combat_type = std::stoi(val);
                else if (key == "ignore_team_combat_type") combat::ignore_team_combat_type = std::stoi(val);
                else if (key == "ignore_sleepers_combat_type") combat::ignore_sleepers_combat_type = std::stoi(val);
                else if (key == "target_bone") combat::target_bone = std::stoi(val);
                else if (key == "fov_color") string_to_color(val, combat::aim_fov_color);
                else if (key == "aim_target_color") string_to_color(val, combat::aim_target_color);
                else if (key == "enable_manipulated") combat::enable_manipulated = std::stoi(val);
                else if (key == "draw_manipulated") combat::draw_manipulated = std::stoi(val);
                else if (key == "manipulated_key") combat::manipulated_key = hotkey_from_int(std::stoi(val));
                else if (key == "total_points") combat::total_points = std::stoi(val);
                else if (key == "manip_distance") combat::manip_distance = std::stof(val);
                else if (key == "enable_hitscan") combat::enable_hitscan = std::stoi(val);
                else if (key == "hitscan_range") combat::hitscan_range = std::stof(val);

                else if (key == "enable_speedHack") movement::enable_speedHack = std::stoi(val);
                else if (key == "enable_airStack") movement::enable_airStack = std::stoi(val);
                else if (key == "enable_tp_to_head") movement::enable_tp_to_head = std::stoi(val);
                else if (key == "enable_spider") movement::enable_spider = std::stoi(val);
                else if (key == "infinite_jump") movement::infinite_jump = std::stoi(val);
                else if (key == "no_mini_sprint") movement::no_mini_sprint = std::stoi(val);
                else if (key == "insta_pickup_player") movement::insta_pickup_player = std::stoi(val);
                else if (key == "can_wield_items") movement::can_wield_items = std::stoi(val);
                else if (key == "anti_aim") movement::anti_aim = std::stoi(val);
                else if (key == "fast_loot") movement::fast_loot = std::stoi(val);
                else if (key == "spider_key") movement::spider_key = hotkey_from_int(std::stoi(val));
                else if (key == "airStack_key") movement::airStack_key = hotkey_from_int(std::stoi(val));
                else if (key == "tp_to_head_key") movement::tp_to_head_key = hotkey_from_int(std::stoi(val));
                else if (key == "speed_key") movement::speed_key = hotkey_from_int(std::stoi(val));
                else if (key == "speed_value") movement::speed_value = std::stof(val);

                else if (key == "time_changer") visuals::time_changer = std::stoi(val);
                else if (key == "time_value") visuals::time_value = std::stof(val);
                else if (key == "resource_esp") visuals::resource_esp = std::stoi(val);
                else if (key == "draw_stone") visuals::draw_stone = std::stoi(val);
                else if (key == "draw_sulfur") visuals::draw_sulfur = std::stoi(val);
                else if (key == "draw_metal") visuals::draw_metal = std::stoi(val);
                else if (key == "draw_hemp") visuals::draw_hemp = std::stoi(val);
                else if (key == "ore_distance") visuals::ore_distance = std::stof(val);
                else if (key == "stone_color") string_to_color(val, visuals::stone_color);
                else if (key == "sulfur_color") string_to_color(val, visuals::sulfur_color);
                else if (key == "metal_color") string_to_color(val, visuals::metal_color);
                else if (key == "hemp_color") string_to_color(val, visuals::hemp_color);
            }
            catch (...) {}
        }

        inline void apply_key3(const std::string& key, const std::string& val) {
            try {
                if (key == "no_sway") misc::no_sway = std::stoi(val);
                else if (key == "no_recoil") misc::no_recoil = std::stoi(val);
                else if (key == "no_spread") misc::no_spread = std::stoi(val);
                else if (key == "insta_eoka") misc::insta_eoka = std::stoi(val);
                else if (key == "wall_shot") misc::wall_shot = std::stoi(val);
                else if (key == "bullet_tp") misc::bullet_tp = std::stoi(val);
                else if (key == "crosshair") visuals::crosshair = std::stoi(val);
                else if (key == "crosshair_type") visuals::crosshair_type = std::stoi(val);
                else if (key == "crosshair_size") visuals::crosshair_size = std::stof(val);
                else if (key == "rainbow_crosshair") visuals::rainbow_crosshair = std::stoi(val);
                else if (key == "crosshair_color") string_to_color(val, visuals::crosshair_color);

                else if (key == "sound_volume") misc::sound_volume = std::stoi(val);
                else if (key == "silent_reload") misc::silent_reload = std::stoi(val);
                else if (key == "reload_indicator") misc::reload_indicator = std::stoi(val);
                else if (key == "custommodel_enabled") visuals::custommodel::enabled = std::stoi(val);
                else if (key == "custommodel_path") strncpy_s(visuals::custommodel::path, val.c_str(), _TRUNCATE);
                else if (key == "custommodel_rainbow") visuals::custommodel::rainbow::enabled = std::stoi(val);
                else if (key == "custommodel_opacity") visuals::custommodel::rainbow::opacity = std::stof(val);
                else if (key == "star_changer_enabled") visuals::star_changer_enabled = std::stoi(val);
                else if (key == "star_changer_size") visuals::star_size = std::stof(val);
                else if (key == "star_changer_brightness") visuals::star_brightness = std::stof(val);
                else if (key == "exit") menu::exit = std::stoi(val);

                else if (key == "render_menu") menu::render_menu = std::stoi(val);
                else if (key == "hit_sound") menu::hit_sound = std::stoi(val);
                else if (key == "watermark_pos") {
                    size_t p = val.find(',');
                    if (p != std::string::npos) {
                        menu::watermark_pos.x = std::stof(val.substr(0, p));
                        menu::watermark_pos.y = std::stof(val.substr(p + 1));
                        menu::reset_watermark_pos = true;
                    }
                }
                else if (key == "bind_list_pos") {
                    size_t p = val.find(',');
                    if (p != std::string::npos) {
                        menu::bind_list_pos.x = std::stof(val.substr(0, p));
                        menu::bind_list_pos.y = std::stof(val.substr(p + 1));
                        menu::reset_bind_pos = true;
                    }
                }
                else if (key == "reload_indicator_pos") {
                    size_t p = val.find(',');
                    if (p != std::string::npos) {
                        misc::reload_indicator_pos.x = std::stof(val.substr(0, p));
                        misc::reload_indicator_pos.y = std::stof(val.substr(p + 1));
                        misc::reset_reload_indicator_pos = true;
                    }
                }
            }
            catch (...) {}
        }

        inline void load(const std::string& name) {
            std::filesystem::path full_path = get_path(name);
            std::ifstream f(full_path, std::ios::binary | std::ios::ate);
            if (!f.is_open()) return;

            const std::streamsize sz = f.tellg();
            f.seekg(0);
            std::string data;
            if (sz > 0) {
                data.resize(static_cast<size_t>(sz));
                f.read(data.data(), sz);
            }
            f.close();

            if (data.size() >= 3 &&
                (unsigned char)data[0] == 0xEF &&
                (unsigned char)data[1] == 0xBB &&
                (unsigned char)data[2] == 0xBF) {
                data.erase(0, 3);
            }

            std::istringstream ls(data);
            std::string line;
            while (std::getline(ls, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                size_t sep = line.find('=');
                if (sep == std::string::npos) continue;

                std::string key = line.substr(0, sep);
                std::string val = line.substr(sep + 1);
                apply_key(key, val);
                apply_key2(key, val);
                apply_key3(key, val);
            }
        }
    }
}
