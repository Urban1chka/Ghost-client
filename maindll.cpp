#include "includes.hpp"
#include "ui_render.hpp"

#include <iostream>

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
HMODULE g_dll_module = nullptr;

void unload(HMODULE hMod) {
    if (window && oWndProc) {
        SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)oWndProc);
    }

    if (entity_data::local_player) {
        reset(true);
    
        auto instances = SDK::TOD_Sky::instances();
        if (instances) {
            for (int i = 0; i < instances->GetSize(); i++) {
                auto* sky = instances->GetArray(i);
                if (sky) {
                    auto* stars = sky->Stars();
                    if (stars) {
                        stars->Size() = 1.0f;
                        stars->Brightness() = 1.0f;
                    }
                }
            }
        }
    }

    hooks::shutdown();

    if (mainRenderTargetView) {
        mainRenderTargetView->Release();
        mainRenderTargetView = nullptr;
    }

    if (pContext) {
        pContext->Release();
        pContext = nullptr;
    }

    if (pDevice) {
        pDevice->Release();
        pDevice = nullptr;
    }

    if (ImGui::GetCurrentContext()) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}
DWORD WINAPI UnloadThread(LPVOID lpParam) {
    HMODULE hMod = static_cast<HMODULE>(lpParam);
    unload(hMod);
    FreeLibraryAndExitThread(hMod, 0);
    return 0;
}

DWORD WINAPI MainThread(LPVOID lpReserved) {
    HMODULE hMod = static_cast<HMODULE>(lpReserved);
    bool init_hook = false;

    ui::init_sound();
    hooks::init();

    do {
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
            kiero::bind(8, (void**)&oPresent, hooks::methods::hkPresent);

            init_hook = true;
        }
    } while (!init_hook);

    while (!setting::menu::exit) {
        Sleep(150);
    }

    Sleep(3000);
    CreateThread(nullptr, 0, UnloadThread, hMod, 0, nullptr);
    return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        g_dll_module = hMod;
        DisableThreadLibraryCalls(hMod);
        CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        if (!lpReserved) {
            unload(hMod);
        }

        break;
    }
    return TRUE;
}