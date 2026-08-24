#pragma once
#include "includes.hpp"
#include "UnityEngine.hpp"
#include "System.hpp"
#include "SDK.hpp"

namespace Get {
	matrix4x4 view_matrix();
	Vector3 player_velocity(SDK::BasePlayer* player_pawn);

	SDK::UnityEngine::Transform* bone_transform(SDK::BaseEntity* player_pawn, int index);
	Vector3 transform_pos(SDK::UnityEngine::Transform* transform);

	SDK::Item* active_item(SDK::BasePlayer* player_pawn);
	std::string item_shortname(SDK::Item* item);

	std::vector<SDK::Item*> belt_items(SDK::BasePlayer* player = nullptr);
	std::vector<SDK::Item*> wear_items(SDK::BasePlayer* player = nullptr);

	bool team(SDK::BasePlayer* player_pawn, SDK::BasePlayer* local_player);
	bool is_sleeper(SDK::BasePlayer* player_pawn);

	void entity_loop();
	bool is_npc(SDK::BasePlayer* player);
};