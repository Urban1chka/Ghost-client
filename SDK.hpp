#pragma once
#include <iostream>
#include "Dissector.hpp"
#include "setting.hpp"
#include "System.hpp"
#include "Structs.hpp"
#include "UnityEngine.hpp"

namespace SDK {
    class FacepunchBehaviour : public UnityEngine::MonoBehaviour { public: };
    class BaseMonoBehaviour : public FacepunchBehaviour { public: };

    class Bounds {
    public:
        Vector3 m_Center;
        Vector3 m_Extents;
        bool InsideBounds(Vector3 targetPoint) {
            Vector3 minBound(this->m_Center.x - this->m_Extents.x, this->m_Center.y - this->m_Extents.y, this->m_Center.z - this->m_Extents.z);
            Vector3 maxBound(this->m_Center.x + this->m_Extents.x, this->m_Center.y + this->m_Extents.y, this->m_Center.z + this->m_Extents.z);
            if (targetPoint.x >= minBound.x && targetPoint.x <= maxBound.x &&
                targetPoint.y >= minBound.y && targetPoint.y <= maxBound.y &&
                targetPoint.z >= minBound.z && targetPoint.z <= maxBound.z)
                return true;
            return false;
        }
    };

    class OBB {
    public:
        CLASS("", "OBB");
        UnityEngine::Quaternion rotation;
        Vector3 position;
        Vector3 extents;
        Vector3 forward;
        Vector3 right;
        Vector3 up;
        float reject;
        METHOD(ToBounds, "ToBounds", Bounds);
    };

    class MainMenuSystem : public UnityEngine::Component { public: };

    class VisualizeTexelDensity : public UnityEngine::MonoBehaviour {
    public:
        CLASS("", "VisualizeTexelDensity");
        FIELD(showHUD, "showHUD", bool);
    };

    class SystemTime {
    public:
        static uint64_t GetGameTick() {
            return GetTickCount64();
        }

        static bool HasElapsed(uint64_t startTick, uint64_t ms) {
            return (GetTickCount64() - startTick) >= ms;
        }
    };

    class BaseNetworkable : public BaseMonoBehaviour {
    public:
        CLASS("", "BaseNetworkable");

        class EntityRealm {
        public:
            CLASS("", "BaseNetworkable/EntityRealm");
            FIELD(entityList, "entityList", System::ListDictionary*);

            METHOD(_Find, "Find", BaseNetworkable*, uint32_t);

            template<typename T = BaseNetworkable*>
            T Find(uint32_t uid) {
                if (!this) return nullptr;
                return reinterpret_cast<T>(_Find(uid));
            }
        };

        STATIC_FIELD(clientEntities, "clientEntities", EntityRealm*);
        METHOD(GetShortPrefabName, "get_ShortPrefabName", System::String*);
    };

    class Model : public UnityEngine::Object {
    public:
        CLASS("", "Model");
        FIELD(boneTransforms, "boneTransforms", System::Array<UnityEngine::Transform*>*);
        METHOD(get_transform, "get_transform", UnityEngine::Transform*);
    };

    class BaseEntity : public BaseNetworkable {
    public:
        enum class Signal {
            Attack,
            Alt_Attack,
            DryFire,
            Reload,
            Deploy,
            Flinch_Head,
            Flinch_Chest,
            Flinch_Stomach,
            Flinch_RearHead,
            Flinch_RearTorso,
            Throw,
            Relax,
            Gesture,
            PhysImpact,
            Eat,
            Startled
        };
        CLASS("", "BaseEntity");

        FIELD(model, "model", Model*);

        METHOD(WorldSpaceBounds, "WorldSpaceBounds", OBB);
        METHOD(BoundsPadding, "BoundsPadding", float);
        METHOD(Distance, "Distance", float, Vector3);

        METHOD(_ServerRPC, "ServerRPC", void, System::String*);
        METHOD(_SendSignalBroadcast, "SendSignalBroadcast", void, Signal, void*);

        void SendSignalBroadcast(Signal a, const char* str = "") {
            if (!this) return;

            void* managedString = Dissector::IL2CPP::CreateNewString(str ? str : "");
            _SendSignalBroadcast(a, managedString);
        }

        void ServerRPC(const char* funcName) {
            if (!this) return;
            auto* str = System::String::New(funcName);
            if (!str) return;
            _ServerRPC(str);
        }
    };

    class AttackEntity : public BaseEntity {
    public:
        CLASS("", "AttackEntity");

        METHOD(StartAttackCooldown, "StartAttackCooldown", void, float);
        METHOD(HasAttackCooldown, "HasAttackCooldown", bool);

        FIELD(lastTickTime, "lastTickTime", float);
        FIELD(repeatDelay, "repeatDelay", float);
        FIELD(deployDelay, "deployDelay", float);
        FIELD(timeSinceDeploy, "timeSinceDeploy", float);
        FIELD(nextAttackTime, "nextAttackTime", float);
    };

    class ItemDefinition {
    public:
        CLASS("", "ItemDefinition");
        FIELD(itemid, "itemid", int);
        FIELD(displayName, "displayName", System::String*);
        FIELD(shortname, "shortname", System::String*);
        FIELD(iconSprite, "iconSprite", UnityEngine::Sprite*);
    };

    class Item {
    public:
        CLASS("", "Item");
        FIELD(uid, "uid", uint64_t);
        FIELD(info, "info", ItemDefinition*);
        FIELD(amount, "amount", int);
        FIELD(heldEntity, "heldEntity", uintptr_t);
    };

    class BaseViewModel {
    public:
        CLASS("", "BaseViewModel");

        STATIC_METHOD(GetActiveModel, "get_ActiveModel", BaseViewModel*);
    };

    class ItemContainer {
    public:
        CLASS("", "ItemContainer");
        FIELD(itemList, "itemList", System::List<Item*>*);
    };

    class PlayerInventory {
    public:
        CLASS("", "PlayerInventory");
        FIELD(containerMain, "containerMain", ItemContainer*);
        FIELD(containerBelt, "containerBelt", ItemContainer*);
        FIELD(containerWear, "containerWear", ItemContainer*);
    };

    class HeldEntity : public BaseEntity {
    public:
        CLASS("", "HeldEntity");

        FIELD(Item, "Item", HeldEntity*);
        FIELD(viewModel, "viewModel", HeldEntity*);

        static inline void(*AddPunch_)(HeldEntity*, Vector3, float) = nullptr;
        void AddPunch(Vector3 amount, float duration) {
            if (AddPunch_) AddPunch_(this, amount, duration);
        }
    };

    class Attack {
    public:
        CLASS("", "Attack");

        FIELD(pointStart, "pointStart", Vector3);
        FIELD(pointEnd, "pointEnd", Vector3);
        FIELD(hitMaterialID, "hitMaterialID", Vector3);
        FIELD(hitPositionWorld, "hitPositionWorld", Vector3);
        FIELD(hitNormalWorld, "hitNormalWorld", Vector3);
        FIELD(hitID, "hitID", uint32_t);
        FIELD(hitBone, "hitBone", uint32_t);
    };

    class HitTest;
    class HitInfo;
    class Projectile;

    class BasePlayer;

    class HitTest {
    public:
        CLASS("", "HitTest");

        FIELD(DidHit, "DidHit", bool);
        FIELD(HitEntity, "HitEntity", BaseEntity*);
        FIELD(HitTransform, "HitTransform", UnityEngine::Transform*);
        FIELD(HitPoint, "HitPoint", Vector3);
        FIELD(HitNormal, "HitNormal", Vector3);
        FIELD(HitMaterial, "HitMaterial", System::String*);
        FIELD(Radius, "Radius", float);
        FIELD(Forgiveness, "Forgiveness", float);
        FIELD(MaxDistance, "MaxDistance", float);
        FIELD(MultiHit, "MultiHit", bool);
        FIELD(BestHit, "BestHit", bool);
        FIELD(ignoreEntity, "ignoreEntity", BaseEntity*);
        FIELD(HitDistance, "HitDistance", float);
        FIELD(HitPart, "HitPart", uint32_t);
        FIELD(AttackRay, "AttackRay", UnityEngine::Ray);
        FIELD(DamageProperties, "DamageProperties", void*);
        FIELD(HitPartName, "HitPartName", System::String*);
        FIELD(GameMaterial, "gameMaterial", void*);
        FIELD(Collider, "collider", void*);
    };
    class HitInfo {
    public:
        CLASS("", "HitInfo");

        FIELD(HitPartName, "HitPartName", System::String*);
        FIELD(HitMaterial, "HitMaterial", System::String*);
        FIELD(Forgiveness, "Forgiveness", float);
        FIELD(HitDistance, "HitDistance", float);

        FIELD(Damage, "damage", float);
        FIELD(HitPosition, "hitPosition", Vector3);
        FIELD(HitNormal, "hitNormal", Vector3);
        FIELD(Point, "point", Vector3);
        FIELD(Direction, "dir", Vector3);
        FIELD(HitEntity, "hitEntity", BaseEntity*);
    };

    class TOD_CycleParameters {
    public:
        CLASS("", "TOD_CycleParameters");
        FIELD(Hour, "Hour", float);
    };
    class TOD_DayParameters {
    public:
        CLASS("", "TOD_DayParameters");
        FIELD(AmbientMultiplier, "AmbientMultiplier", float);
        FIELD(ReflectionMultiplier, "ReflectionMultiplier", float);
    };
    class TOD_NightParameters {
    public:
        CLASS("", "TOD_NightParameters");
        FIELD(AmbientMultiplier, "AmbientMultiplier", float);
        FIELD(LightMultiplier, "LightMultiplier", float);

        FIELD(AmbientColor, "AmbientColor", UnityEngine::Color);
        FIELD(LightColor, "LightColor", UnityEngine::Color);
    };

    class TOD_AtmosphereParameters {
    public:
        CLASS("", "TOD_AtmosphereParameters");
        FIELD(FogMultiplier, "FogMultiplier", float);
        FIELD(FarthestFogDistance, "FarthestFogDistance", float);
    };

    class TOD_StarParameters {
    public:
        CLASS("", "TOD_StarParameters");
        FIELD(Size, "Size", float);
        FIELD(Brightness, "Brightness", float);
    };

    class ItemIcon {
    public:
        CLASS("", "ItemIcon");

        FIELD(queuedForLooting, "queuedForLooting", bool);
        FIELD(item, "item", SDK::Item*);

        static inline void (*TryToMove_)(ItemIcon*, void*) = nullptr;
        void TryToMove(void* method = nullptr) {
            if (TryToMove_) TryToMove_(this, method);
        }

        void RunTimedAction() {
            if (!this) return;
            static auto method = get_itemIcon_method("RunTimedAction", 0, "System.Void");
            if (method) {
                using fn_t = void(*)(ItemIcon*);
                reinterpret_cast<fn_t>(method)(this);
            }
        }
    private:
        static void* get_itemIcon_method(const char* methodName, int paramCount, const char* returnType = "System.Void") {
            static Dissector::IL2CPP::IL2CPPClass* itemIconClass = nullptr;

            if (!itemIconClass) {
                itemIconClass = Dissector::FindClass("Assembly-CSharp", "ItemIcon");
                if (!itemIconClass) {
                    itemIconClass = Dissector::FindClass("", "ItemIcon");
                }
            }

            if (!itemIconClass)
                return nullptr;

            auto* method = Dissector::FindMethod(itemIconClass, methodName, paramCount, returnType);
            if (!method)
                return nullptr;

            return method->methodPtr;
        }
    };

    class Shader {
    public:
        CLASS("UnityEngine", "Shader");

        STATIC_METHOD(Find, "Find", Shader*, System::String*);
        STATIC_METHOD(PropertyToID, "PropertyToID", int, System::String*);

        static Shader* Find(const char* name) {
            auto* str = System::String::New(name);
            if (!str) return nullptr;
            return Find(str);
        }

        static int PropertyToID(const char* name) {
            auto* str = System::String::New(name);
            if (!str) return 0;
            return PropertyToID(str);
        }
    };

    class Material : public UnityEngine::GameObject {
    public:
        CLASS("UnityEngine", "Material");

        void ctor_shader(Shader* sh) {
            static auto m = Dissector::FindMethod(TypeClass(), ".ctor", 1);
            if (m) ((void(*)(void*, Shader*))m->methodPtr)(this, sh);
        }
        void ctor_material(Material* src) {
            static auto m = Dissector::FindMethod(TypeClass(), ".ctor", 1);
            if (m) ((void(*)(void*, Material*))m->methodPtr)(this, src);
        }
        METHOD(SetColor, "SetColor", void, int, UnityEngine::Color);
        METHOD(SetInt, "SetInt", void, int, int);
        METHOD(SetFloat, "SetFloat", void, int, float);
        METHOD(SetTexture, "SetTexture", void, int, UnityEngine::Object*);
        METHOD(shader, "get_shader", Shader*);
        METHOD(SetShader, "set_shader", void, Shader*);
        METHOD(get_color, "get_color", UnityEngine::Color);
        METHOD(set_color, "set_color", void, UnityEngine::Color);

        void SetColor(const char* proper, UnityEngine::Color value) {
            SetColor(Shader::PropertyToID(proper), value);
        }

        void SetInt(const char* name, int value) {
            SetInt(Shader::PropertyToID(name), value);
        }

        void SetFloat(const char* name, float value) {
            SetFloat(Shader::PropertyToID(name), value);
        }

        void SetTexture(const char* name, UnityEngine::Object* tex) {
            SetTexture(Shader::PropertyToID(name), tex);
        }
    };

    struct Line {
        Vector3 point0;
        Vector3 point1;

        Line(Vector3 point0, Vector3 point1) {
            this->point0 = point0;
            this->point1 = point1;
        }

        Line(Vector3 origin, Vector3 direction, float length) {
            this->point0 = origin;
            this->point1 = origin + direction * length;
        }
        Vector3 ClosestPoint(Vector3 pos) {
            Vector3 a = this->point1 - this->point0;
            float magnitude = a.Magnitude();
            Vector3 vector = a / magnitude;
            return this->point0 + (vector * std::clamp(Vector3(pos - this->point0).dot_product(vector), 0.f, magnitude));
        }
    };

    class TOD_Sky : public UnityEngine::MonoBehaviour {
    public:
        enum AmbientMode {
            Skybox,
            Trilight,
            Flat,
            Custom
        };

        CLASS("", "TOD_Sky");
        STATIC_FIELD(instances, "instances", System::List<TOD_Sky*>*);

        FIELD(Cycle, "Cycle", void*);
        FIELD(Day, "Day", void*);
        FIELD(Night, "Night", void*);
        FIELD(Atmosphere, "Atmosphere", void*);
        FIELD(Clouds, "Clouds", void*);
        FIELD(Stars, "Stars", TOD_StarParameters*);
        FIELD(Weather, "Weather", void*);

        class RenderSettings {
        public:
            CLASS("UnityEngine", "RenderSettings");
            STATIC_METHOD(set_ambientMode, "set_ambientMode", void, int);
            STATIC_METHOD(set_ambientIntensity, "set_ambientIntensity", void, float);
            STATIC_METHOD(set_ambientLight, "set_ambientLight", void, UnityEngine::Color);
            STATIC_METHOD(get_skybox, "get_skybox", Material*);
            STATIC_METHOD(set_skybox, "set_skybox", void, Material*);
            STATIC_METHOD(set_fogColor, "set_fogColor", void, UnityEngine::Color);
            STATIC_METHOD(set_fogDensity, "set_fogDensity", void, float);
        };

        static void set_ambientMode(int mode) {
            RenderSettings::set_ambientMode(mode);
        }

        static void set_ambientIntensity(float intensity) {
            RenderSettings::set_ambientIntensity(intensity);
        }

        static void set_ambientLight(UnityEngine::Color color) {
            RenderSettings::set_ambientLight(color);
        }

        static Material* get_skybox() {
            return RenderSettings::get_skybox();
        }

        static void set_skybox(Material* material) {
            RenderSettings::set_skybox(material);
        }

        static void set_fogColor(UnityEngine::Color color) {
            RenderSettings::set_fogColor(color);
        }

        static void set_fogDensity(float density) {
            RenderSettings::set_fogDensity(density);
        }

        static void set_skybox_color(UnityEngine::Color color) {
            static auto id = []() -> int {
                return Shader::PropertyToID((char*)"_Tint");
                }();
            Material* mat = get_skybox();
            if (mat) {
                mat->SetColor("_Tint", color);
            }
        }

        static inline void(*UpdateAmbient_)(TOD_Sky*) = nullptr;
        void UpdateAmbient() {
            if (UpdateAmbient_) UpdateAmbient_(this);
        }
    };

    class Admin {
    public:
        CLASS("ConVar", "Admin");
        STATIC_FIELD(admintime, "admintime", float);
    };

    class Graphics {
    public:
        CLASS("ConVar", "Graphics");
        STATIC_FIELD(_fov, "_fov", float);
    };

    class SkinnedMultiMesh;
    class PlayerModel;

    class Renderer {
    public:
        CLASS("", "Renderer");

        METHOD(material, "get_material", Material*);
        METHOD(get_sharedMaterial, "get_sharedMaterial", Material*);
        METHOD(set_material, "set_material", void, Material*);
        METHOD(set_enabled, "set_enabled", void, bool);
        METHOD(isVisible, "get_isVisible", bool);
        METHOD(materials, "get_materials", System::Array<Material*>*);
    };

    class SkinnedMultiMesh {
    public:
        CLASS("", "SkinnedMultiMesh");

        FIELD(Renderers, "<Renderers>k__BackingField", System::List<Renderer*>*);
        METHOD(SetVisible, "SetVisible", void, bool);
        METHOD(IsCurrentlyVisible, "IsCurrentlyVisible", bool);
        static inline void(*RebuildModel_)(SkinnedMultiMesh*, PlayerModel*, bool) = nullptr;
        void RebuildModel(PlayerModel* model, bool reset) {
            if (RebuildModel_)
                RebuildModel_(this, model, reset);
        }
    };

    class PlayerModel {
    public:
        CLASS("", "PlayerModel");

        FIELD(Position, "Position", Vector3);
        FIELD(_multiMesh, "_multiMesh", SkinnedMultiMesh*);
        STATIC_METHOD(RebuildAll, "RebuildAll", void);
    };

    class Mesh : public UnityEngine::Object {
    public:
        CLASS("UnityEngine", "Mesh");

        STATIC_METHOD(Internal_Create, "Internal_Create", void, Mesh*);
        METHOD(get_vertexCount, "get_vertexCount", int);
        METHOD(set_subMeshCount, "set_subMeshCount", void, uint32_t);
        METHOD(SetArrayForChannelImpl, "SetArrayForChannelImpl", void, int, int, int, System::Array<void*>*, int, int, int, int);
        METHOD(SetIndicesImpl, "SetIndicesImpl", void, int, int, int, System::Array<void*>*, int, int, int, int);
        METHOD(SetBoneWeightsImpl, "SetBoneWeightsImpl", void, System::Array<void*>*);
        METHOD(set_bindposes, "set_bindposes", void, System::Array<void*>*);
        METHOD(RecalculateBoundsImpl, "RecalculateBoundsImpl", void, int);
        METHOD(RecalculateNormalsImpl, "RecalculateNormalsImpl", void, int);
        METHOD(RecalculateTangentsImpl, "RecalculateTangentsImpl", void, int);
        METHOD(UploadMeshDataImpl, "UploadMeshDataImpl", void, bool);
    };

    class MeshFilter : public UnityEngine::Component {
    public:
        CLASS("UnityEngine", "MeshFilter");

        METHOD(set_sharedMesh, "set_sharedMesh", void, Mesh*);
        METHOD(get_sharedMesh, "get_sharedMesh", Mesh*);
    };

    class MeshRenderer : public Renderer {
    public:
        CLASS("UnityEngine", "MeshRenderer");
    };


    class SkinSet {
    public:
        CLASS("", "SkinSet");

        FIELD(BodyMaterial, "BodyMaterial", Material*);
        FIELD(HeadMaterial, "HeadMaterial", Material*);
        FIELD(EyeMaterial, "EyeMaterial", Material*);
    };

    class InputMessage {
    public:
        CLASS("", "InputMessage");
        FIELD(mouseDelta, "mouseDelta", Vector3);
        FIELD(aimAngles, "aimAngles", Vector3);
        FIELD(buttons, "buttons", int);
        FIELD(ShouldPool, "ShouldPool", bool);
        FIELD(_disposed, "_disposed", bool);
    };

    class BaseCombatEntity : public BaseEntity {
    public:
        CLASS("", "BaseCombatEntity");
        FIELD(lifeState, "lifestate", LifeState);
        FIELD(health, "_health", float);
        FIELD(maxHealth, "_maxHealth", float);

        static inline void(*DoHitNotify_)(BaseCombatEntity*, HitInfo*, Projectile*) = nullptr;
        void DoHitNotify(BaseCombatEntity* entity, HitInfo* info, Projectile* prj) {
            return DoHitNotify_(entity, info, prj);
        }
    };

    class InputState {
    public:
        CLASS("", "InputState");
        FIELD(current, "current", InputMessage*);
        FIELD(previous, "previous", InputMessage*);
    };

    class PlayerInput {
    public:
        CLASS("", "PlayerInput");
        FIELD(state, "state", InputState*);
        FIELD(bodyAngles, "bodyAngles", Vector3);
        FIELD(headAngles, "headAngles", Vector3);
    };

    class PlayerEyes {
    public:
        CLASS("", "PlayerEyes");
        FIELD(bodyRotation, "bodyRotation", UnityEngine::Quaternion);
        FIELD(headAngles, "headAngles", Vector3);
        FIELD(position, "position", Vector3);
        FIELD(viewOffset, "viewOffset", Vector3);
        FIELD(rotation, "rotation", UnityEngine::Quaternion);

        STATIC_FIELD(EyeOffset, "EyeOffset", Vector3);
        METHOD(MovementForward, "MovementForward", Vector3);
        METHOD(MovementRight, "MovementRight", Vector3);

        static inline Vector3(*BodyLeanOffset_)(PlayerEyes*) = nullptr;
        Vector3 BodyLeanOffset() {
            return BodyLeanOffset_(this);
        }
        static inline void(*DoFirstPersonCamera_)(PlayerEyes*, UnityEngine::Component*) = nullptr;
        void DoFirstPersonCamera(UnityEngine::Component* cam) {
            return DoFirstPersonCamera_(this, cam);
        }
    };

    class BaseProjectile : public AttackEntity {
    public:
        class Magazine {
        public:
            CLASS("", "BaseProjectile+Magazine");
            FIELD(ammoType, "ammoType", void*);
            FIELD(contents, "contents", int);
            FIELD(capacity, "capacity", int);
        };

        CLASS("", "BaseProjectile");
        FIELD(primaryMagazine, "primaryMagazine", Magazine*);
        FIELD(NoiseRadius, "NoiseRadius", float);
        FIELD(damageScale, "damageScale", float);
        FIELD(distanceScale, "distanceScale", float);
        FIELD(projectileVelocityScale, "projectileVelocityScale", float);
        FIELD(automatic, "automatic", bool);
        FIELD(usableByTurret, "usableByTurret", bool);
        FIELD(turretDamageScale, "turretDamageScale", float);
        FIELD(MuzzlePoint, "MuzzlePoint", UnityEngine::Transform*);
        FIELD(reloadTime, "reloadTime", float);
        FIELD(canUnloadAmmo, "canUnloadAmmo", bool);
        FIELD(fractionalReload, "fractionalReload", bool);
        FIELD(reloadStartDuration, "reloadStartDuration", float);
        FIELD(reloadFractionDuration, "reloadFractionDuration", float);
        FIELD(reloadEndDuration, "reloadEndDuration", float);
        FIELD(aimSway, "aimSway", float);
        FIELD(aimSwaySpeed, "aimSwaySpeed", float);
        FIELD(aimCone, "aimCone", float);
        FIELD(hipAimCone, "hipAimCone", float);
        FIELD(aimconePenaltyPerShot, "aimconePenaltyPerShot", float);
        FIELD(aimConePenaltyMax, "aimConePenaltyMax", float);
        FIELD(aimconePenaltyRecoverTime, "aimconePenaltyRecoverTime", float);
        FIELD(aimconePenaltyRecoverDelay, "aimconePenaltyRecoverDelay", float);
        FIELD(stancePenaltyScale, "stancePenaltyScale", float);
        FIELD(hasADS, "hasADS", bool);
        FIELD(noAimingWhileCycling, "noAimingWhileCycling", bool);
        FIELD(manualCycle, "manualCycle", bool);
        FIELD(needsCycle, "needsCycle", bool);
        FIELD(isCycling, "isCycling", bool);
        FIELD(aiming, "aiming", bool);
        FIELD(useEmptyAmmoState, "useEmptyAmmoState", bool);
        FIELD(isBurstWeapon, "isBurstWeapon", bool);
        FIELD(canChangeFireModes, "canChangeFireModes", bool);
        FIELD(defaultOn, "defaultOn", bool);
        FIELD(internalBurstRecoilScale, "internalBurstRecoilScale", float);
        FIELD(internalBurstFireRateScale, "internalBurstFireRateScale", float);
        FIELD(internalBurstAimConeScale, "internalBurstAimConeScale", float);
        FIELD(resetDuration, "resetDuration", float);
        FIELD(numShotsFired, "numShotsFired", int);
        FIELD(nextReloadTime, "nextReloadTime", float);
        FIELD(startReloadTime, "startReloadTime", float);
        FIELD(stancePenalty, "stancePenalty", float);
        FIELD(aimconePenalty, "aimconePenalty", float);
        FIELD(cachedModHash, "cachedModHash", uint32_t);
        FIELD(sightAimConeScale, "sightAimConeScale", float);
        FIELD(sightAimConeOffset, "sightAimConeOffset", float);
        FIELD(hipAimConeScale, "hipAimConeScale", float);
        FIELD(hipAimConeOffset, "hipAimConeOffset", float);
        FIELD(isReloading, "isReloading", bool);
        FIELD(timeSinceReloadFinished, "timeSinceReloadFinished", float);
        FIELD(swaySampleTime, "swaySampleTime", float);
        FIELD(lastShotTime, "lastShotTime", float);
        FIELD(reloadPressTime, "reloadPressTime", float);
        FIELD(fractionalReloadDesiredCount, "fractionalReloadDesiredCount", int);
        FIELD(fractionalReloadNumAdded, "fractionalReloadNumAdded", int);
        FIELD(currentBurst, "currentBurst", int);
        FIELD(triggerReady, "triggerReady", bool);
        FIELD(nextHeightCheckTime, "nextHeightCheckTime", float);
        FIELD(cachedUnderground, "cachedUnderground", bool);
        FIELD(createdProjectiles, "createdProjectiles", System::List<Projectile*>*);

        bool HasReloadCooldown() {
            return SDK::UnityEngine::Time::GetTime() < this->nextReloadTime();
        }

        float CalculateCooldownTime(float nextTime, float cooldown) {
            float current = SDK::UnityEngine::Time::GetTime();
            float ret = nextTime;
            if (ret < 0.f) {
                ret = max(0.f, current + cooldown);
            }
            else if (current - ret <= 0.f) {
                ret = min(ret + cooldown, current + cooldown);
            }
            else {
                ret = max(ret + cooldown, current + cooldown);
            }
            return ret;
        }

        METHOD(doAttack, "DoAttack", void);
        METHOD(LaunchProjectile, "LaunchProjectile", void);
        METHOD(ShotFired, "ShotFired", void);
        METHOD(UpdateAmmoDisplay, "UpdateAmmoDisplay", void);
        METHOD(DidAttackClientside, "DidAttackClientside", void);
        METHOD(BeginCycle, "BeginCycle", void);

        static inline Vector3(*GetModifiedAimConeDirection_)(float, Vector3, bool) = nullptr;
        Vector3 GetModifiedAimConeDirection(float accuracy, Vector3 inputVec, bool memoize) {
            if (GetModifiedAimConeDirection_)
                return GetModifiedAimConeDirection_(accuracy, inputVec, memoize);
            return Vector3();
        }
    };

    class Projectile : public BaseMonoBehaviour {
    public:
        CLASS("", "Projectile");
        FIELD(traveledDistance, "traveledDistance", float);
        FIELD(traveledTime, "traveledTime", float);
        FIELD(currentVelocity, "currentVelocity", Vector3);
        FIELD(previousPosition, "previousPosition", Vector3);
        FIELD(integrity, "integrity", float);
        FIELD(gravityModifier, "gravityModifier", float);
        FIELD(projectileID, "projectileID", int);
        FIELD(hitTest, "hitTest", HitTest*);
        FIELD(hitInfo, "hitInfo", HitInfo*);
        FIELD(drag, "drag", float);
        FIELD(owner, "owner", BasePlayer*);
        FIELD(thickness, "thickness", float);
        FIELD(currentPosition, "currentPosition", Vector3);
        FIELD(swimScale, "swimScale", Vector3);
        FIELD(swimSpeed, "swimSpeed", Vector3);

        static inline bool(*DoHit_)(Projectile*, HitTest*, Vector3, Vector3) = nullptr;
        bool DoHit(HitTest* test, Vector3 point, Vector3 world) {
            return DoHit_(this, test, point, world);
        }

        static inline void(*Update_)(Projectile*) = nullptr;
        void Update() {
            if (Update_) Update_(this);
        }

        static inline void(*DoMovement_)(Projectile*, float) = nullptr;
        void DoMovement(float deltaTime) {
            this->swimScale() = Vector3(0.f, 0.f, 0.f);
            this->swimSpeed() = Vector3(0.f, 0.f, 0.f);
            return DoMovement_(this, deltaTime);
        }
    };
    namespace ProtoBuf {
        class PlayerProjectileUpdate {
        public:
            int& projectileId() {
                return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0x10);
            }

            Vector3& curPosition() {
                return *reinterpret_cast<Vector3*>(reinterpret_cast<uintptr_t>(this) + 0x14);
            }

            Vector3& curVelocity() {
                return *reinterpret_cast<Vector3*>(reinterpret_cast<uintptr_t>(this) + 0x20);
            }

            float& travelTime() {
                return *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + 0x2C);
            }

            static PlayerProjectileUpdate* New() {
                static Dissector::IL2CPP::IL2CPPClass* klass = Dissector::FindClass("ProtoBuf", "PlayerProjectileUpdate");
                if (!klass) return nullptr;

                auto* obj = reinterpret_cast<PlayerProjectileUpdate*>(Dissector::IL2CPP::CreateNewObject(klass));
                if (!obj) return nullptr;

                static auto* ctorMethod = Dissector::FindMethod(klass, ".ctor", 0, "System.Void");
                if (ctorMethod && ctorMethod->methodPtr) {
                    using ctor_fn = void(*)(void*);
                    reinterpret_cast<ctor_fn>(ctorMethod->methodPtr)(obj);
                }

                return obj;
            }
        };
    }

    class Stream;
    class PlayerTick {
    public:
        CLASS("", "PlayerTick");
        FIELD(Position, "Position", Vector3);

        static inline void(*WriteToStreamDelta_)(PlayerTick*, Stream*, PlayerTick*) = nullptr;

        void WriteToStreamDelta(Stream* stream, PlayerTick* previous) {
            return WriteToStreamDelta_(this, stream, previous);
        }
    };

    class BowWeapon : public BaseProjectile {
    public:
        CLASS("", "BowWeapon");
        FIELD(arrowBack, "arrowBack", float);
        FIELD(attackReady, "attackReady", bool);
    };

    class FlintStrikeWeapon : public BaseProjectile {
    public:
        CLASS("", "FlintStrikeWeapon");

        FIELD(successFraction, "successFraction", float);
        FIELD(_didSparkThisFrame, "_didSparkThisFrame", bool);

        static inline void(*DoAttack_)(FlintStrikeWeapon*) = nullptr;
        void DoAttack() {
            if (DoAttack_) DoAttack_(this);
        }
    };

    class BaseMountable {
    public:
        CLASS("", "BaseMountable");
        FIELD(canWieldItems, "canWieldItems", bool);

        static inline Vector3(*EyePositionForPlayer_)(BaseMountable*, BasePlayer*, UnityEngine::Quaternion) = nullptr;
        Vector3 EyePositionForPlayer(BasePlayer* ply, UnityEngine::Quaternion rot) {
            return EyePositionForPlayer_(this, ply, rot);
        }
    };

    class ModelState;

    class Collider : public UnityEngine::Component {
    public:
        CLASS("UnityEngine", "Collider");
    };
    class CapsuleCollider : public Collider {
    public:
        CLASS("UnityEngine", "CapsuleCollider");

        METHOD(get_radius, "get_radius", float);
        METHOD(set_radius, "set_radius", void, float);

        METHOD(get_height, "get_height", float);
        METHOD(set_height, "set_height", void, float);

        METHOD(get_center, "get_center", Vector3);
        METHOD(set_center, "set_center", void, Vector3);

        METHOD(get_direction, "get_direction", int);
        METHOD(set_direction, "set_direction", void, int);
    };

    class RigidBody {
    public:
        CLASS("", "RigidBody");
        METHOD(velocity, "get_velocity", Vector3);
        METHOD(set_velocity, "set_velocity", void, Vector3);
    };

    class BaseMovement : public UnityEngine::MonoBehaviour {
    public:
        CLASS("", "BaseMovement");
        FIELD(TargetMovement, "<TargetMovement>k__BackingField", Vector3);
        FIELD(Running, "<Running>k__BackingField", float);

        void Jump(ModelState* state) {
            if (!this) return;

            static void* methodPtr = nullptr;
            if (!methodPtr) {
                auto* movementClass = Dissector::FindClass("Assembly-CSharp", "PlayerWalkMovement");
                if (!movementClass) movementClass = Dissector::FindClass("", "PlayerWalkMovement");

                if (movementClass) {
                    auto* method = Dissector::FindMethod(movementClass, "Jump", 2, "System.Void");
                    if (method) methodPtr = method->methodPtr;
                }
            }
            if (!methodPtr) return;

            using fn_t = void(__fastcall*)(BaseMovement*, ModelState*, bool, void*);
            reinterpret_cast<fn_t>(methodPtr)(this, state, false, nullptr);
        }
    };

    class PlayerWalkMovement : public BaseMovement {
    public:
        CLASS("", "PlayerWalkMovement");

        FIELD(body, "body", RigidBody*);
        FIELD(groundAngle, "groundAngle", float);
        FIELD(groundAngleNew, "groundAngleNew", float);
        FIELD(gravityMultiplier, "gravityMultiplier", float);
        FIELD(gravityMultiplierSwimming, "gravityMultiplierSwimming", float);
        FIELD(gravityTestRadius, "gravityTestRadius", float);
        FIELD(maxAngleWalking, "maxAngleWalking", float);
        FIELD(maxAngleClimbing, "maxAngleClimbing", float);

        FIELD(Grounded, "Grounded", bool);
        FIELD(flying, "flying", bool);
        FIELD(swimming, "swimming", bool);
        FIELD(Ducking, "Ducking", float);
        FIELD(climbing, "climbing", bool);
        FIELD(sliding, "sliding", bool);
        FIELD(jumping, "jumping", bool);
        FIELD(jumpTime, "jumpTime", float);
        FIELD(landTime, "landTime", float);
        FIELD(groundTime, "groundTime", float);

        FIELD(velocity, "Velocity", Vector3);
        FIELD(capsule, "capsule", CapsuleCollider*);

        void TeleportTo(Vector3 pos) {
            if (!this) return;

            static Dissector::IL2CPP::IL2CPPClass* movementClass = nullptr;
            if (!movementClass) {
                movementClass = Dissector::FindClass("Assembly-CSharp", "PlayerWalkMovement");
                if (!movementClass) {
                    movementClass = Dissector::FindClass("", "PlayerWalkMovement");
                }
            }
            if (!movementClass) return;

            static void* methodPtr = nullptr;
            if (!methodPtr) {
                auto* method = Dissector::FindMethod(movementClass, "TeleportTo", 2, "System.Void");
                if (method) {
                    methodPtr = method->methodPtr;
                }
            }
            if (!methodPtr) return;

            using fn_t = void(__fastcall*)(PlayerWalkMovement*, Vector3, void*);
            reinterpret_cast<fn_t>(methodPtr)(this, pos, nullptr);
        }
       

        static inline void(*UpdateVelocity_)(PlayerWalkMovement*) = nullptr;
        void UpdateVelocity() {
            if (UpdateVelocity_) UpdateVelocity_(this);
        }
    };

    class StringPool {
    public:
        CLASS("Assembly-CSharp", "StringPool");

        STATIC_METHOD(GetID, "Get", uint32_t, System::String*);
        STATIC_METHOD(GetString, "Get", System::String*, uint32_t);

        static uint32_t Get(const char* str) {
            auto* il2cppString = System::String::New(str);
            if (!il2cppString) return 0;

            return GetID(il2cppString);
        }

        static System::String* Get(uint32_t i) {
            return GetString(i);
        }
    };

    class ModelState {
    public:
        enum class Flags : int {
            Ducked = 1,
            Jumped = 2,
            OnGround = 4,
            Sleeping = 8,
            Sprinting = 16,
            OnLadder = 32,
            Flying = 64,
            Aiming = 128,
            Prone = 256,
            Mounted = 512,
            Relaxed = 1024,
            OnPhone = 2048,
        };
        CLASS("", "ModelState");

        FIELD(flags, "flags", int);
        FIELD(poseType, "poseType", int);

        METHOD(SetFlag, "SetFlag", void, Flags, bool);
        METHOD(GetFlag, "GetFlag", bool, Flags);
    };
    class BasePlayer : public BaseCombatEntity {
    public:
        enum PlayerFlags : int {
            Unused1 = 1,
            Unused2 = 2,
            IsAdmin = 4,
            ReceivingSnapshot = 8,
            Sleeping = 16,
            Spectating = 32,
            Wounded = 64,
            IsDeveloper = 128,
            Connected = 256,
            ThirdPersonViewmode = 1024,
            EyesViewmode = 2048,
            ChatMute = 4096,
            NoSprint = 8192,
            Aiming = 16384,
            DisplaySash = 32768,
            Relaxed = 65536,
            SafeZone = 131072,
            ServerFall = 262144,
            Workbench1 = 1048576,
            Workbench2 = 2097152,
            Workbench3 = 4194304,
        };

        CLASS("", "BasePlayer");

        FIELD(displayName, "_displayName", System::String*);
        FIELD(playerModel, "playerModel", PlayerModel*);
        FIELD(movement, "movement", BaseMovement*);
        FIELD(input, "input", PlayerInput*);
        FIELD(eyes, "eyes", PlayerEyes*);
        FIELD(playerFlags, "playerFlags", int);
        FIELD(Online, "Online", bool);
        FIELD(Position, "Position", Vector3);
        FIELD(clientTickInterval, "clientTickInterval", float);
        FIELD(currentTeam, "currentTeam", uint64_t);
        FIELD(clActiveItem, "clActiveItem", uint64_t);
        FIELD(userID, "userID", uint64_t);
        FIELD(inventory, "inventory", PlayerInventory*);
        FIELD(mounted, "mounted", BaseMountable*);
        FIELD(modelState, "modelState", ModelState*);
        FIELD(Frozen, "Frozen", bool);
        FIELD(lastSentTick, "lastSentTick", PlayerTick*);
        FIELD(lastSentTickTime, "lastSentTickTime", float);

        METHOD(GetHeight, "GetHeight", float);
        METHOD(GetRadius, "GetRadius", float);
        METHOD(GetMaxSpeed, "GetMaxSpeed", float);
        METHOD(SendProjectileUpdate, "SendProjectileUpdate", void);
        METHOD(GetJumpHeight, "GetJumpHeight", float);
        METHOD(GetMountVelocity, "GetMountVelocity", Vector3);
        METHOD(Menu_AssistPlayer, "Menu_AssistPlayer", void);

        static inline void(*ClientInput_)(BasePlayer*, uintptr_t) = nullptr;
        void ClientInput(uintptr_t state) {
            if (ClientInput_)
                ClientInput_(this, state);
        }

        static inline void(*SendClientTick_)(BasePlayer*) = nullptr;
        void SendClientTick() {
            return SendClientTick_(this);
        }

        float MaxVelocity() {
            if (!this) return 0.f;

            if (this->mounted())
                return this->GetMaxSpeed() * 4;

            return this->GetMaxSpeed();
        }
        float GetHeight() {
            if (!this) return 0.f;
            auto* state = this->modelState();
            bool ducked = state ? state->GetFlag(ModelState::Flags::Ducked) : false;
            return GetHeight(ducked);
        }

        Vector3 GetParentVelocity() {
            if (!this) return { 0, 0, 0 };

            static Dissector::IL2CPP::IL2CPPClass* entityClass = nullptr;
            if (!entityClass) {
                entityClass = Dissector::FindClass("Assembly-CSharp", "BaseEntity");
                if (!entityClass) entityClass = Dissector::FindClass("", "BaseEntity");
            }
            if (!entityClass) return { 0, 0, 0 };

            static void* methodPtr = nullptr;
            if (!methodPtr) {
                auto* method = Dissector::FindMethod(entityClass, "GetParentVelocity", 0, "Vector3");
                if (method) methodPtr = method->methodPtr;
            }
            if (!methodPtr) return { 0, 0, 0 };

            using fn_t = Vector3(__fastcall*)(BasePlayer*, void*);
            return reinterpret_cast<fn_t>(methodPtr)(this, nullptr);
        }

        bool GetKeyState(RustButton b) {
            return ((input()->state()->current()->buttons() & (int)b) == (int)b);
        }
        bool HasPlayerFlag(PlayerFlags flag) {
            if (!this) return false;

            return (playerFlags() & flag) == flag;
        }
        void add_modelstate_flag(ModelState::Flags flag) {
            int flags = this->modelState()->flags();
            this->modelState()->flags() = flags |= (int)flag;
        }
    };

    class LocalPlayer {
    public:
        CLASS("", "LocalPlayer");
        STATIC_METHOD(GetEntity, "get_Entity", BasePlayer*);
    };

    class Camera {
    public:
        CLASS("UnityEngine", "Camera");

        STATIC_METHOD(get_main, "get_main", Camera*);
        METHOD(set_aspect, "set_aspect", void, float);

        static void SetAspect(float aspect) {
            auto* main = get_main();
            if (!main) return;

            main->set_aspect(aspect);
        }
    };

    inline void AssistPlayer(BasePlayer* player) {
        static float LastPickup = 0.0f;
        if (LocalPlayer::GetEntity()->lastSentTickTime() > LastPickup + 0.5f) {
            player->Menu_AssistPlayer(LocalPlayer::GetEntity());
            LastPickup = LocalPlayer::GetEntity()->lastSentTickTime();
        }
    }
}

struct RustPlayer {
    SDK::BaseEntity* pawn;
    bool is_visible;
    bool is_npc;
};
struct RustResource {
    uintptr_t pawn;
    Vector3 position;
    string name;

    OreType type;
    bool is_hemp;
};

namespace entity_data {
    inline std::vector<RustPlayer> players_list;
    inline vector<RustResource> resource_list;
    inline SDK::BasePlayer* local_player;
}