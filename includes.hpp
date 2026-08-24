#pragma once
#include <Windows.h>
#include <iostream>
#include <d3d11.h>

#include "SDK.hpp"
#include "Math.hpp"
#include "Hooks.hpp"
#include "Getter.hpp"
#include "setting.hpp"
#include "Dissector.hpp"

#include "lib/kiero/kiero.h"
#include "lib/kiero/minhook/include/MinHook.h"

#include "lib/imgui/imgui.h"
#include "lib/imgui/imgui_impl_win32.h"
#include "lib/imgui/imgui_impl_dx11.h"
#include "lib/imgui/imgui_internal.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

typedef HRESULT(__stdcall* Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;

extern Present oPresent;
extern HWND window;
extern WNDPROC oWndProc;
extern ID3D11Device* pDevice;
extern ID3D11DeviceContext* pContext;
extern ID3D11RenderTargetView* mainRenderTargetView;