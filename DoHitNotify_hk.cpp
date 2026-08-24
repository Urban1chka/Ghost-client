#include "Hooks.hpp"
#pragma comment(lib, "winmm.lib") 

void hooks::methods::DoHitNotify_hk(SDK::BaseCombatEntity* entity, SDK::HitInfo* hitInfo, SDK::Projectile* projectile) {
	__try
	{
		if (entity->IsFrom("BasePlayer")) {
			
			if (setting::menu::hit_sound) {
				ui::play_sound(1);
			}

			return;
		}
	}
	__except (true) {
	}
	return entity->DoHitNotify(entity, hitInfo, projectile);
}