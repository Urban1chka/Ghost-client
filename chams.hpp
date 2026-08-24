#pragma once
#define _HAS_STD_BYTE 0
#define NOMINMAX
#include "SDK.hpp"
#include "setting.hpp"
#include "custommodel.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <Windows.h>
#include <algorithm>
#include <fstream>
#include <cstring>

namespace chams {
    struct PlayerChamsData {
        uintptr_t renderer = 0;
        uintptr_t original_material = 0;
    };

    struct BundleInfo {
        std::string name;
        std::string path;
    };

    inline std::unordered_map<uintptr_t, std::vector<PlayerChamsData>> saved_materials;

    inline uintptr_t chams_mat = 0;
    inline uintptr_t loaded_bundle = 0;
    inline bool initialized = false;
    inline float last_init_check = 0.0f;
    inline char load_status[256] = {};
    inline std::vector<BundleInfo> bundle_list;
    inline std::string last_scan_path;
    inline bool scan_done = false;

    inline bool is_valid(uintptr_t p) {
        return p > 0x10000 && p < 0x7FFFFFFFFFFF;
    }

    inline float clamp01(float v) {
        if (v < 0.0f) return 0.0f;
        if (v > 1.0f) return 1.0f;
        return v;
    }

    inline int prop_id(const char* name) {
        if (!name) return 0;
        static std::unordered_map<std::string, int> cache;
        auto it = cache.find(name);
        if (it != cache.end()) return it->second;

        uintptr_t s = custommodel::str_new(name);
        if (!is_valid(s)) return 0;

        int id = SDK::UnityEngine::SafeExecution::Execute<int>(custommodel::fn_Shader_PropertyToID(), 0, s);
        if (id != 0) cache[name] = id;
        return id;
    }

    inline void set_int(uintptr_t mat, int id, int val) {
        if (!is_valid(mat) || !id) return;
        static auto fn = custommodel::getMethod("Material", "SetInt", 2);
        if (fn) SDK::UnityEngine::SafeExecution::Execute<void*>(fn, (void*)0, mat, (uintptr_t)id, (uintptr_t)val);
    }

    inline void set_float(uintptr_t mat, int id, float val) {
        if (!is_valid(mat) || !id) return;
        static auto fn = custommodel::getMethod("Material", "SetFloat", 2);
        if (fn) SDK::UnityEngine::SafeExecution::Execute<void*>(fn, (void*)0, mat, (uintptr_t)id, val);
    }

    inline void set_color(uintptr_t mat, custommodel::UColor col) {
        if (!is_valid(mat)) return;
        auto fn = custommodel::fn_Material_set_color();
        if (fn) SDK::UnityEngine::SafeExecution::Execute<void*>(fn, (void*)0, mat, &col);
    }

    inline void set_color_named(uintptr_t mat, const char* prop, custommodel::UColor col) {
        if (!is_valid(mat) || !prop) return;
        uintptr_t str = custommodel::str_new(prop);
        if (!is_valid(str)) return;
        static auto fn = custommodel::getMethod("Material", "SetColor", 2);
        if (fn) SDK::UnityEngine::SafeExecution::Execute<void*>(fn, (void*)0, mat, str, &col);
    }

    inline void set_render_queue(uintptr_t mat, int queue) {
        if (!is_valid(mat)) return;
        static auto fn = custommodel::getMethod("Material", "set_renderQueue", 1);
        if (fn) SDK::UnityEngine::SafeExecution::Execute<void*>(fn, (void*)0, mat, (uintptr_t)queue);
    }

    inline void set_renderer_material(uintptr_t renderer, uintptr_t mat) {
        if (!is_valid(renderer) || !is_valid(mat)) return;
        if (custommodel::fn_Renderer_SetMaterial())
            SDK::UnityEngine::SafeExecution::Execute<void*>(custommodel::fn_Renderer_SetMaterial(), (void*)0, renderer, mat);
    }

    inline uintptr_t get_renderer_material(uintptr_t renderer) {
        if (!is_valid(renderer)) return 0;
        if (custommodel::fn_Renderer_GetSharedMaterial())
            return SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(custommodel::fn_Renderer_GetSharedMaterial(), (uintptr_t)0, renderer);
        return 0;
    }

    inline uintptr_t fn_AssetBundle_LoadFromFile() {
        static uintptr_t v = 0;
        if (!v) {
            v = custommodel::getMethod("AssetBundle", "LoadFromFile", 1, "UnityEngine");
            if (!v) v = custommodel::getMethod("AssetBundle", "LoadFromFile", 3, "UnityEngine");
        }
        return v;
    }

    inline uintptr_t fn_AssetBundle_LoadAllAssets() {
        static uintptr_t v = 0;
        if (!v) {
            v = custommodel::getMethod("AssetBundle", "LoadAllAssets", 0, "UnityEngine");
            if (!v) v = custommodel::getMethod("AssetBundle", "LoadAllAssets", 1, "UnityEngine");
        }
        return v;
    }

    inline uintptr_t fn_AssetBundle_Unload() {
        static uintptr_t v = 0;
        if (!v) v = custommodel::getMethod("AssetBundle", "Unload", 1, "UnityEngine");
        return v;
    }

    inline uintptr_t fn_Object_GetType() {
        static uintptr_t v = 0;
        if (!v) v = custommodel::getMethod("Object", "GetType", 0, "System");
        return v;
    }

    inline uintptr_t fn_Type_get_Name() {
        static uintptr_t v = 0;
        if (!v) v = custommodel::getMethod("Type", "get_Name", 0, "System");
        return v;
    }

    inline std::string il2cpp_str_to_std(uintptr_t str) {
        if (!is_valid(str)) return {};
        int len = *(int*)(str + 0x10);
        if (len <= 0 || len > 512) return {};
        const wchar_t* chars = (const wchar_t*)(str + 0x14);
        std::string out;
        out.reserve(len);
        for (int i = 0; i < len; i++) {
            wchar_t c = chars[i];
            out.push_back(c < 128 ? (char)c : '?');
        }
        return out;
    }

    inline uintptr_t create_fallback_mat(int ztest_value = 8) {
        uintptr_t sh = custommodel::findShader("Hidden/Internal-Colored");
        if (!is_valid(sh)) sh = custommodel::findShader("GUI/Text Shader");
        if (!is_valid(sh)) sh = custommodel::findShader("Unlit/Color");
        if (!is_valid(sh)) sh = custommodel::findShader("UI/Default");
        if (!is_valid(sh)) sh = custommodel::findShader("Standard");
        if (!is_valid(sh)) return 0;

        uintptr_t mat = is_valid(custommodel::k_Material()) ? custommodel::obj_new(custommodel::k_Material()) : 0;
        if (!is_valid(mat)) return 0;
        if (custommodel::fn_Material_ctor_Shader())
            SDK::UnityEngine::SafeExecution::Execute<void*>(custommodel::fn_Material_ctor_Shader(), (void*)0, mat, sh);
        if (!is_valid(mat)) return 0;

        set_int(mat, prop_id("_ZWrite"), 0);
        set_int(mat, prop_id("_ZTest"), ztest_value);
        set_int(mat, prop_id("_Cull"), 0);
        set_render_queue(mat, 5000);
        return mat;
    }

    inline void scan_bundles(bool force = false) {
        char path_buf[MAX_PATH] = {};
        if (!setting::visuals::chams::bundle_path[0]) {
            char mod[MAX_PATH] = {};
            GetModuleFileNameA(nullptr, mod, MAX_PATH);
            std::string dir(mod);
            size_t slash = dir.find_last_of("\\/");
            if (slash != std::string::npos) dir = dir.substr(0, slash + 1);
            dir += "lib\\bundles";
            strncpy_s(setting::visuals::chams::bundle_path, dir.c_str(), _TRUNCATE);
        }
        strncpy_s(path_buf, setting::visuals::chams::bundle_path, _TRUNCATE);

        std::string cur(path_buf);
        if (!force && scan_done && cur == last_scan_path)
            return;

        bundle_list.clear();
        last_scan_path = cur;
        scan_done = true;

        try {
            namespace fs = std::filesystem;
            if (!fs::exists(path_buf) || !fs::is_directory(path_buf)) {
                snprintf(load_status, sizeof(load_status), "folder not found");
                return;
            }

            for (const auto& entry : fs::directory_iterator(path_buf)) {
                if (!entry.is_regular_file()) continue;

                BundleInfo bi;
                bi.path = entry.path().string();
                bi.name = entry.path().filename().string();
                bundle_list.push_back(std::move(bi));
            }

            std::sort(bundle_list.begin(), bundle_list.end(),
                [](const BundleInfo& a, const BundleInfo& b) { return a.name < b.name; });

            if (bundle_list.empty()) {
                snprintf(load_status, sizeof(load_status), "no bundles found");
                return;
            }
            snprintf(load_status, sizeof(load_status), "%zu files found", bundle_list.size());
        }
        catch (...) {
            snprintf(load_status, sizeof(load_status), "scan error");
        }
    }

    inline void unload_bundle() {
        if (is_valid(loaded_bundle) && fn_AssetBundle_Unload()) {
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_AssetBundle_Unload(), (void*)0, loaded_bundle, (uintptr_t)1);
        }
        loaded_bundle = 0;
        chams_mat = 0;
    }

    inline bool load_selected_bundle() {
        custommodel::ensureThreadAttached();

        if (bundle_list.empty()) {
            scan_bundles(true);
            if (bundle_list.empty()) {
                snprintf(load_status, sizeof(load_status), "no bundles");
                return false;
            }
        }

        int idx = setting::visuals::chams::bundle_index;
        if (idx < 0 || idx >= (int)bundle_list.size()) {
            idx = 0;
            setting::visuals::chams::bundle_index = 0;
        }

        const auto& bi = bundle_list[idx];

        std::error_code ec;
        auto filesize = std::filesystem::file_size(bi.path, ec);
        if (ec || filesize == 0) {
            snprintf(load_status, sizeof(load_status), "file empty");
            chams_mat = create_fallback_mat(8);
            return is_valid(chams_mat);
        }

        snprintf(load_status, sizeof(load_status), "loading %s...", bi.name.c_str());

        unload_bundle();

        uintptr_t loadFn = fn_AssetBundle_LoadFromFile();
        if (!loadFn) {
            chams_mat = create_fallback_mat(8);
            snprintf(load_status, sizeof(load_status), "LoadFromFile missing");
            return is_valid(chams_mat);
        }

        uintptr_t pathStr = custommodel::str_new(bi.path.c_str());
        if (!is_valid(pathStr)) return false;

        uintptr_t bundle = SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(loadFn, (uintptr_t)0, pathStr);
        if (!is_valid(bundle)) {
            chams_mat = create_fallback_mat(8);
            snprintf(load_status, sizeof(load_status), "load failed – fallback used");
            return is_valid(chams_mat);
        }

        loaded_bundle = bundle;

        uintptr_t matType = custommodel::type_from_name("UnityEngine.Material, UnityEngine.CoreModule");
        if (!is_valid(matType)) matType = custommodel::type_from_name("UnityEngine.Material");

        uintptr_t shaderType = custommodel::type_from_name("UnityEngine.Shader, UnityEngine.CoreModule");
        if (!is_valid(shaderType)) shaderType = custommodel::type_from_name("UnityEngine.Shader");

        uintptr_t loadAllT = custommodel::getMethod("AssetBundle", "LoadAllAssets", 1, "UnityEngine");
        uintptr_t loadAll = fn_AssetBundle_LoadAllAssets();

        uintptr_t firstMat = 0;

        if (loadAllT && is_valid(matType)) {
            uintptr_t arr = SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(loadAllT, (uintptr_t)0, bundle, matType);
            if (is_valid(arr) && custommodel::arr_len(arr) > 0) {
                firstMat = custommodel::Read<uintptr_t>(arr + 0x20);
            }
        }

        if (!is_valid(firstMat) && loadAllT && is_valid(shaderType)) {
            uintptr_t arr = SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(loadAllT, (uintptr_t)0, bundle, shaderType);
            if (is_valid(arr) && custommodel::arr_len(arr) > 0) {
                uintptr_t sh = custommodel::Read<uintptr_t>(arr + 0x20);
                if (is_valid(sh)) {
                    uintptr_t newMat = is_valid(custommodel::k_Material()) ? custommodel::obj_new(custommodel::k_Material()) : 0;
                    if (is_valid(newMat) && custommodel::fn_Material_ctor_Shader()) {
                        SDK::UnityEngine::SafeExecution::Execute<void*>(custommodel::fn_Material_ctor_Shader(), (void*)0, newMat, sh);
                        if (is_valid(newMat)) firstMat = newMat;
                    }
                }
            }
        }

        if (!is_valid(firstMat) && loadAll) {
            uintptr_t arr = SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(loadAll, (uintptr_t)0, bundle);
            if (is_valid(arr)) {
                uintptr_t n = custommodel::arr_len(arr);
                if (n > 256) n = 256;
                for (uintptr_t i = 0; i < n; i++) {
                    uintptr_t obj = custommodel::Read<uintptr_t>(arr + 0x20 + i * 8);
                    if (!is_valid(obj)) continue;

                    std::string tname;
                    if (fn_Object_GetType() && fn_Type_get_Name()) {
                        uintptr_t ty = SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(fn_Object_GetType(), (uintptr_t)0, obj);
                        if (is_valid(ty)) {
                            uintptr_t nameStr = SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(fn_Type_get_Name(), (uintptr_t)0, ty);
                            tname = il2cpp_str_to_std(nameStr);
                        }
                    }

                    if (tname.find("Material") != std::string::npos) {
                        firstMat = obj;
                        break;
                    }
                    else if (tname.find("Shader") != std::string::npos) {
                        uintptr_t newMat = is_valid(custommodel::k_Material()) ? custommodel::obj_new(custommodel::k_Material()) : 0;
                        if (is_valid(newMat) && custommodel::fn_Material_ctor_Shader()) {
                            SDK::UnityEngine::SafeExecution::Execute<void*>(custommodel::fn_Material_ctor_Shader(), (void*)0, newMat, obj);
                            if (is_valid(newMat)) {
                                firstMat = newMat;
                                break;
                            }
                        }
                    }
                    else if (!firstMat && i == 0) {
                        firstMat = obj;
                    }
                }
            }
        }

        if (is_valid(firstMat)) {
            chams_mat = firstMat;
            set_int(chams_mat, prop_id("_ZTest"), 8);
            set_int(chams_mat, prop_id("_ZWrite"), 0);
            set_render_queue(chams_mat, 5000);

            snprintf(load_status, sizeof(load_status), "OK: %s", bi.name.c_str());
            return true;
        }

        chams_mat = create_fallback_mat(8);
        snprintf(load_status, sizeof(load_status), "no Material/Shader – fallback");
        return is_valid(chams_mat);
    }

    inline void init() {
        if (initialized) return;
        custommodel::ensureThreadAttached();

        scan_bundles(true);
        if (!bundle_list.empty() && setting::visuals::chams::bundle_index >= 0) {
            load_selected_bundle();
            initialized = is_valid(chams_mat);
            return;
        }

        chams_mat = create_fallback_mat(8);
        snprintf(load_status, sizeof(load_status), "fallback material");
        initialized = is_valid(chams_mat);
    }

    inline void restore_all() {
        for (auto& [key, mats] : saved_materials) {
            for (auto& entry : mats) {
                if (is_valid(entry.renderer) && is_valid(entry.original_material)) {
                    set_renderer_material(entry.renderer, entry.original_material);
                }
            }
        }
        saved_materials.clear();
    }

    inline void apply_player(SDK::BasePlayer* player) {
        if (!player || !is_valid(chams_mat)) return;

        auto pm = player->playerModel();
        if (!pm) return;
        auto mm = pm->_multiMesh();
        if (!mm) return;
        auto renderers = mm->Renderers();
        if (!renderers) return;

        int size = renderers->GetSize();
        if (size <= 0 || size > 64) return;

        uintptr_t player_key = (uintptr_t)player;

        for (int i = 0; i < size; i++) {
            auto* renderer = renderers->GetArray(i);
            if (!renderer) continue;
            uintptr_t renderer_key = (uintptr_t)renderer;

            bool already_saved = false;
            if (saved_materials.count(player_key)) {
                for (auto& entry : saved_materials[player_key]) {
                    if (entry.renderer == renderer_key) {
                        already_saved = true;
                        break;
                    }
                }
            }
            if (!already_saved) {
                PlayerChamsData data;
                data.renderer = renderer_key;
                data.original_material = get_renderer_material(renderer_key);
                saved_materials[player_key].push_back(data);
            }

            if (get_renderer_material(renderer_key) != chams_mat) {
                set_renderer_material(renderer_key, chams_mat);
            }
        }
    }

    inline void tick() {
        if (!entity_data::local_player) return;

        if (!initialized) {
            float now = SDK::UnityEngine::Time::GetTime();
            if (now - last_init_check < 1.5f) return;
            last_init_check = now;
            init();
            return;
        }

        if (!setting::visuals::chams::enabled) {
            if (!saved_materials.empty())
                restore_all();
            return;
        }

        if (!is_valid(chams_mat)) {
            if (!bundle_list.empty()) load_selected_bundle();
            if (bundle_list.empty()) chams_mat = create_fallback_mat(8);
            if (!is_valid(chams_mat)) return;
        }

        if (setting::visuals::chams::players_visible_color) {
            float* col = setting::visuals::chams::players_visible_color;
            float r = clamp01(col[0]);
            float g = clamp01(col[1]);
            float b = clamp01(col[2]);
            float a = clamp01(col[3] > 0.01f ? col[3] : 1.0f);

            custommodel::UColor c{ r, g, b, a };
            set_color(chams_mat, c);
            set_color_named(chams_mat, "_Color", c);
            set_color_named(chams_mat, "_BaseColor", c);
            set_color_named(chams_mat, "_TintColor", c);
            set_color_named(chams_mat, "_EmissionColor", c);
            set_color_named(chams_mat, "_ColorVisible", c);
            set_color_named(chams_mat, "_FlatColor", c);
        }

        std::unordered_map<uintptr_t, bool> active_players;

        for (const auto& player : entity_data::players_list) {
            auto* player_pawn = (SDK::BasePlayer*)player.pawn;
            if (!player_pawn) continue;

            if (setting::visuals::ignore_npc_visuals_type && player.is_npc) continue;

            if (setting::visuals::ignore_team_visuals_type) {
                auto* lp = entity_data::local_player;
                if (lp) {
                    auto playerTeam = player_pawn->currentTeam();
                    auto localTeam = lp->currentTeam();
                    if (localTeam != 0 && localTeam == playerTeam) continue;
                }
            }

            if (setting::visuals::ignore_sleepers_visuals_type) {
                if (player_pawn->HasPlayerFlag(SDK::BasePlayer::PlayerFlags::Sleeping)) continue;
            }

            if (player_pawn == entity_data::local_player) continue;

            active_players[(uintptr_t)player_pawn] = true;
            apply_player(player_pawn);
        }

        std::vector<uintptr_t> to_remove;

        for (auto& [key, mats] : saved_materials) {
            if (!active_players.count(key)) {
                for (auto& entry : mats) {
                    if (is_valid(entry.renderer) && is_valid(entry.original_material))
                        set_renderer_material(entry.renderer, entry.original_material);
                }
                to_remove.push_back(key);
            }
        }
        for (auto key : to_remove) {
            saved_materials.erase(key);
        }
    }
}