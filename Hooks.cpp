#include "Hooks.hpp"
#include "IconManager.hpp"

#define SetHook(cls_name, method_name, args_count, original_ptr, hook_func) \
    { \
        void* method_ptr = find_method("", cls_name, method_name, args_count); \
        if (method_ptr) { \
            if (hookengine::hook( \
                method_ptr, \
                reinterpret_cast<void**>(&(original_ptr)), \
                reinterpret_cast<void*>(hook_func) \
            )) { \
                char buf[128]; \
                snprintf(buf, sizeof(buf), "[+]  %s", #method_name); \
                intro::SetHookLog(hook_index++, buf, ImColor(46, 204, 113)); \
            } else { \
                char buf[128]; \
                snprintf(buf, sizeof(buf), "[!]  %s", #method_name); \
                intro::SetHookLog(hook_index++, buf, ImColor(231, 76, 60)); \
            } \
        } else { \
            char buf[128]; \
            snprintf(buf, sizeof(buf), "[-]  %s", #method_name); \
            intro::SetHookLog(hook_index++, buf, ImColor(241, 196, 15)); \
        } \
    }

bool hooks::init() {
    if (MH_Initialize() != MH_OK) return false;
    int hook_index = 0;

    if (!il2cpp_attached) {
        Dissector::Attach();
        il2cpp_attached = true;
    }

    auto find_method = [](const char* ns, const char* cls, const char* method, int args = -1) -> void* {
        auto* klass = Dissector::FindClass(ns, cls);
        if (!klass) klass = Dissector::FindClass("", cls);
        if (!klass) return nullptr;

        auto* m = Dissector::FindMethod(klass, method, args);
        if (!m) return nullptr;

        return reinterpret_cast<void*>(m->methodPtr);
    };

    SetHook("PlayerWalkMovement", "UpdateVelocity", 0, SDK::PlayerWalkMovement::UpdateVelocity_, hooks::methods::UpdateVelocity_hk);
    SetHook("Projectile", "Update", 0, SDK::Projectile::Update_, hooks::methods::ProjectileUpdate_hk);
    SetHook("BasePlayer", "ClientInput", 1, SDK::BasePlayer::ClientInput_, hooks::methods::ClientInput_hk);
    SetHook("TOD_Sky", "UpdateAmbient", 0, SDK::TOD_Sky::UpdateAmbient_, hooks::methods::UpdateAmbient_hk);
    SetHook("AimConeUtil", "GetModifiedAimConeDirection", 3, SDK::BaseProjectile::GetModifiedAimConeDirection_, hooks::methods::GetModifiedAimConeDirection_hk);
    SetHook("HeldEntity", "AddPunch", 2, SDK::HeldEntity::AddPunch_, hooks::methods::AddPunch_hk);
    SetHook("FlintStrikeWeapon", "DoAttack", 0, SDK::FlintStrikeWeapon::DoAttack_, hooks::methods::DoAttack_hk);
    SetHook("ItemIcon", "TryToMove", 0, SDK::ItemIcon::TryToMove_, hooks::methods::TryToMove_hk);
    SetHook("Projectile", "DoHit", 3, SDK::Projectile::DoHit_, hooks::methods::DoHit_hk);
    SetHook("BasePlayer", "SendClientTick", 0, SDK::BasePlayer::SendClientTick_, hooks::methods::SendClientTick_hk);
    SetHook("BaseCombatEntity", "DoHitNotify", 1, SDK::BaseCombatEntity::DoHitNotify_, hooks::methods::DoHitNotify_hk);
    SetHook("Projectile", "DoMovement", 1, SDK::Projectile::DoMovement_, hooks::methods::DoMovement_hk);

    SetHook("PlayerEyes", "get_BodyLeanOffset", 0, SDK::PlayerEyes::BodyLeanOffset_, hooks::methods::BodyLeanOffset_hk);
    SetHook("PlayerEyes", "DoFirstPersonCamera", 1, SDK::PlayerEyes::DoFirstPersonCamera_, hooks::methods::DoFirstPersonCamera_hk);
    SetHook("BaseMountable", "EyePositionForPlayer", 2, SDK::BaseMountable::EyePositionForPlayer_, hooks::methods::EyePositionForPlayer_hk);
#undef SetHook

    return true;
}

void hooks::shutdown() {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    kiero::shutdown();
}
void hooks::InitImGui() {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;

    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 14.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    io.Fonts->Build();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(pDevice, pContext);

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    IconManager::Get().Initialize(pDevice);
}
