#include "Hooks.hpp"

void hooks::methods::TryToMove_hk(SDK::ItemIcon* a1, void* method) {
    if (!a1) return;

    __try {
        a1->TryToMove();

        if (entity_data::local_player) {
            if (setting::movement::fast_loot) {
                if (a1->queuedForLooting()) {
                    a1->RunTimedAction();
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}