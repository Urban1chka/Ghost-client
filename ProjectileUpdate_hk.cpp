#include "Hooks.hpp"

void hooks::methods::ProjectileUpdate_hk(SDK::Projectile* projectile) {
    if (!projectile) return;

    __try {
        projectile->Update();

        if (entity_data::local_player) {
            if (projectile->owner() != entity_data::local_player) return;

            SDK::BasePlayer* best_target = nullptr;
            Vector3 best_hit_pos = { 0,0,0 };

            Vector3 projectile_pos = projectile->currentPosition();
            Vector3 projectile_vel = projectile->currentVelocity();

            SDK::HitTest* hitTest = projectile->hitTest();
            SDK::HitInfo* hitInfo = projectile->hitInfo();

            if (setting::combat::aim_enable) {
                if (projectile->traveledTime() <= 0.4f) {
                    Vector3 velocity = projectile->currentVelocity();
                    float speed = velocity.Length();

                    if (speed > 1.0f) {
                        weapon_info.launch_speed = speed;
                    }
                    weapon_info.gravity_modifier = projectile->gravityModifier();
                    weapon_info.drag = projectile->drag();
                }
            }
            if (setting::visuals::draw_bullet_tracers) {
                void* projectile_id = (void*)projectile;
                float current_time = SDK::UnityEngine::Time::GetTime();
                float traveled = projectile->traveledTime();
                Vector3 pos = projectile->currentPosition();

                auto it = last_bulletPosition.find(projectile_id);
                if (it != last_bulletPosition.end()) {
                    const auto& info = it->second;
                    float dist = (pos - info.pos).Length();
                    float time_delta = current_time - info.time;

                    bool continuous =
                        traveled > 0.015f &&
                        time_delta < 0.15f &&
                        dist > 0.6f && dist < 90.0f;

                    if (continuous) {
                        tracers.emplace_back(TracerSegment{
                            info.pos,
                            pos,
                            current_time
                            });
                    }
                }
                last_bulletPosition[projectile_id] = { pos, current_time };
            }
        }
    }   
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}