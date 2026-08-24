#include "Hooks.hpp"
#include "manipulator.hpp"

void hooks::methods::DoFirstPersonCamera_hk(SDK::PlayerEyes* a1, SDK::UnityEngine::Component* cam) {
    if (!a1 || !cam) return;

    __try {
        if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key) && !current_manipulate_angle.Empty()) {
            g_in_camera = true; a1->DoFirstPersonCamera(cam); g_in_camera = false;

            Vector3 re_p = entity_data::local_player->GetTransform()->GetPosition() + entity_data::local_player->GetTransform()->up() * (SDK::PlayerEyes::EyeOffset().y + entity_data::local_player->eyes()->viewOffset().y);
            cam->GetTransform()->set_position(re_p);
        } 
        else {
            a1->DoFirstPersonCamera(cam);
        }
    }
    __except (true) {
    }
}
