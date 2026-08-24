#include "Hooks.hpp"

void hooks::methods::DoMovement_hk(SDK::Projectile* projectile, float deltaTime) {
    if (!projectile) {
        return;
    }

    if (projectile->owner() == entity_data::local_player) {
        auto currentVelocity = projectile->currentVelocity() * deltaTime;
        auto intmag = currentVelocity.Magnitude();

        if (intmag > 0.0001f) {
            auto intnum = 1.f / intmag;
            auto speed = currentVelocity * intnum;
            auto lineposition = projectile->currentPosition() + speed * currentVelocity.Magnitude();

            if (setting::misc::bullet_tp) {
                SDK::BasePlayer* projectileTarget = bullets_target_info.bullet_target_player;
                if (projectileTarget) {
                    SDK::UnityEngine::Transform* playerTransform = bullets_target_info.bullet_target_transform;
                    SDK::HitTest* hitTest = projectile->hitTest();

                    if (playerTransform && hitTest) {
                        Vector3 playerPos = playerTransform->GetPosition();
                        Vector3 bulletPos = projectile->currentPosition();
                        Vector3 nextBulletPos = projectile->currentPosition() + (projectile->currentVelocity() * SDK::UnityEngine::Time::GetFixedDeltaTime());

                        SDK::Line updateLine = SDK::Line(bulletPos, nextBulletPos);
                        Vector3 closestUpdateToPlayer = updateLine.ClosestPoint(playerPos);

                        Vector3 yplayerPos;
                        if (closestUpdateToPlayer.y - 1 > playerTransform->GetPosition().y)
                            yplayerPos = playerTransform->GetPosition() - Vector3(0, 0.2, 0);
                        else
                            yplayerPos = playerTransform->GetPosition();

                        float distanceToPlayer = closestUpdateToPlayer.Distance(yplayerPos);

                        Vector3 newBulletPos = Vector3::MoveTowards(closestUpdateToPlayer, playerPos, 0.99f);
                        Vector3 newBulletPosx2 = Vector3::MoveTowards(newBulletPos, playerPos, 0.99f);

                        if (distanceToPlayer < 3.2f) {
                            Vector3 pointStart = newBulletPos;
                            Vector3 vector = newBulletPosx2;
                            vector -= projectile->currentVelocity().Normalized() * 0.001f;
                            Vector3 vector2 = vector;
                            Vector3 b2 = (pointStart - closestUpdateToPlayer).Normalized() * 0.01f;
                            Vector3 b3 = (vector2 - pointStart).Normalized() * 0.01f;

                            if (SDK::UnityEngine::LineOfSight(bulletPos, closestUpdateToPlayer) &&
                                SDK::UnityEngine::LineOfSight(closestUpdateToPlayer, playerPos) &&
                                SDK::UnityEngine::LineOfSight(closestUpdateToPlayer - b2, pointStart + b2) &&
                                SDK::UnityEngine::LineOfSight(pointStart - b3, vector2) &&
                                SDK::UnityEngine::LineOfSight(vector2, vector))
                            {
                                SDK::ProtoBuf::PlayerProjectileUpdate* ppu = SDK::ProtoBuf::PlayerProjectileUpdate::New();
                                if (ppu && hitTest && playerTransform && projectileTarget) {
                                    if (projectile->projectileID() != 0) {
                                        Vector3 velocityPerTick = projectile->currentVelocity() * SDK::UnityEngine::Time::GetFixedDeltaTime();
                                        float velocityPerTickSpeed = velocityPerTick.Length();

                                        if (velocityPerTickSpeed > 0.0001f) {
                                            float speedPerSecond = 1.f / velocityPerTickSpeed;
                                            float distanceTraveled = (closestUpdateToPlayer - bulletPos).Length();
                                            float traveledTime = distanceTraveled * speedPerSecond * SDK::UnityEngine::Time::GetFixedDeltaTime();

                                            projectile->traveledDistance() += distanceTraveled;
                                            projectile->traveledTime() += traveledTime;

                                            ppu->projectileId() = projectile->projectileID();
                                            ppu->travelTime() = projectile->traveledTime() + traveledTime;
                                            ppu->curVelocity() = projectile->currentVelocity();
                                            ppu->curPosition() = closestUpdateToPlayer;

                                            if (entity_data::local_player) {
                                                entity_data::local_player->SendProjectileUpdate(ppu);
                                                ppu->curPosition() = newBulletPos;
                                                entity_data::local_player->SendProjectileUpdate(ppu);
                                            }

                                            hitTest->DidHit() = true;
                                            hitTest->HitEntity() = projectileTarget;
                                            hitTest->HitTransform() = playerTransform;
                                            hitTest->HitPoint() = playerTransform->InverseTransformPoint(newBulletPosx2);
                                            hitTest->HitNormal() = playerTransform->InverseTransformDirection(newBulletPosx2);
                                            hitTest->AttackRay() = SDK::UnityEngine::Ray(newBulletPos, Vector3());

                                            auto matStr = SDK::System::String::New("Flesh");
                                            if (matStr) {
                                                hitTest->HitMaterial() = matStr;
                                            }

                                            if (check_hit(newBulletPosx2, projectile)) {
                                                projectile->DoHit(hitTest, newBulletPosx2, Vector3());
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Vector3 bulletPos = projectile->currentPosition();
            Vector3 nextBulletPos = projectile->currentPosition() + (projectile->currentVelocity() * SDK::UnityEngine::Time::GetFixedDeltaTime());
        }
    }

    return projectile->DoMovement(deltaTime);
}