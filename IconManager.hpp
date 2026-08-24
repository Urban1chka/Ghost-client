#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <imgui.h>

struct IconData {
    ID3D11ShaderResourceView* srv = nullptr;

    int width = 0;
    int height = 0;

    ImTextureID GetTextureID() const { return (ImTextureID)(intptr_t)srv; }
    bool IsValid() const { return srv != nullptr; }
};

class IconManager {
public:
    static IconManager& Get() {
        static IconManager instance;
        return instance;
    }

    void Initialize(ID3D11Device* device);
    void Shutdown();
    void SetGamePath(const std::string& path);
    bool Load(const std::string& name);
    IconData* Get(const std::string& name);
    ImTextureID GetTextureID(const std::string& name);
    void Draw(const std::string& name, const ImVec2& pos, const ImVec2& size = ImVec2(0, 0), const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), ImU32 col = IM_COL32_WHITE);
    bool Exists(const std::string& name) const;

private:
    IconManager() = default;
    ~IconManager() { Shutdown(); }
    IconManager(const IconManager&) = delete;
    IconManager& operator=(const IconManager&) = delete;

    bool CreateTexture(const std::string& name, const unsigned char* rgba, int width, int height);
    bool LoadFromFileInternal(const std::string& name, const std::string& fullpath);
    std::string GetExePath();

    ID3D11Device* m_device = nullptr;
    std::string m_gamePath;
    std::unordered_map<std::string, IconData> m_icons;
    std::unordered_set<std::string> m_failed;
    bool m_initialized = false;
};