#include "IconManager.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <filesystem>

std::string IconManager::GetExePath() {
    char path[MAX_PATH]{};
    if (!GetModuleFileNameA(NULL, path, MAX_PATH)) return "";

    std::filesystem::path p(path);
    return p.parent_path().string();
}

void IconManager::Initialize(ID3D11Device* device) {
    if (m_initialized || !device) return;

    m_device = device;
    m_device->AddRef();
    m_initialized = true;
    m_gamePath = GetExePath();
}

void IconManager::Shutdown() {
    for (auto& [name, icon] : m_icons) {
        if (icon.srv) {
            icon.srv->Release();
            icon.srv = nullptr;
        }
    }

    m_icons.clear();
    m_failed.clear();

    if (m_device) {
        m_device->Release();
        m_device = nullptr;
    }

    m_initialized = false;
}

void IconManager::SetGamePath(const std::string& path) {
    m_gamePath = path;
    m_failed.clear();
}

bool IconManager::CreateTexture(const std::string& name, const unsigned char* rgba, int width, int height) {
    if (!m_device || !rgba || width <= 0 || height <= 0) return false;

    if (auto it = m_icons.find(name); it != m_icons.end()) {
        if (it->second.srv)
            it->second.srv->Release();
        m_icons.erase(it);
    }

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subResource{};
    subResource.pSysMem = rgba;
    subResource.SysMemPitch = width * 4;

    ID3D11Texture2D* pTexture = nullptr;
    if (FAILED(m_device->CreateTexture2D(&desc, &subResource, &pTexture)) || !pTexture)
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    ID3D11ShaderResourceView* pSRV = nullptr;
    HRESULT hr = m_device->CreateShaderResourceView(pTexture, &srvDesc, &pSRV);
    pTexture->Release();

    if (FAILED(hr) || !pSRV)
        return false;

    IconData icon;
    icon.srv = pSRV;
    icon.width = width;
    icon.height = height;
    m_icons[name] = icon;
    return true;
}

bool IconManager::LoadFromFileInternal(const std::string& name, const std::string& fullpath) {
    int w = 0, h = 0, channels = 0;
    unsigned char* data = stbi_load(fullpath.c_str(), &w, &h, &channels, 4);
    if (!data)
        return false;

    bool result = CreateTexture(name, data, w, h);
    stbi_image_free(data);
    return result;
}

bool IconManager::Load(const std::string& name) {
    if (!m_initialized || name.empty()) return false;

    if (m_icons.count(name)) return true;
    if (m_failed.count(name)) return false;

    std::string fullpath = m_gamePath + "\\Bundles\\items\\" + name + ".png";
    if (LoadFromFileInternal(name, fullpath)) return true;

    fullpath = m_gamePath + "\\Bundles\\items\\" + name + ".jpg";
    if (LoadFromFileInternal(name, fullpath)) return true;

    m_failed.insert(name);
    return false;
}

IconData* IconManager::Get(const std::string& name) {
    auto it = m_icons.find(name);
    if (it != m_icons.end()) return &it->second;

    if (Load(name)) {
        it = m_icons.find(name);
        if (it != m_icons.end()) return &it->second;
    }

    return nullptr;
}

ImTextureID IconManager::GetTextureID(const std::string& name) {
    IconData* icon = Get(name);
    return icon ? icon->GetTextureID() : (ImTextureID)0;
}

bool IconManager::Exists(const std::string& name) const {
    return m_icons.count(name) > 0;
}

void IconManager::Draw(const std::string& name, const ImVec2& pos, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, ImU32 col) {
    IconData* icon = Get(name);
    if (!icon || !icon->IsValid()) return;

    ImVec2 draw_size = size;
    if (draw_size.x <= 0.f) draw_size.x = (float)icon->width;
    if (draw_size.y <= 0.f) draw_size.y = (float)icon->height;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    if (!dl) return;

    dl->AddImage(icon->GetTextureID(), pos, ImVec2(pos.x + draw_size.x, pos.y + draw_size.y), uv0, uv1, col);
}