#pragma once
#include "Math.hpp"
#undef GetClassName

namespace SDK {
	namespace UnityEngine {
		class Color {
		public:
			float r, g, b, a;

			Color() : r(0), g(0), b(0), a(1) {}
			Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
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

		class Rect {
		public:
			float m_XMin; // 0x0
			float m_YMin; // 0x4
			float m_Width; // 0x8
			float m_Height; // 0xC

			Rect(float x, float y, float width, float height) {
				m_XMin = x;
				m_YMin = y;
				m_Width = width;
				m_Height = height;
			}

			Rect(Vector2 position, Vector2 size) {
				m_XMin = position.x;
				m_YMin = position.y;
				m_Width = size.x;
				m_Height = size.y;
			}
		};

		class Quaternion {
		public:
			CLASS("UnityEngine", "Quaternion");

			float x; // 0x0
			float y; // 0x4
			float z; // 0x8
			float w; // 0xC
		};

		struct Ray {
		public:
			Vector3 m_Origin;
			Vector3 m_Direction;

			Ray() : m_Origin(Vector3(0, 0, 0)), m_Direction(Vector3(0, 0, 0)) {}
			Ray(Vector3 origin, Vector3 direction) : m_Origin(origin), m_Direction(direction) {}
		};

		class Event {
		public:
			CLASS("UnityEngine", "Event");
			enum  EventType {
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

			STATIC_METHOD(GetCurrent, "get_current", Event*);
			METHOD(GetType, "get_type", EventType);
		};

		class Screen {
		public:
			CLASS("UnityEngine", "Screen");

			STATIC_METHOD(GetWidth, "get_width", int);
			STATIC_METHOD(GetHeight, "get_height", int);
		};

		class Time {
		public:
			CLASS("UnityEngine", "Time");

			STATIC_METHOD(GetTime, "get_time", float);
			STATIC_METHOD(GetDeltaTime, "get_deltaTime", float);
			STATIC_METHOD(GetSmoothDeltaTime, "get_smoothDeltaTime", float);
			STATIC_METHOD(GetFrameCount, "get_frameCount", int);
			STATIC_METHOD(GetRealtimeSinceStartup, "get_realtimeSinceStartup", float);
			STATIC_METHOD(GetFixedTime, "get_fixedTime", float);
			STATIC_METHOD(GetFixedDeltaTime, "get_fixedDeltaTime", float);
		};

		class Object {
		public:
			CLASS("UnityEngine", "Object");

			METHOD(GetName, "get_name", System::String*);
		};

		class Component;
		class GameObject : public Object {
		public:
			CLASS("UnityEngine", "GameObject");

			METHOD(AddComponent, "AddComponent", Component*, System::Type*);
			METHOD(GetComponent, "GetComponent", Component*, System::Type*);

			METHOD(GetComponentsInChildren, "GetComponentsInChildren", System::Array<Component*>*, System::Type*);
		};

		class Transform;
		class Component : public Object {
		public:
			CLASS("UnityEngine", "Component");

			METHOD(GetGameObject, "get_gameObject", GameObject*);
			METHOD(GetTransform, "get_transform", Transform*);
			METHOD(GetComponent, "GetComponent", Component*, System::Type*);

			char* GetClassName() {
				if (!this) return nullptr;

				auto oc = *reinterpret_cast<uint64_t*>(this);
				if (!oc) return nullptr;

				return *reinterpret_cast<char**>(oc + 0x10);
			}

			bool IsFrom(const char* fromClassName) {
				if (!this)
					return false;

				const char* className = this->GetClassName();
				if (strcmp(className, fromClassName) != NULL)
					return false;

				return true;
			}
		};

		class SafeExecution {
		public:
			static int fail(unsigned int code, struct _EXCEPTION_POINTERS* ep) {
				if (code == EXCEPTION_ACCESS_VIOLATION) {
					return EXCEPTION_EXECUTE_HANDLER;
				}
				else {
					return EXCEPTION_CONTINUE_SEARCH;
				};
			}
		public:
			template<typename T = void*, typename R = void*, typename... Args>
			static T Execute(uint64_t ptr, R ret, Args... args) {
				__try {
					return reinterpret_cast<T(__stdcall*)(Args...)>(ptr)(args...);
				}
				__except (fail(GetExceptionCode(), GetExceptionInformation())) {
					return ret;
				}
			}
		};

		class AssetBundle : public Object {
		public:
			CLASS("UnityEngine", "AssetBundle");

			METHOD(Unload, "Unload", void, bool);
			METHOD(GetAllAssetNames, "GetAllAssetNames", System::Array<System::String*>*);
			METHOD(LoadAssetInternal, "LoadAsset", Object*, System::String*);
			METHOD(LoadAssetWithTypeInternal, "LoadAsset", Object*, System::String*, System::Type*);
			STATIC_METHOD(LoadFromFileInternal, "LoadFromFile", AssetBundle*, System::String*);

			template<typename T = Object>
			T* LoadAsset(const char* name) {
				if (!this || !name) return nullptr;
				return reinterpret_cast<T*>(LoadAssetInternal(System::String::New(name)));
			}

			template<typename T = Object>
			T* LoadAsset(const char* name, System::Type* type) {
				if (!this || !name) return nullptr;
				return reinterpret_cast<T*>(LoadAssetWithTypeInternal(System::String::New(name), type));
			}

			static AssetBundle* LoadFromFile(const char* path) {
				if (!path) return nullptr;
				return LoadFromFileInternal(System::String::New(path));
			}
		};

		class ConsoleSystem {
		public:
			struct Option {
				static Option* Client() {
					static auto method = get_option_method("get_Client", 0, "Option");
					if (method) {
						using fn_t = Option * (*)(void);
						return reinterpret_cast<fn_t>(method)();
					}
					return nullptr;
				}
				bool IsFromServer() {
					return *reinterpret_cast<bool*>(this + 0x6);
				}
				static Option* Quiet() {
					static auto method = get_option_method("Quiet", 0, "Option");
					if (method) {
						using fn_t = Option * (*)(void);
						return reinterpret_cast<fn_t>(method)();
					}
					return nullptr;
				}
			private:
				static void* get_option_method(const char* methodName, int paramCount, const char* returnType = "System.Void") {
					static Dissector::IL2CPP::IL2CPPClass* optionClass = nullptr;

					if (!optionClass) {
						optionClass = Dissector::FindClass("Facepunch.Console", "Option");
						if (!optionClass) {
							optionClass = Dissector::FindClass("", "Option");
						}
					}

					if (!optionClass)
						return nullptr;

					auto* method = Dissector::FindMethod(optionClass, methodName, paramCount, returnType);
					if (!method)
						return nullptr;

					return method->methodPtr;
				}
			};

			static void WriteLine(System::String* value) {
				static auto method = get_console_method("WriteLine", 1, "System.Void");
				if (method) {
					using fn_t = void(*)(System::String*);
					reinterpret_cast<fn_t>(method)(value);
				}
			}
		private:
			static void* get_console_method(const char* methodName, int paramCount, const char* returnType = "System.Void") {
				static Dissector::IL2CPP::IL2CPPClass* consoleClass = nullptr;

				if (!consoleClass) {
					consoleClass = Dissector::FindClass("mscorlib", "System::Console");
					if (!consoleClass) {
						consoleClass = Dissector::FindClass("mscorlib", "System.Console");
					}
					if (!consoleClass) {
						consoleClass = Dissector::FindClass("", "Console");
					}
				}

				if (!consoleClass)
					return nullptr;

				auto* method = Dissector::FindMethod(consoleClass, methodName, paramCount, returnType);
				if (!method)
					return nullptr;

				return method->methodPtr;
			}

		public:
			static inline System::String* (*Run_)(Option*, System::String*, System::Array<System::Object_*>*) = nullptr;
			static System::String* Run(Option* option, System::String* command, System::Array<System::Object_*>* args) {
				return Run_(option, command, args);
			}
		};

		class GamePhysics {
		public:
			enum  QueryTriggerInteraction {
				UseGlobal = 0,
				Ignore = 1,
				Collide = 2,
			};

			CLASS("", "GamePhysics")

			STATIC_METHOD(Verify, "Verify", bool, void*, float)
			STATIC_METHOD(LineOfSightRadius, "LineOfSightRadius", bool, Vector3, Vector3, int, float, float)
			STATIC_METHOD(LineOfSight, "LineOfSight", bool, Vector3, Vector3, int, float)
			STATIC_METHOD(CheckCapsule, "CheckCapsule", bool, Vector3, Vector3, float, int, QueryTriggerInteraction)
		};

		inline bool LineOfSight(Vector3 a, Vector3 b) {
			bool result = false;
			if (!a.Empty() && !b.Empty()) {
				int mask = setting::misc::wall_shot ? 2162688 | 8388608 | 134217728 : 1503731969 | 2162688 | 8388608 | 2097152 | 1 | 2097152 | 32768 | 65536;
				result = GamePhysics::LineOfSight(a, b, mask, 0.f) && GamePhysics::LineOfSight(b, a, mask, 0.f);
			}
			return result;
		}
		inline bool LineOfSightRadius(Vector3 p0, Vector3 p1, int layerMask, float radius) {
			bool result = GamePhysics::LineOfSightRadius(p0, p1, layerMask, radius, 0.f) && GamePhysics::LineOfSightRadius(p1, p0, layerMask, radius, 0.f);
			return result;
		}
		inline float GetDistanceToGround(Vector3 position) {
			if (position.Empty()) return 999.0f;

			Vector3 down = Vector3(0.0f, -1.0f, 0.0f);
			float radius = 0.35f;
			float height = 1.8f;
			float maxDistance = 30.0f;
			float step = 0.15f;

			int mask = 1503731969 | 8388608;

			Vector3 startPos = position + Vector3(0.0f, 0.1f, 0.0f);
			Vector3 capsuleOffset = Vector3(0.0f, height - (radius * 2.0f), 0.0f);

			for (float dist = 0.0f; dist < maxDistance; dist += step) {
				Vector3 currentBottom = startPos + (down * dist);
				Vector3 currentTop = currentBottom + capsuleOffset;

				if (UnityEngine::GamePhysics::CheckCapsule(currentBottom, currentTop, radius, mask,
					UnityEngine::GamePhysics::QueryTriggerInteraction::Ignore)) {
					return (dist - 0.1f) < 0.0f ? 0.0f : (dist - 0.1f);
				}
			}

			return 999.0f;
		}

		class Transform : public Component {
		public:
			CLASS("UnityEngine", "Transform");

			METHOD(up, "get_up", Vector3);
			METHOD(GetPosition, "get_position", Vector3);
			METHOD(get_localPosition, "get_localPosition", Vector3);
			METHOD(get_rotation, "get_rotation", Quaternion);

			METHOD(set_position, "set_position", void, Vector3);
			METHOD(set_localPosition, "set_localPosition", void, Vector3);
			METHOD(set_rotation, "set_rotation", void, Quaternion);
			METHOD(set_scale, "set_localScale", void, Vector3);

			METHOD(InverseTransformPoint, "InverseTransformPoint", Vector3, Vector3);
			METHOD(InverseTransformDirection, "InverseTransformDirection", Vector3, Vector3);
		};

		class Behaviour : public Component {
		public:
			CLASS("UnityEngine", "Behaviour");

		};

		class MonoBehaviour : public Behaviour {
		public:
			CLASS("UnityEngine", "MonoBehaviour");

		};

		class Color32 {
		public:
			uint8_t r, g, b, a;
		};

		class Texture {
		public:
			CLASS("UnityEngine", "Texture");

		};

		class Texture2D : public Texture {
		public:
			CLASS("UnityEngine", "Texture2D");

			STATIC_METHOD(GetWhiteTexture, "get_whiteTexture", Texture2D*);
			METHOD(get_width, "get_width", int);
			METHOD(get_height, "get_height", int);
			METHOD(get_format, "get_format", int);

			METHOD(GetRawTextureData, "GetRawTextureData", System::Array<uint8_t>*);
			METHOD(LoadImage, "LoadImage", bool, System::Array<uint8_t>*);

			System::Array<Color32>* GetPixels32() {
				using fn = System::Array<Color32> * (*)(Texture2D*);
				static auto ptr = reinterpret_cast<fn>(Dissector::FindMethod(TypeClass(), "GetPixels32", 0)->methodPtr);
				return ptr(this);
			}

			bool Apply() {
				using fn = bool(*)(Texture2D*, bool, bool);
				static auto ptr = reinterpret_cast<fn>(Dissector::FindMethod(TypeClass(), "Apply", 2)->methodPtr);
				return ptr(this, false, false);
			}

			bool Reinitialize(int width, int height) {
				using fn = bool(*)(Texture2D*, int, int, int, bool);
				static auto ptr = reinterpret_cast<fn>(Dissector::FindMethod(TypeClass(), "Reinitialize", 4)->methodPtr);
				return ptr(this, width, height, 5, false);
			}
		};

		class GUIContent {
		public:
			CLASS("UnityEngine", "GUIContent");

			STATIC_METHOD(Temp, "Temp", GUIContent*, System::String*);
		};

		class GUIStyle {
		public:
			CLASS("UnityEngine", "GUIStyle");

			METHOD(SetAlignment, "set_alignment", void, Enums::TextAnchor);
			METHOD(SetFontSize, "set_fontSize", void, int);

			Vector2 CalcSize(GUIContent* content) {
				Vector2 ret;

				typedef void(*CalcSizeFN)(GUIStyle*, GUIContent*, Vector2*);
				static uintptr_t methodOff = (uintptr_t)Dissector::FindMethod(Dissector::FindClass("UnityEngine", "GUIStyle"), "Internal_CalcSize_Injected", 2)->methodPtr;
				reinterpret_cast<CalcSizeFN>(methodOff)(this, content, &ret);

				return ret;
			}
		};

		class GUISkin {
		public:
			CLASS("UnityEngine", "GUISkin");

			METHOD(GetLabel, "get_label", GUIStyle*);
		};

		class GUI {
		public:
			CLASS("UnityEngine", "GUI");

			STATIC_METHOD(SetColor, "set_color", void, Color);
			STATIC_METHOD(Label, "Label", void, Rect, System::String*);
			STATIC_METHOD(DrawTexture, "DrawTexture", void, Rect, Texture*);
			STATIC_METHOD(GetSkin, "get_skin", GUISkin*);
		};

		class Sprite : public Object {
		public:
			CLASS("UnityEngine", "Sprite");

			METHOD(get_texture, "get_texture", Texture2D*);
			METHOD(get_rect, "get_rect", Rect);
			METHOD(get_pivot, "get_pivot", Vector2);
			METHOD(get_border, "get_border", Vector4);

			float GetTextureWidth() {
				auto tex = get_texture();
				return tex ? (float)tex->get_width() : 0.f;
			}

			float GetTextureHeight() {
				auto tex = get_texture();
				return tex ? (float)tex->get_height() : 0.f;
			}
		};

		class Resources {
		public:
			CLASS("UnityEngine", "Resources");

			STATIC_METHOD(FindObjectsOfTypeAll, "FindObjectsOfTypeAll", System::Array<Object*>*, System::Type*);

			template<typename T = Object>
			static T* Load(const char* path) {
				static auto loadMethod = Dissector::FindMethod(TypeClass(), "Load", 1);
				if (!loadMethod) return nullptr;

				using LoadFn = Object * (*)(System::String*);
				auto fn = reinterpret_cast<LoadFn>(loadMethod->methodPtr);
				return reinterpret_cast<T*>(fn(System::String::New(path)));
			}
		};

		class ImageConversion {
		public:
			CLASS("UnityEngine", "ImageConversion");

			STATIC_METHOD(LoadImage, "LoadImage", bool, Texture2D*, System::Array<uint8_t>*);
		};

		class Camera : public Component {
		public:
			CLASS("UnityEngine", "Camera");
			STATIC_METHOD(get_main, "get_main", Camera*);
			METHOD(ScreenToPoint, "ScreenToWorldPoint", Vector3, Vector3 position);
		};
	}
}