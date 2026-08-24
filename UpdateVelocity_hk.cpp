#include "Hooks.hpp"

ImVec2 rotatePoint1(const ImVec2& point, const ImVec2& center, float angle) {
    float s = sin(angle);
    float c = cos(angle);

    ImVec2 rotatedPoint;
    rotatedPoint.x = (point.x - center.x) * c - (point.y - center.y) * s + center.x;
    rotatedPoint.y = (point.x - center.x) * s + (point.y - center.y) * c + center.y;

    return rotatedPoint;
}

void hooks::methods::UpdateVelocity_hk(SDK::PlayerWalkMovement* movement) {
    if (!movement) return;

    __try {
        if (entity_data::local_player) {
            if (!movement->flying()) {
                auto TargetMovement = movement->TargetMovement();

                if (setting::movement::infinite_jump) {
                    movement->landTime() = 0.0f;
                    movement->jumpTime() = 0.0f;
                    movement->groundTime() = 9999.0f;
                }
                if (setting::movement::enable_tp_to_head) {
                    auto* local_head_bone = Get::bone_transform(entity_data::local_player, BoneList::head);
                    Vector3 local_head = Get::transform_pos(local_head_bone);

                    if (!local_head.Empty()) {
                        SDK::BasePlayer* target_player = nullptr;
                        Vector3 target_pos;
                        float best_dist = 4.5f;

                        for (const auto& player : entity_data::players_list) {
                            if (!player.pawn) continue;

                            SDK::BasePlayer* pawn = (SDK::BasePlayer*)player.pawn;
                            if (!pawn) continue;

                            auto* head_bone = Get::bone_transform(pawn, BoneList::head);
                            if (!head_bone) continue;

                            Vector3 head_pos = Get::transform_pos(head_bone);
                            if (head_pos.Empty()) continue;

                            float dist = local_head.Distance(head_pos);
                            if (dist < best_dist) {
                                best_dist = dist;
                                target_player = pawn;
                                target_pos = head_pos;
                            }
                        }

                        if (target_player && best_dist <= 3.f && is_active(setting::movement::tp_to_head_key)) {
                            ImVec2 center(target_pos.x, target_pos.z);
                            float radius = 0.55f;

                            static float rotation = 0.0f;
                            static float speed = 5.0f;

                            float delta_time = SDK::UnityEngine::Time::GetDeltaTime();
                            if (delta_time <= 0.0f || delta_time > 0.1f) delta_time = 0.016f;

                            rotation += speed * delta_time;

                            ImVec2 startPoint = rotatePoint1(
                                ImVec2(center.x + radius, center.y),
                                center,
                                rotation
                            );

                            Vector3 teleportPos(startPoint.x, target_pos.y + 0.04f, startPoint.y);
                            movement->TeleportTo(teleportPos);
                        }
                    }
                }
                if (setting::movement::no_mini_sprint) {
                    if (TargetMovement.Length() > 0.1f) {
                        float speedMult;

                        if (movement->swimming() || GetAsyncKeyState(VK_LCONTROL)) {
                            speedMult = 1.7f;
                        }
                        else {
                            speedMult = 5.6f;
                        }

                        if (is_active(setting::movement::speed_key) && setting::movement::enable_speedHack) {
                            speedMult += setting::movement::speed_value;
                        }

                        Vector3 newVel(TargetMovement.x / TargetMovement.Length() * speedMult,
                            TargetMovement.y, TargetMovement.z / TargetMovement.Length() * speedMult);

                        movement->TargetMovement() = newVel;
                    }
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    movement->UpdateVelocity();
}