#include "Hooks.hpp"
#include "manipulator.hpp"

Vector3 hooks::methods::EyePositionForPlayer_hk(SDK::BaseMountable* mount, SDK::BasePlayer* player, SDK::UnityEngine::Quaternion lookRot) {
    __try {
        if (player && player->userID()) {
            if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key)) {
                return mount->EyePositionForPlayer(player, lookRot) + current_manipulate_angle;
            }
        }
    }
    __except (true) {
    }

    return mount->EyePositionForPlayer(player, lookRot);
}