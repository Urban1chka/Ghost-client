#include "Hooks.hpp"

void hooks::methods::SendClientTick_hk(SDK::BasePlayer* player) {
    if (player && entity_data::local_player != 0 && player->userID() == entity_data::local_player->userID()) {
        __try {
            if (setting::movement::anti_aim) {
                if (player->input() && player->input()->state() && player->input()->state()->current()) {
                    player->input()->state()->current()->aimAngles() = Vector3(-999, rand() % 999, rand() % 999);
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    if (SDK::BasePlayer::SendClientTick_) {
        SDK::BasePlayer::SendClientTick_(player);
    }
}