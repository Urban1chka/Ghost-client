#pragma once
#include "xorstr.hpp"

inline auto gameAssembly = GetModuleHandleA(xorstr_("GameAssembly.dll"));
#define ProcAddr(func) GetProcAddress(gameAssembly, func)

template<typename T, typename... Args>
inline T call(const char* func, Args... args) {
	return reinterpret_cast<T(__fastcall*)(Args...)>(ProcAddr(func))(args...);
}

namespace SDK {
	namespace System {
		class BufferList;

		class ListDictionary {
		public:
			BufferList* GetValues() {
				return *reinterpret_cast<BufferList**>(this + 0x28);
			}
		};

		class Object_ {
		public:

		};

		class BufferList {
		public:
			int GetSize() {
				return *reinterpret_cast<int*>(this + 0x10);
			}

			void* GetBuffer() {
				return *reinterpret_cast<void**>(this + 0x18);
			}

			void* GetArray(int index) {
				return *reinterpret_cast<void**>((uintptr_t)this->GetBuffer() + (0x20 + ((uintptr_t)index * 0x8)));
			}
		};

		template<typename T = void*>
		class List {
		public:
			T GetBuffer() {
				return *reinterpret_cast<T*>(this + 0x10);
			}

			T GetArray(int index) {
				return *reinterpret_cast<T*>(this->GetBuffer() + (0x20 + ((uintptr_t)index * 0x8)));
			}

			int GetSize() {
				return *reinterpret_cast<int*>((uintptr_t)this + 0x18);
			}
		};

		template<typename T = void*>
		class Array {
		public:
			int ArraySize() {
				return *reinterpret_cast<int*>((uintptr_t)this + 0x18);
			}

			T GetArray(int index) {
				return *reinterpret_cast<T*>((uintptr_t)this + 0x20 + (uintptr_t)index * 0x8);
			}
		};

		class String {
		public:
			char pad_0000[0x10];
			int len;
			wchar_t buffer[0];

			static String* New(const char* str)
			{
				return call<String*, const char*>(xorstr_("il2cpp_string_new"), str);
			}

		};

		class Type {
		public:
			CLASS("System", "Type");

			STATIC_METHOD(GetTypeInternal, "GetType", Type*, System::String*);

			static Type* GetType(const char* qualified_name) {
				if (!qualified_name) return nullptr;
				return GetTypeInternal(System::String::New(qualified_name));
			}

			static Type* SkinnedMeshRenderer() {
				return GetType(xorstr_("UnityEngine.SkinnedMeshRenderer, UnityEngine.CoreModule"));
			}

			static Type* Renderer() {
				return GetType(xorstr_("UnityEngine.Renderer, UnityEngine.CoreModule"));
			}

			static Type* Shader() {
				return GetType(xorstr_("UnityEngine.Shader, UnityEngine.CoreModule"));
			}

			static Type* Projectile() {
				return GetType(xorstr_("Projectile, Assembly-CSharp"));
			}

			static Type* ItemModProjectile() {
				return GetType(xorstr_("ItemModProjectile, Assembly-CSharp"));
			}
		};
	}
}