#include "Hooks.hpp"

void hooks::methods::DoAttack_hk(SDK::FlintStrikeWeapon* weapon_info) {
    if (!weapon_info) return;

    __try {
        if (entity_data::local_player) {
            if (setting::misc::insta_eoka) {
                weapon_info->_didSparkThisFrame() = true;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        if (entity_data::local_player) {
            if (!setting::misc::insta_eoka) {
                weapon_info->_didSparkThisFrame() = false;
            }
        }
    }

    weapon_info->DoAttack();
}
