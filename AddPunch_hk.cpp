#include "Hooks.hpp"

void hooks::methods::AddPunch_hk(SDK::HeldEntity* attackEntity, Vector3 amount, float duration) {
    Vector3 amount_new = amount;

    __try {
        if (entity_data::local_player && entity_data::local_player->input() && entity_data::local_player->input()->state() && entity_data::local_player->input()->state()->current()) {

            if (setting::misc::no_recoil) {
                amount_new = amount * 0.0f;
            }

            entity_data::local_player->input()->state()->current()->aimAngles() += (amount - amount_new);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    if (attackEntity) {
        attackEntity->AddPunch(amount_new, duration);
    }
}