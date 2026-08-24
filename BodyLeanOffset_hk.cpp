#include "Hooks.hpp"
#include "manipulator.hpp"

Vector3 hooks::methods::BodyLeanOffset_hk(SDK::PlayerEyes* a1) {
    __try {
        if (g_in_camera) {
            return a1->BodyLeanOffset();
        }
        if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key) && !current_manipulate_angle.Empty()) {
            return current_manipulate_angle;
        }
    }
    __except (true) {
    }

    return a1->BodyLeanOffset();
}
