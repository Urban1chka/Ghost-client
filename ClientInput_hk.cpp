#include "Hooks.hpp"
#include "custommodel.hpp"
#include "chams.hpp"

#include <cmath>
#include <unordered_map>
#include "manipulator.hpp"

namespace predict {
    float CalcBulletDrop(float time, float gravity) {
        return 0.5f * gravity * time * time;
    }

    bool SolveQuadratic(float a, float b, float c, float& t0, float& t1) {
        float disc = b * b - 4.0f * a * c;
        if (disc < 0.0f) return false;
        float sqrt_disc = std::sqrt(disc);
        t0 = (-b - sqrt_disc) / (2.0f * a);
        t1 = (-b + sqrt_disc) / (2.0f * a);
        return true;
    }

    Vector3 CalculateRustPrediction(
        Vector3 start_pos,
        Vector3 target_pos,
        Vector3 target_velocity,
        const WeaponInfo& weapon)
    {
        float bullet_speed = weapon.launch_speed > 1.0f ? weapon.launch_speed : 375.0f;
        bullet_speed *= 0.975f;

        float gravity = 9.81f * (weapon.gravity_modifier > 0.001f ? weapon.gravity_modifier : 1.0f);

        if (std::isnan(target_velocity.x) || std::isnan(target_velocity.y) || std::isnan(target_velocity.z))
            target_velocity = Vector3{ 0.f, 0.f, 0.f };

        Vector3 to_target = target_pos - start_pos;
        float dist = to_target.Length();
        if (dist < 0.1f)
            return target_pos;

        Vector3 relative_vel = target_velocity;
        Vector3 delta = to_target;

        float a = relative_vel.Dot(relative_vel) - bullet_speed * bullet_speed;
        float b = 2.0f * (delta.Dot(relative_vel));
        float c = delta.Dot(delta);

        float t0 = 0.0f, t1 = 0.0f;
        float time = dist / bullet_speed;

        if (std::fabs(a) > 0.0001f && SolveQuadratic(a, b, c, t0, t1)) {
            float best_t = 9999.0f;
            if (t0 > 0.01f && t0 < best_t) best_t = t0;
            if (t1 > 0.01f && t1 < best_t) best_t = t1;
            if (best_t < 8.0f)
                time = best_t;
        }

        time = std::clamp(time, 0.01f, 8.0f);

        Vector3 predicted = target_pos + target_velocity * time;
        float drop = CalcBulletDrop(time, gravity);
        predicted.y += drop;

        for (int iter = 0; iter < 5; ++iter) {
            Vector3 delta_p = predicted - start_pos;
            float d = delta_p.Length();
            float new_time = d / bullet_speed;
            new_time = std::clamp(new_time, 0.01f, 8.0f);

            float alpha = 0.6f;
            time = time * (1.0f - alpha) + new_time * alpha;

            predicted = target_pos + target_velocity * time;
            drop = CalcBulletDrop(time, gravity);
            predicted.y += drop;
        }

        if (dist > 70.0f) {
            float dist_factor = 1.0f - ((dist - 70.0f) * 0.0007f);
            dist_factor = std::clamp(dist_factor, 0.92f, 1.0f);
            Vector3 lead = predicted - target_pos;
            predicted = target_pos + lead * dist_factor;
        }

        Vector3 lead = predicted - target_pos;
        float max_possible_lead = bullet_speed * time * 0.94f;
        float lead_len = lead.Length();
        if (lead_len > max_possible_lead && lead_len > 0.01f) {
            predicted = target_pos + lead * (max_possible_lead / lead_len);
        }

        return predicted;
    }

    void GetAimAnglesAndPredict() {
        target_info.has_target = false;
        target_info.target_player = nullptr;
        target_info.target_transform = nullptr;

        matrix4x4 v_matrix = Get::view_matrix();
        float screen_w = static_cast<float>(SDK::UnityEngine::Screen::GetWidth());
        float screen_h = static_cast<float>(SDK::UnityEngine::Screen::GetHeight());
        Vector2 screen_center{ screen_w * 0.5f, screen_h * 0.5f };

        Vector3 eyes_pos = Get::transform_pos(Get::bone_transform(entity_data::local_player, BoneList::head));
        if (eyes_pos.Empty())
            return;

        if (is_active(setting::combat::manipulated_key) && setting::combat::enable_manipulated && !current_manipulate_angle.Empty()) {
            eyes_pos = eyes_pos + current_manipulate_angle;
        }

        float current_fov = SDK::Graphics::_fov();
        if (current_fov <= 0.0f)
            current_fov = 90.0f;

        float fov_radius = setting::combat::aim_fov * (90.0f / current_fov);
        float min_screen_dist = fov_radius;

        SDK::BasePlayer* best_target = nullptr;
        SDK::UnityEngine::Transform* best_bone_trans = nullptr;
        Vector3 best_predicted_pos{ 0.f, 0.f, 0.f };

        for (const auto& player : entity_data::players_list) {
            auto player_pawn = reinterpret_cast<SDK::BasePlayer*>(player.pawn);
            if (!player_pawn)
                continue;

            if (setting::combat::ignore_team_combat_type && Get::team(player_pawn, entity_data::local_player))
                continue;
            if (setting::combat::ignore_npc_combat_type && player.is_npc)
                continue;
            if (setting::combat::ignore_sleepers_combat_type && Get::is_sleeper(player_pawn))
                continue;

            auto bone_trans = Get::bone_transform(player_pawn, setting::combat::target_bone);
            if (!bone_trans)
                continue;

            Vector3 world_pos = Get::transform_pos(bone_trans);
            if (world_pos.Empty())
                continue;

            if (setting::combat::visible_check_combat_type) {
                if (!SDK::UnityEngine::LineOfSight(eyes_pos, world_pos)) continue;
            }

            Vector3 target_velocity = Get::player_velocity(player_pawn);
            if (std::isnan(target_velocity.x) || std::isnan(target_velocity.y) || std::isnan(target_velocity.z))
                target_velocity = Vector3{ 0.f, 0.f, 0.f };

            Vector3 temp_predicted = CalculateRustPrediction(eyes_pos, world_pos, target_velocity, weapon_info);
            if (std::isnan(temp_predicted.x) || std::isnan(temp_predicted.y) || std::isnan(temp_predicted.z))
                temp_predicted = world_pos;

            Vector2 screen_world{}, screen_pred{};
            bool world_ok = world_pos.world_to_screen(screen_world, v_matrix.matrix);
            bool pred_ok = temp_predicted.world_to_screen(screen_pred, v_matrix.matrix);

            float world_dist = world_ok ? std::hypot(screen_world.x - screen_center.x, screen_world.y - screen_center.y) : 99999.f;
            float pred_dist = pred_ok ? std::hypot(screen_pred.x - screen_center.x, screen_pred.y - screen_center.y) : 99999.f;



            bool in_fov = (world_ok && world_dist <= fov_radius) || (pred_ok && pred_dist <= fov_radius);
            if (in_fov) {
                float current = (std::min)(world_dist, pred_dist);
                if (current < min_screen_dist) {
                    min_screen_dist = current;
                    best_target = player_pawn;
                    best_predicted_pos = temp_predicted;
                    best_bone_trans = bone_trans;
                }
            }
        }


        if (best_target && !best_predicted_pos.Empty() && best_bone_trans) {
            target_info.has_target = true;
            target_info.predicted_pos = best_predicted_pos;
            target_info.aim_direction = (best_predicted_pos - eyes_pos).Normalized();
            target_info.target_player = best_target;
            target_info.target_transform = best_bone_trans;
        }
    }
}
void GetBulletTPTarget() {
    if (!setting::misc::bullet_tp) {
        bullets_target_info.bullet_target_player = nullptr;
        bullets_target_info.bullet_target_transform = nullptr;
        return;
    }

    Vector3 local_pos = Get::transform_pos(Get::bone_transform(entity_data::local_player, BoneList::head));

    SDK::BasePlayer* best_target = nullptr;
    SDK::UnityEngine::Transform* best_transform = nullptr;
    float best_dist = 99999.f;

    for (const auto& player : entity_data::players_list) {
        auto player_pawn = (SDK::BasePlayer*)player.pawn;
        if (!player_pawn || player_pawn == entity_data::local_player) continue;

        if (setting::combat::ignore_team_combat_type && Get::team(player_pawn, entity_data::local_player)) continue;
        if (setting::combat::ignore_npc_combat_type && player.is_npc) continue;
        if (setting::combat::ignore_sleepers_combat_type && Get::is_sleeper(player_pawn)) continue;
        if (player_pawn->lifeState() == LifeState::Dead) continue;
        if (player_pawn->health() <= 0.f) continue;

        SDK::UnityEngine::Transform* player_bone = Get::bone_transform(player_pawn, BoneList::head);
        if (!player_bone) continue;

        Vector3 player_pos = Get::transform_pos(player_bone);
        if (player_pos.Empty()) continue;

        float dist = local_pos.Distance(player_pos);
        if (dist < best_dist) {
            best_dist = dist;
            best_target = player_pawn;
            best_transform = player_bone;
        }
    }

    bullets_target_info.bullet_target_player = best_target;
    bullets_target_info.bullet_target_transform = best_transform;
}

void hooks::methods::ClientInput_hk(SDK::BasePlayer* local_player, uintptr_t state) {
    SDK::BasePlayer::ClientInput_(local_player, state);

    __try {
        entity_data::local_player = SDK::LocalPlayer::GetEntity();
        Get::entity_loop();

        if (entity_data::local_player) {
            reset(false);

            auto active_item = Get::active_item(entity_data::local_player);
            auto held_raw = active_item ? active_item->heldEntity() : 0;
            auto* held_entity = held_raw ? reinterpret_cast<SDK::BaseProjectile*>(held_raw) : nullptr;
            auto* held_base = held_raw ? reinterpret_cast<SDK::BaseEntity*>(held_raw) : nullptr;
            int id = active_item && active_item->info() ? active_item->info()->itemid() : 0;

            auto movement = entity_data::local_player->movement();
            if (!movement) return;

            auto walk_movement = (SDK::PlayerWalkMovement*)movement;
            if (!walk_movement) return;

            Vector3 localEyes = Get::transform_pos(Get::bone_transform(entity_data::local_player, BoneList::head));
            if (localEyes.Empty()) return;

            if (setting::visuals::custommodel::enabled) {
                custommodel::tick();
            }
            if (setting::visuals::chams::enabled) {
                chams::tick();
            }

            if (setting::combat::aim_enable) {
                predict::GetAimAnglesAndPredict();
            }
            if (setting::misc::bullet_tp) {
                GetBulletTPTarget();
            }

            if (setting::movement::no_mini_sprint) {
                entity_data::local_player->add_modelstate_flag(SDK::ModelState::Flags::Sprinting);
            }
            if (setting::movement::insta_pickup_player) {
                for (const auto& player : entity_data::players_list) {
                    auto player_pawn = (SDK::BasePlayer*)player.pawn;

                    if (player_pawn->HasPlayerFlag(SDK::BasePlayer::PlayerFlags::Wounded) && localEyes.Distance(Get::transform_pos(Get::bone_transform(player_pawn, BoneList::head))) < 3.f && entity_data::local_player->GetKeyState(RustButton::USE)) {
                        AssistPlayer(player_pawn);
                    }
                }
            }
            if (setting::visuals::time_changer) {
                SDK::Admin::admintime() = setting::visuals::time_value;
            }
            if (setting::misc::silent_reload) {
                if (held_entity && held_base) {
                    if (id != 795236088 && id != 200773292 && id != 1525520776) {
                        float last_shot_time = held_entity->lastShotTime();

                        static float prev_last_shot = 0.0f;
                        static uint64_t prev_item = 0;
                        uint64_t cur_item = entity_data::local_player->clActiveItem();

                        if (cur_item != prev_item) {
                            prev_item = cur_item;
                            prev_last_shot = last_shot_time;
                            did_reload = false;
                            just_shot = false;
                            time_since_last_shot = 0;
                        }

                        if (last_shot_time != prev_last_shot && last_shot_time > 0.0f) {
                            just_shot = true;
                            did_reload = false;
                            fixed_time_last_shot = SDK::UnityEngine::Time::GetTime();
                            prev_last_shot = last_shot_time;
                        }

                        if (!did_reload) {
                            time_since_last_shot = SDK::UnityEngine::Time::GetTime() - fixed_time_last_shot;
                        }
                        if (just_shot && (time_since_last_shot > 0.2f)) {
                            held_base->ServerRPC("StartReload");
                            held_base->SendSignalBroadcast(SDK::BaseEntity::Signal::Reload);
                            just_shot = false;
                        }
                        if (time_since_last_shot > (held_entity->reloadTime() - (held_entity->reloadTime() / 10))
                            && !did_reload) {
                            held_base->ServerRPC("Reload");
                            did_reload = true;
                            time_since_last_shot = 0;
                        }
                    }
                }
            }
            if (setting::misc::no_sway) {
                if (held_entity != 0) {
                    held_entity->aimSway() = 0.0f;
                    held_entity->aimSwaySpeed() = 0.0f;
                    held_entity->aimconePenalty() = 0.0f;
                    held_entity->stancePenalty() = 0.0f;
                }
            }

            if (setting::visuals::fov_changer || is_active(setting::visuals::zoom_key) || !setting::visuals::fov_changer) {
                float targetFOV = 90.0f;

                if (is_active(setting::visuals::zoom_key)) {
                    targetFOV = setting::visuals::zoom_value;
                }
                else if (setting::visuals::fov_changer) {
                    targetFOV = setting::visuals::fov_value;
                }

                SDK::Graphics::_fov() = targetFOV;
            }
            if (setting::movement::enable_spider && is_active(setting::movement::spider_key) ||
                setting::movement::enable_airStack && is_active(setting::movement::airStack_key) ||
                setting::movement::enable_tp_to_head && is_active(setting::movement::tp_to_head_key)) {
                walk_movement->groundAngle() = 0.0f;
                walk_movement->groundAngleNew() = 0.0f;
            }

            if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key)) {
                find_manipulate_angle();
            }
            if (setting::movement::enable_airStack && is_active(setting::movement::airStack_key)) {
                walk_movement->gravityMultiplier() = 0.0f;
                walk_movement->gravityMultiplierSwimming() = -4.0f;
                walk_movement->jumpTime() = 0.0f;
                walk_movement->jumping() = false;
            }
            if (setting::combat::aim_enable && is_active(setting::combat::aim_key) && target_info.has_target) {
                if (!setting::combat::use_pSilent) {
                    auto playerInput = entity_data::local_player->input();
                    if (playerInput) {
                        Vector3 current_angles = playerInput->bodyAngles();
                        Vector3 target_angles = Vector3::Angles(localEyes, target_info.predicted_pos);

                        Vector3 delta = target_angles - current_angles;

                        while (delta.x > 180.0f)  delta.x -= 360.0f;
                        while (delta.x < -180.0f) delta.x += 360.0f;
                        while (delta.y > 180.0f)  delta.y -= 360.0f;
                        while (delta.y < -180.0f) delta.y += 360.0f;

                        float smooth = max(1.0f, setting::combat::aim_smooth);
                        Vector3 smoothed_angles = current_angles + (delta / smooth);

                        playerInput->bodyAngles() = smoothed_angles.ClampAngles();
                    }
                }
            }
            if (setting::combat::auto_fire && is_active(setting::combat::aim_key)) {
                if (setting::combat::aim_enable && setting::combat::use_pSilent) {
                    if (target_info.has_target && target_info.target_player) {
                        if (held_entity && active_item) {
                            const char* classname = held_entity->GetClassName();
                            bool isProjectile = (
                                strcmp(classname, "BaseProjectile") == 0 ||
                                strcmp(classname, "BowWeapon") == 0 ||
                                strcmp(classname, "CompoundBowWeapon") == 0 ||
                                strcmp(classname, "BaseLauncher") == 0 ||
                                strcmp(classname, "CrossbowWeapon") == 0 ||
                                strcmp(classname, "FlintStrikeWeapon") == 0);

                            if (isProjectile) {
                                float current_time = SDK::UnityEngine::Time::GetTime();
                                if (!held_entity->HasReloadCooldown() && target_info.target_player && held_entity->nextAttackTime() <= current_time) {

                                    auto target_playerPos = target_info.predicted_pos;
                                    Vector3 shot_eyes = localEyes;
                                    bool can_shoot = false;

                                    if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key) && !current_manipulate_angle.Empty()) {
                                        shot_eyes = localEyes + current_manipulate_angle;
                                    }

                                    if (SDK::UnityEngine::LineOfSight(shot_eyes, target_playerPos)) {
                                        Vector3 test_eye = entity_data::local_player->eyes()->position();
                                        if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key) && !current_manipulate_angle.Empty()) {
                                            test_eye = test_eye + current_manipulate_angle;
                                        }
                                        if (check_eye(test_eye)) {
                                            can_shoot = true;
                                        }
                                    }

                                    if (!can_shoot && setting::combat::enable_hitscan) {
                                        HitscanResult hs = hitscan_from_eye(target_playerPos);
                                        if (hs.found) {
                                            shot_eyes = hs.hit_point;
                                            if (SDK::UnityEngine::LineOfSight(shot_eyes, target_playerPos)) {
                                                Vector3 test_eye = entity_data::local_player->eyes()->position();
                                                if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key) && !current_manipulate_angle.Empty()) {
                                                    test_eye = test_eye + (shot_eyes - localEyes);
                                                }
                                                if (check_eye(test_eye)) {
                                                    can_shoot = true;
                                                }
                                            }
                                        }
                                    }

                                    if (can_shoot) {
                                        held_entity->SendSignalBroadcast(SDK::BaseEntity::Signal::Attack, "");
                                        held_entity->doAttack();
                                        held_entity->LaunchProjectile();
                                        held_entity->ShotFired();
                                        held_entity->DidAttackClientside();
                                        held_entity->UpdateAmmoDisplay();
                                        held_entity->nextAttackTime() = current_time + held_entity->repeatDelay();
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (auto mounted = entity_data::local_player->mounted()) {
                if (mounted) {
                    mounted->canWieldItems() = setting::movement::can_wield_items;
                }
            }
        }
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}