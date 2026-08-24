#include "Getter.hpp"

matrix4x4 Get::view_matrix() {
    auto camera = SDK::UnityEngine::Camera::get_main();
    if (!camera) return {};

    auto camera_addr = *(uintptr_t*)((uintptr_t)camera + 0x10);
    if (!camera_addr) return {};

    auto view_matrix = *(matrix4x4*)(camera_addr + 0x2E4);
    return view_matrix;
}
Vector3 Get::player_velocity(SDK::BasePlayer* player) {
    if (!player) return { 0, 0, 0 };

    if (player == entity_data::local_player) {
        if (!player->movement()) return { 0, 0, 0 };
        auto walk_movement = (SDK::PlayerWalkMovement*)player->movement();
        if (!walk_movement) return { 0, 0, 0 };

        auto velocity = walk_movement->TargetMovement();
        if (velocity.Length() < 0.1f) {
            velocity = walk_movement->velocity();
        }

        return velocity;
    }

    auto base_trans = player->GetTransform();
    if (!base_trans) return { 0, 0, 0 };

    auto current_pos = base_trans->GetPosition();
    if (current_pos.Empty()) return { 0, 0, 0 };

    auto player_ptr = reinterpret_cast<uintptr_t>(player);
    auto& track = g_velocity_map[player_ptr];

    float current_time = SDK::UnityEngine::Time::GetTime();
    float delta_time = current_time - track.last_update_time;

    if (track.last_position.Length() > 0.1f) {
        if (delta_time > 0.0001f && delta_time < 0.5f) {
            Vector3 raw_velocity = (current_pos - track.last_position) / delta_time;

            if (raw_velocity.Length() < 60.0f) {
                track.calculated_velocity = track.calculated_velocity * 0.3f + raw_velocity * 0.7f;
            }
        }
    }

    track.last_position = current_pos;
    track.last_update_time = current_time;

    return track.calculated_velocity;
}

SDK::UnityEngine::Transform* Get::bone_transform(SDK::BaseEntity* player_pawn, int index) {
    if (!player_pawn) return nullptr;

    auto model = player_pawn->model();
    if (!model) return nullptr;

    auto bone_array = model->boneTransforms();
    if (!bone_array) return nullptr;

    int bone_count = bone_array->ArraySize();
    if (index < 0 || index >= bone_count) return nullptr;

    return bone_array->GetArray(index);
}
Vector3 Get::transform_pos(SDK::UnityEngine::Transform* transform) {
    if (!transform) return Vector3{ 0, 0, 0 };
    return transform->GetPosition();
}

bool Get::team(SDK::BasePlayer* player_pawn, SDK::BasePlayer* local_player) {
    if (!player_pawn || !local_player) return false;

    auto playerTeam = player_pawn->currentTeam();
    auto localTeam = local_player->currentTeam();

    return (localTeam != 0 && localTeam == playerTeam);
}
bool Get::is_sleeper(SDK::BasePlayer* player_pawn) {
    int flags = static_cast<int>(player_pawn->playerFlags());

    return (flags & static_cast<int>(SDK::BasePlayer::PlayerFlags::Sleeping)) != 0;
}

SDK::Item* Get::active_item(SDK::BasePlayer* player) {
    auto inventory = entity_data::local_player->inventory();
    if (!inventory) return nullptr;

    auto belt = inventory->containerBelt();
    if (!belt) return nullptr;

    auto items = belt->itemList();
    if (!items) return nullptr;

    int count = items->GetSize();
    if (count < 1) return nullptr;

    for (int i = 0; i < count; i++) {
        auto item = items->GetArray(i);
        if (!item) continue;

        if (item->uid() == entity_data::local_player->clActiveItem()) {
            return item;
        }
    }

    return nullptr;
}
std::string Get::item_shortname(SDK::Item* item) {
    if (!item) return "";

    auto info = item->info();
    if (!info) return "";

    auto sn = info->shortname();
    if (!sn || sn->len <= 0 || !sn->buffer) return "";

    char name_buf[128] = {};
    int len = (sn->len > 60) ? 60 : sn->len;

    WideCharToMultiByte(CP_UTF8, 0, sn->buffer, len, name_buf, sizeof(name_buf) - 1, nullptr, nullptr);
    return std::string(name_buf);
}

std::vector<SDK::Item*> Get::belt_items(SDK::BasePlayer* player) {
    std::vector<SDK::Item*> result(6, nullptr);
    if (!player) return result;

    auto inventory = player->inventory();
    if (!inventory) return result;

    auto belt = inventory->containerBelt();
    if (!belt) return result;

    auto items = belt->itemList();
    if (!items) return result;

    int count = items->GetSize();
    for (int i = 0; i < count && i < 6; i++) {
        result[i] = items->GetArray(i);
    }
    return result;
}
std::vector<SDK::Item*> Get::wear_items(SDK::BasePlayer* player) {
    std::vector<SDK::Item*> result(7, nullptr);
    if (!player) return result;

    auto inventory = player->inventory();
    if (!inventory) return result;

    auto wear = inventory->containerWear();
    if (!wear) return result;

    auto items = wear->itemList();
    if (!items) return result;

    int count = items->GetSize();
    for (int i = 0; i < count && i < 7; i++) {
        result[i] = items->GetArray(i);
    }
    return result;
}

bool Get::is_npc(SDK::BasePlayer* player) {
    if (!player) return false;

    if (player->IsFrom("NPCPlayer") ||
        player->IsFrom("ScientistNPC") ||
        player->IsFrom("BaseNPC") ||
        player->IsFrom("HumanNPC") ||
        player->IsFrom("TunnelDweller") ||
        player->IsFrom("UnderwaterDweller") ||
        player->IsFrom("ScarecrowNPC") ||
        player->IsFrom("BanditGuard") ||
        player->IsFrom("GingerbreadNPC") ||
        player->IsFrom("FrankensteinPet") ||
        player->IsFrom("HTNPlayer")) {
        return true;
    }

    return false;
}
void Get::entity_loop() {
    std::vector<RustPlayer> player_list;

    auto client_entities = SDK::BaseNetworkable::clientEntities();
    if (!client_entities || IsBadReadPtr(client_entities, 8)) return;

    auto entity_realm = client_entities->entityList();
    if (!entity_realm || IsBadReadPtr(entity_realm, 8)) return;

    auto buffer_list = entity_realm->GetValues();
    if (!buffer_list || IsBadReadPtr(buffer_list, 8)) return;

    int size = buffer_list->GetSize();
    Vector3 eyes_pos = transform_pos(bone_transform(entity_data::local_player, BoneList::head));

    for (int i = 0; i < size; i++) {
        RustPlayer rustPlayer;

        void* element = buffer_list->GetArray(i);
        if (!element) continue;

        auto* pawn = (SDK::BasePlayer*)(element);
        if (pawn == entity_data::local_player) continue;

        auto lifeState = pawn->lifeState();
        if (lifeState == LifeState::Dead) continue;

        if (pawn->IsFrom("BasePlayer") || is_npc(pawn)) {
            auto bone = bone_transform(pawn, BoneList::head);
            auto world_pos = transform_pos(bone);

            if (!bone) {
                rustPlayer.is_visible = false;
                continue;
            }
            if (world_pos.Empty()) {
                rustPlayer.is_visible = false;
                continue;
            }

            rustPlayer.pawn = pawn;
            rustPlayer.is_npc = is_npc(pawn);
            rustPlayer.is_visible = SDK::UnityEngine::LineOfSight(eyes_pos, world_pos);

            player_list.push_back(rustPlayer);
        }
    }

    entity_data::players_list = player_list;

    if (setting::visuals::resource_esp) {
        std::vector<RustResource> resource_list;

        for (int i = 0; i < size; i++) {
            void* element = buffer_list->GetArray(i);
            if (!element) continue;

            SDK::BaseEntity* ent = reinterpret_cast<SDK::BaseEntity*>(element);

            auto transform = ent->GetTransform();
            if (!transform) continue;

            Vector3 pos = transform->GetPosition();
            if (pos.Empty()) continue;

            float dist = (eyes_pos - pos).Length();
            if (dist > setting::visuals::ore_distance) continue;

            auto prefabName = ent->GetShortPrefabName();
            if (!prefabName || prefabName->len <= 0 || !prefabName->buffer) continue;

            char name_buf[128] = {};
            int len = (prefabName->len > 60) ? 60 : prefabName->len;
            WideCharToMultiByte(CP_UTF8, 0, prefabName->buffer, len, name_buf, sizeof(name_buf) - 1, nullptr, nullptr);
            std::string pname(name_buf);

            bool has_hemp = pname.find("hemp") != std::string::npos;
            bool has_metal = pname.find("metal") != std::string::npos;
            bool has_sulfur = pname.find("sulfur") != std::string::npos;
            bool has_ore = pname.find("ore") != std::string::npos;
            bool has_stone = pname.find("stone") != std::string::npos;

            bool is_hemp_ent = has_hemp && (ent->IsFrom("CollectibleEntity") || ent->IsFrom("OreResource"));
            bool is_ore_ent = (has_metal || has_sulfur || has_ore || has_stone) && !has_hemp;

            if (!is_hemp_ent && !is_ore_ent) continue;

            RustResource resource;
            resource.pawn = reinterpret_cast<uintptr_t>(ent);
            resource.position = pos;
            resource.name = pname;
            resource.is_hemp = false;

            if (has_hemp) {
                resource.is_hemp = true;
            }
            else if (has_metal) {
                resource.type = OreType::metal;
            }
            else if (has_sulfur) {
                resource.type = OreType::sulfur;
            }
            else {
                resource.type = OreType::stone;
            }

            bool duplicate = false;
            for (size_t j = 0; j < resource_list.size(); j++) {
                auto& existing = resource_list[j];
                float dx = pos.x - existing.position.x;
                float dy = pos.y - existing.position.y;
                float dz = pos.z - existing.position.z;

                float d = sqrtf(dx * dx + dy * dy + dz * dz);
                if (d < 1.0f) {
                    if (!resource.is_hemp && !existing.is_hemp) {
                        if (existing.type != OreType::stone && resource.type == OreType::stone) {
                            duplicate = true;
                            break;
                        }
                        if (resource.type != OreType::stone && existing.type == OreType::stone) {
                            existing = resource;
                            duplicate = true;
                            break;
                        }
                    }
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate) {
                resource_list.push_back(resource);
            }
        }

        entity_data::resource_list = resource_list;
    }
}