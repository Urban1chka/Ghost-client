#include "server_side.hpp"
#include "Hooks.hpp"
#include <algorithm>

bool check_hit(Vector3 HitPositionWorld, SDK::Projectile* projectile) {
    if (!projectile || !target_info.has_target || !target_info.target_player) return false;

    if (projectile->integrity() <= 0.f) {
        return false;
    }

    SDK::BasePlayer* target_ply = target_info.target_player;
    if (!target_ply) return false;

    float timeSinceLastTick = (SDK::UnityEngine::Time::GetRealtimeSinceStartup() - entity_data::local_player->lastSentTickTime());
    float last_tick_time = (std::max)(0.f, (std::min)(timeSinceLastTick, 1.f));

    Vector3 parentVelocity = target_ply->GetParentVelocity();
    float magnitude = parentVelocity.Magnitude();

    float num2 = 1.f + AntiHackProtection::projectile_forgiveness;
    float num9 = AntiHackProtection::projectile_clientframes / 60.f;
    float num10 = AntiHackProtection::projectile_serverframes * (std::max)(SDK::UnityEngine::Time::GetDeltaTime(), SDK::UnityEngine::Time::GetSmoothDeltaTime());
    float num12 = (last_tick_time + num9 + num10) * num2;

    float num14 = projectile->currentPosition().Distance(HitPositionWorld) + 0.01f;
    float num19 = target_ply->BoundsPadding() + num12 * magnitude + AntiHackProtection::tickhistoryforgiveness;
    float num20 = target_ply->Distance(HitPositionWorld);

    bool flag2 = (target_ply->playerFlags() & SDK::BasePlayer::PlayerFlags::Sleeping) != 0;
    bool flag3 = (target_ply->playerFlags() & SDK::BasePlayer::PlayerFlags::Wounded) != 0;
    bool flag4 = (target_ply->mounted() != nullptr);

    float maxVel = target_ply->MaxVelocity();
    float num16 = maxVel + parentVelocity.Magnitude();
    float num17 = target_ply->BoundsPadding() + num12 * num16;
    float num18 = target_ply->Distance(HitPositionWorld);

    float num21 = maxVel + parentVelocity.Magnitude();
    float num22 = num12 * num21;
    float num24 = projectile->traveledDistance() + 1.f + projectile->currentPosition().Magnitude() + num22;

    if (num20 > num19) {
        return false;
    }

    if (num14 > num24) {
        return false;
    }

    return true;
}

bool check_eye(Vector3 eyePos) {
    float timeSinceLastTick = (SDK::UnityEngine::Time::GetRealtimeSinceStartup() - entity_data::local_player->lastSentTickTime());
    float last_tick_time = (std::max)(0.f, (std::min)(timeSinceLastTick, 1.f));
    bool hit = true;
    float num = 1.f + AntiHackProtection::eye_forgiveness;
    float eye_clientframes = AntiHackProtection::eye_clientframes;
    float eye_serverframes = AntiHackProtection::eye_serverframes;
    float num2 = eye_clientframes / 60.f;

    float num3 = eye_serverframes * (std::max)({
        SDK::UnityEngine::Time::GetDeltaTime(),
        SDK::UnityEngine::Time::GetSmoothDeltaTime(),
        SDK::UnityEngine::Time::GetFixedDeltaTime()
        });

    float num4 = (last_tick_time + num2 + num3) * num;
    float num5 = entity_data::local_player->MaxVelocity() + entity_data::local_player->GetParentVelocity().Magnitude();
    float num6 = entity_data::local_player->BoundsPadding() + num4 * num5;
    float num7 = entity_data::local_player->eyes()->position().Distance(eyePos) + 0.01f;
    if (num7 > num6)
    {
        hit = false;
    }
    float num8 = abs(entity_data::local_player->GetMountVelocity().y + entity_data::local_player->GetParentVelocity().y);
    float num9 = entity_data::local_player->BoundsPadding() + num4 * num8 + entity_data::local_player->GetJumpHeight();
    float num10 = abs(entity_data::local_player->eyes()->position().y - eyePos.y) + 0.01f;
    if (num10 > num9)
    {
        hit = false;
    }
    return hit;
}