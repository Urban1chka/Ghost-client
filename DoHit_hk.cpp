#include "Hooks.hpp"

bool hooks::methods::DoHit_hk(SDK::Projectile* projectile, SDK::HitTest* hitTest, Vector3 point, Vector3 normal) {
	__try {
		if (setting::misc::wall_shot) {
			if (hitTest) {
				auto lol = hitTest->HitEntity();

				if (!lol) {
					return false;
				}

				auto classname = lol->GetClassName();
				if (classname &&
					!m_strcmp(classname, "BasePlayer") &&
					!m_strcmp(classname, "BaseAnimalNPC") &&
					!m_strcmp(classname, "Boar") &&
					!m_strcmp(classname, "Bear") &&
					!m_strcmp(classname, "BaseNPC") &&
					!m_strcmp(classname, "ScientistNPC") &&
					!m_strcmp(classname, "NPCPlayer") &&
					!m_strcmp(classname, "Wolf")) {
					return false;
				}
			}
		}
	}
	__except (true)
	{
	}
	return projectile->DoHit(hitTest, point, normal);
}
