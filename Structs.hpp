#pragma once
#include <cstdint>
#include "math.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>

using namespace std;

struct UnityEngine_TransformAccessReadOnly {
	uintptr_t TransformData;
	uint32_t Index;
};
struct UnityEngine_TransformData {
	uintptr_t TransformArray;
	uintptr_t TransformIndices;
};

enum OreType : int {
	stone,
	metal,
	sulfur
};

enum TextAnchor {
	UpperLeft = 0,
	UpperCenter = 1,
	UpperRight = 2,
	MiddleLeft = 3,
	MiddleCenter = 4,
	MiddleRight = 5,
	LowerLeft = 6,
	LowerCenter = 7,
	LowerRight = 8
};
enum RustButton {
	FORWARD = 2,
	BACKWARD = 4,
	LEFT = 8,
	RIGHT = 16,
	JUMP = 32,
	DUCK = 64,
	SPRINT = 128,
	USE = 256,
	FIRE_PRIMARY = 1024,
	FIRE_SECONDARY = 2048,
	RELOAD = 8192,
	FIRE_THIRD = 134217728,
};
enum EventType {
	MouseDown = 0,
	MouseUp = 1,
	MouseMove = 2,
	MouseDrag = 3,
	KeyDown = 4,
	KeyUp = 5,
	ScrollWheel = 6,
	Repaint = 7,
	Layout = 8,
	DragUpdated = 9,
	DragPerform = 10,
	Ignore = 11,
	Used = 12,
	ValidateCommand = 13,
	ExecuteCommand = 14,
	DragExited = 15,
	ContextClick = 16,
	MouseEnterWindow = 20,
	MouseLeaveWindow = 21,
	TouchDown = 30,
	TouchUp = 31,
	TouchMove = 32,
	TouchEnter = 33,
	TouchLeave = 34,
	TouchStationary = 35
};

enum Tag : int {
	main_camera = 5,
	player = 6,
	resource = 0,
	terrain = 20001,
	misc = 20006,
	animal = 20008,
	corpse = 20009,
	monument = 20015,
	helicopter = 20017,
	skydometag = 20011
};

enum UIBlackoutOverlay : int {
	FULLBLACK = 0,
	BINOCULAR = 1,
	SCOPE = 2,
	HELMETSLIT = 3,
	SNORKELGOGGLE = 4,
	NVG = 5,
	FULLWHITE = 6,
	SUNGLASSES = 7,
	NONE = 64
};

enum Layer : uint32_t {
	Default = 0,
	TransparentFX = 1,
	Ignore_Raycast = 2,
	Reserved1 = 3,
	Water = 4,
	UI = 5,
	Reserved2 = 6,
	Reserved3 = 7,
	Deployed = 8,
	Ragdoll = 9,
	Invisible = 10,
	AI = 11,
	PlayerMovement = 12,
	Vehicle_Detailed = 13,
	Game_Trace = 14,
	Vehicle_World = 15,
	WorldL = 16,
	Player_Server = 17,
	Trigger = 18,
	Player_Model_Rendering = 19,
	Physics_Projectile = 20,
	Construction = 21,
	Construction_Socket = 22,
	Terrain = 23,
	Transparent = 24,
	Clutter = 25,
	Debris = 26,
	Vehicle_Large = 27,
	Prevent_Movement = 28,
	Prevent_Building = 29,
	Tree = 30,
	Unused3 = 31
};

enum CameraMode : int
{
	FirstPerson = 0,
	ThirdPerson = 1,
	Eyes = 2,
	FirstPersonWithArms = 3,
	Last = 3
};

enum BoneList : int {
	pelvis = 0,
	l_hip = 1,
	l_knee = 2,
	l_foot = 3,
	l_toe = 4,
	l_ankle_scale = 5,
	penis = 6,
	GenitalCensor = 7,
	GenitalCensor_LOD0 = 8,
	Inner_LOD0 = 9,
	GenitalCensor_LOD1 = 11,
	GenitalCensor_LOD2 = 12,
	r_hip = 13,
	r_knee = 14,
	r_foot = 15,
	r_toe = 16,
	r_ankle_scale = 17,
	spine1 = 18,
	spine1_scale = 19,
	spine2 = 20,
	spine3 = 21,
	spine4 = 22,
	l_clavicle = 23,
	l_upperarm = 24,
	l_forearm = 25,
	l_hand = 26,
	l_index1 = 27,
	l_index2 = 28,
	l_index3 = 29,
	l_little1 = 30,
	l_little2 = 31,
	l_little3 = 32,
	l_middle1 = 33,
	l_middle2 = 34,
	l_middle3 = 35,
	l_prop = 36,
	l_ring1 = 37,
	l_ring2 = 38,
	l_ring3 = 39,
	l_thumb1 = 40,
	l_thumb2 = 41,
	l_thumb3 = 42,
	IKtarget_righthand_min = 43,
	IKtarget_righthand_max = 44,
	l_ulna = 45,
	neck = 46,
	head = 47,
	jaw = 48,
	eyeTranform = 49,
	l_eye = 50,
	l_Eyelid = 51,
	r_eye = 52,
	r_Eyelid = 53,
	r_clavicle = 54,
	r_upperarm = 55,
	r_forearm = 56,
	r_hand = 57,
	r_index1 = 58,
	r_index2 = 59,
	r_index3 = 60,
	r_little1 = 61,
	r_little2 = 62,
	r_little3 = 63,
	r_middle2 = 65,
	r_middle3 = 66,
	r_prop = 67,
	r_ring1 = 68,
	r_ring2 = 69,
	r_ring3 = 70,
	r_thumb1 = 71,
	r_thumb2 = 72,
	r_thumb3 = 73,
	IKtarget_lefthand_min = 74,
	IKtarget_lefthand_max = 75,
	r_ulna = 76,
	l_breast = 77,
	r_breast = 78,
	BoobCensor = 79,
	BreastCensor_LOD0 = 80,
	BreastCensor_LOD1 = 83,
	BreastCensor_LOD2 = 84
};

enum LifeState : int {
	Alive = 0,
	Dead = 1
};

enum WeaponType : int {
	AssultRifle = 1545779598,
	IceAssultRifle = -1335497659,
	Lr300 = -1812555177,
	M249 = -2069578888,
	M39 = -28201841,
	Sar = -904863145,
	HMLMG = -1214542497,
	L96 = -778367295,
	BoltAction = 1588298435,
	CustomSmg = 1796682209,
	Mp5 = 1318558775,
	Thompson = -1758372725,
	Python = 1373971859,
	SemiPistol = 818877484,
	Revolver = 649912614,
	M92 = -852563019,
	Nailgun = 1953903201,
	Eoka = -75944661,
	Spas12 = -41440462,
	PumpShotgun = 795371088,
	WaterPipe = -1367281941,
	DoubleBarrel = -765183617,
	Bow = 1443579727,
	CompoundBow = 884424049,
	CrossBow = 1965232394,
	GrenadeLauncher = -1123473824,
	RocketLauncher = 442886268,
	SnowBallGun = 1103488722,
	FlameThrower = -1215753368,
	JackHammer = 1488979457,
	IcePickaxe = -1780802565,
	MetalPickaxe = -1302129395,
	StonePickaxe = 171931394
};

enum Category {
	Shirt,
	Pants = 5,
	Jacket = 1,
	Hat,
	Mask,
	Footwear,
	Weaponn = 6,
	Miscc,
	Deployable = 9,
	Gloves
};