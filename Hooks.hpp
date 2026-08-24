#pragma once
#include <intrin.h>
#include <algorithm>
#include <unordered_map>

#include "Dissector.hpp"
#include "custommodel.hpp"
#include "lib/kiero/minhook/include/MinHook.h"

#include "SDK.hpp"
#include "xorstr.hpp"
#include "ui_render.hpp"
#include "includes.hpp"
#include "server_side.hpp"
#include "chams.hpp"

inline float fixed_time_last_shot = 0.0f;
inline float time_since_last_shot = 0.0f;
inline bool did_reload = false;
inline bool just_shot = false;

struct TracerSegment {
    Vector3 start;
    Vector3 end;
    float time;
};
struct BulletPosInfo {
    Vector3 pos;
    float time;
};

struct WeaponInfo {
    float launch_speed = 375.f;
    float gravity_modifier = 1.0f;
    float drag = 1.0f;
};
struct TargetInfo {
    bool has_target = false;
    Vector3 predicted_pos = { 0, 0, 0 };
    Vector3 aim_direction = { 0, 0, 0 };
    SDK::BasePlayer* target_player = nullptr;
    SDK::UnityEngine::Transform* target_transform = nullptr;
};
struct BulletsTargetInfo {
    SDK::BasePlayer* bullet_target_player = nullptr;
    SDK::UnityEngine::Transform* bullet_target_transform = nullptr;
};

struct PlayerVelocityTrack {
    Vector3 last_position = { 0, 0, 0 };
    Vector3 calculated_velocity = { 0, 0, 0 };
    float last_update_time = 0.0f;
};

inline WeaponInfo weapon_info;
inline TargetInfo target_info;

inline BulletsTargetInfo bullets_target_info;
inline std::vector<TracerSegment> tracers;

inline std::unordered_map<uintptr_t, PlayerVelocityTrack> g_velocity_map;
inline std::unordered_map<void*, BulletPosInfo> last_bulletPosition;

inline ImColor green_color = ImColor(46, 204, 113);
inline ImColor red_color = ImColor(231, 76, 60);

inline bool il2cpp_attached = false;
inline bool imgui_init = false;

inline const BoneList bounds_bones[] = {
    BoneList::head, BoneList::l_foot, BoneList::r_foot,
    BoneList::l_hand, BoneList::r_hand, BoneList::pelvis
};
inline const std::vector<std::pair<int, int>> skeleton_connections = {
    {head, neck}, {neck, spine4}, {spine4, spine2}, {spine2, pelvis},
    {spine4, l_clavicle}, {l_clavicle, l_upperarm}, {l_upperarm, l_forearm}, {l_forearm, l_hand},
    {spine4, r_clavicle}, {r_clavicle, r_upperarm}, {r_upperarm, r_forearm}, {r_forearm, r_hand},
    {pelvis, l_hip}, {l_hip, l_knee}, {l_knee, l_foot},
    {pelvis, r_hip}, {r_hip, r_knee}, {r_knee, r_foot}
};

inline void reset(bool force_all = false) {
    auto movement = entity_data::local_player->movement();
    auto walk_movement = (SDK::PlayerWalkMovement*)movement;

    if (!setting::visuals::fov_changer && !is_active(setting::visuals::zoom_key) || force_all) {
        SDK::Graphics::_fov() = 90.0f;
    }
    if (!is_active(setting::movement::airStack_key) || force_all) {
        walk_movement->gravityMultiplier() = 2.5f;
        walk_movement->gravityMultiplierSwimming() = 1.0f;
    }
    if (!setting::visuals::time_changer || force_all) {
        SDK::Admin::admintime() = -1;
    }
    if (!setting::visuals::custommodel::enabled || force_all) {
        custommodel::revert();
    }
    if (!setting::visuals::chams::enabled || force_all) {
        chams::restore_all();
    }
}

namespace hookengine {
    inline bool hook(void* target, void** original, void* detour) {
        if (!target || !detour) return false;

        if (MH_CreateHook(target, detour, original) != MH_OK) return false;
        return MH_EnableHook(target) == MH_OK;
    }
    inline bool unhook(void* target) {
        if (!target) return false;

        MH_DisableHook(target);
        return MH_RemoveHook(target) == MH_OK;
    }
}
namespace hooks {
    bool init();
    void shutdown();
    void InitImGui();

    namespace methods {
        HRESULT hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
        LRESULT WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        Vector3 GetModifiedAimConeDirection_hk(float aimCone, Vector3 inputVec, bool canAddCone);
        void DoHitNotify_hk(SDK::BaseCombatEntity* entity, SDK::HitInfo* hitInfo, SDK::Projectile* projectile);

        void SendClientTick_hk(SDK::BasePlayer* player);

        Vector3 BodyLeanOffset_hk(SDK::PlayerEyes* a1);
        void DoFirstPersonCamera_hk(SDK::PlayerEyes* a1, SDK::UnityEngine::Component* cam);
        Vector3 EyePositionForPlayer_hk(SDK::BaseMountable* mount, SDK::BasePlayer* player, SDK::UnityEngine::Quaternion lookRot);

        bool DoHit_hk(SDK::Projectile* projectile, SDK::HitTest* hitTest, Vector3 point, Vector3 normal);
        void AddPunch_hk(SDK::HeldEntity* attackEntity, Vector3 amount, float duration);
        void DoAttack_hk(SDK::FlintStrikeWeapon* weapon_info);

        void DoMovement_hk(SDK::Projectile* projectile, float deltaTime);
        void UpdateVelocity_hk(SDK::PlayerWalkMovement* movement);
        void ClientInput_hk(SDK::BasePlayer* local_player, uintptr_t state);
        void ProjectileUpdate_hk(SDK::Projectile* projectile);
        void TryToMove_hk(SDK::ItemIcon* a1, void* method);
        void UpdateAmbient_hk(SDK::TOD_Sky* sky);
    }
}