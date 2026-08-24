#include "Hooks.hpp"

void hooks::methods::UpdateAmbient_hk(SDK::TOD_Sky* sky) {
    if (!sky) return;

    __try {
        sky->UpdateAmbient();
        if (entity_data::local_player) {
            auto stars = sky->Stars();

            if (setting::visuals::bright_night) {
                SDK::UnityEngine::Color night_color;
                night_color.r = setting::visuals::night_color[0];
                night_color.g = setting::visuals::night_color[1];
                night_color.b = setting::visuals::night_color[2];
                night_color.a = setting::visuals::night_color[3];

                sky->set_ambientMode(static_cast<int>(SDK::TOD_Sky::AmbientMode::Flat));
                sky->set_ambientIntensity(6.0f);
                sky->set_ambientLight(night_color);
            }
            if (setting::visuals::star_changer_enabled) {
                if (stars) {
                    stars->Size() = setting::visuals::star_size;
                    stars->Brightness() = setting::visuals::star_brightness;
                }
            }
            if (!setting::visuals::star_changer_enabled) {
                if (stars) {
                    stars->Size() = 1.0f;
                    stars->Brightness() = 1.0f;
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}