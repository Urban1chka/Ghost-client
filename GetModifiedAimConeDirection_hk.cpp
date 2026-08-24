#include "Hooks.hpp"

Vector3 hooks::methods::GetModifiedAimConeDirection_hk(float aimCone, Vector3 inputVec, bool canAddCone) {
    __try {
        if (entity_data::local_player) {
            if (is_active(setting::combat::aim_key) && setting::combat::aim_enable) {
                if (setting::combat::use_pSilent) {
                    if (target_info.has_target) {
                        inputVec = target_info.aim_direction;
                    }
                }
            }

            if (setting::misc::no_spread) {
                aimCone *= 0.0f;
            }
        }
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    return SDK::BaseProjectile::GetModifiedAimConeDirection_(aimCone, inputVec, canAddCone);
}