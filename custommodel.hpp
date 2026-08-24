#pragma once
#define _HAS_STD_BYTE 0
#define NOMINMAX
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <commdlg.h>
#include <cstdarg>
#include <algorithm>
#include "SDK.hpp"
#include <d3d11.h>
namespace Gdiplus { using std::min; using std::max; }
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "ole32.lib")

extern ID3D11Device* pDevice;

namespace custommodel {
    inline void trace(const char*, ...) {}

    inline bool& enabled() { return setting::visuals::custommodel::enabled; }
    inline char* path() { return setting::visuals::custommodel::path; }
    inline uintptr_t& mesh_cache() { return setting::visuals::custommodel::mesh_cache; }
    inline bool& loaded() { return setting::visuals::custommodel::loaded; }
    inline const char*& status() { return setting::visuals::custommodel::status; }
    inline uintptr_t& orig_mesh() { return setting::visuals::custommodel::orig_mesh; }

    inline bool is_valid(uintptr_t p) { return p > 0x10000 && p < 0x7FFFFFFFFFFF; }
    template<typename T>
    inline T Read(uintptr_t addr) { return *reinterpret_cast<T*>(addr); }

    inline uintptr_t il2cpp_class_from_name(const char* ns, const char* name) {
        auto* klass = Dissector::FindClass(ns, name);
        if (!klass) klass = Dissector::FindClass("", name);
        return (uintptr_t)klass;
    }

    inline uintptr_t il2cpp_method_from_class(uintptr_t klass, const char* name, int argc = -1) {
        if (!klass) return 0;
        auto* m = Dissector::FindMethod((Dissector::IL2CPP::IL2CPPClass*)klass, name, argc);
        return m ? (uintptr_t)m->methodPtr : 0;
    }

    inline uintptr_t il2cpp_field_offset(uintptr_t klass, const char* name) {
        if (!klass) return 0;
        auto* f = Dissector::FindField((Dissector::IL2CPP::IL2CPPClass*)klass, name);
        return f ? (uintptr_t)Dissector::FindField((Dissector::IL2CPP::IL2CPPClass*)klass, name, false) : 0;
    }

    struct JVal {
        enum Type { NUL, BOOL, NUM, STR, ARR, OBJ } type = NUL;
        bool b = false;
        double n = 0;
        std::string s;
        std::vector<JVal> arr;
        std::vector<std::pair<std::string, JVal>> obj;

        const JVal* find(const char* key) const {
            if (type != OBJ) return nullptr;
            for (auto& kv : obj) if (kv.first == key) return &kv.second;
            return nullptr;
        }
        const JVal* at(size_t i) const { return type == ARR && i < arr.size() ? &arr[i] : nullptr; }
    };

    struct JParser {
        const char* p = nullptr;
        const char* e = nullptr;

        void skipws() { while (p < e && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++; }
        bool ch(char c) { skipws(); if (p < e && *p == c) { p++; return true; } return false; }

        std::string readstr() {
            std::string out; skipws();
            if (p >= e || *p != '"') return out;
            p++;
            while (p < e && *p != '"') {
                if (*p == '\\' && p + 1 < e) { p++; out.push_back(*p++); }
                else out.push_back(*p++);
            }
            if (p < e) p++;
            return out;
        }
        double readnum() {
            skipws(); const char* s = p;
            while (p < e && (strchr("+-0123456789.eE", *p))) p++;
            return strtod(std::string(s, p - s).c_str(), nullptr);
        }

        bool parseVal(JVal& v) {
            skipws();
            if (p >= e) return false;
            char c = *p;
            if (c == '"') { v.type = JVal::STR; v.s = readstr(); return true; }
            if (c == '{') {
                v.type = JVal::OBJ; p++;
                while (p < e) {
                    skipws(); if (p < e && *p == '}') break;
                    std::string k = readstr(); ch(':'); JVal vv; if (!parseVal(vv)) break;
                    v.obj.emplace_back(k, vv); if (!ch(',')) break;
                }
                ch('}'); return true;
            }
            if (c == '[') {
                v.type = JVal::ARR; p++;
                while (p < e) {
                    skipws(); if (p < e && *p == ']') break;
                    JVal vv; if (!parseVal(vv)) break;
                    v.arr.push_back(vv); if (!ch(',')) break;
                }
                ch(']'); return true;
            }
            if (c == 't') { v.type = JVal::BOOL; v.b = true; p += 4; return true; }
            if (c == 'f') { v.type = JVal::BOOL; v.b = false; p += 5; return true; }
            if (c == 'n') { v.type = JVal::NUL; p += 4; return true; }
            v.type = JVal::NUM; v.n = readnum(); return true;
        }
        bool parse(const std::string& txt, JVal& root) {
            p = txt.c_str(); e = p + txt.size();
            return parseVal(root);
        }
    };

    struct GlbMesh {
        std::vector<float> pos, norm, uv;
        std::vector<uint32_t> idx;
        std::vector<uint16_t> joints16;
        std::vector<uint32_t> joints32;
        bool joints32Mode = false;
        std::vector<float> weights;
        std::vector<float> invBind;
        bool hasSkin = false;
        std::string tex;
    };

    struct ObjMesh {
        std::vector<float> pos;
        std::vector<float> norm;
        std::vector<float> uv;
        std::vector<uint32_t> idx;
    };

    inline bool readAccessor(const JVal* acc, const JVal* bvs, const JVal*,
        const std::string& binData, int compType, int count, int components,
        std::vector<float>& outFloat, std::vector<uint32_t>& outU32, std::vector<uint16_t>& outU16)
    {
        if (!acc) return false;
        int bv = (int)acc->find("bufferView")->n;
        int off = acc->find("byteOffset") ? (int)acc->find("byteOffset")->n : 0;
        const JVal* bvJ = bvs ? bvs->at(bv) : nullptr;
        if (!bvJ) return false;
        int bOff = bvJ->find("byteOffset") ? (int)bvJ->find("byteOffset")->n : 0;
        int stride = bvJ->find("byteStride") ? (int)bvJ->find("byteStride")->n : 0;
        size_t base = (size_t)bOff + (size_t)off;

        if (base >= binData.size()) return false;

        auto byteSize = [&](int ct) -> int {
            switch (ct) {
            case 5120: return 1; case 5121: return 1; case 5122: return 2;
            case 5123: return 2; case 5125: return 4; case 5126: return 4; default: return 4;
            }
            };
        int es = byteSize(compType);
        int totalEl = count * components;

        if (compType == 5126) {
            outFloat.resize(totalEl);
            for (int i = 0; i < totalEl; i++) {
                size_t o = stride ? (base + (size_t)(i / components) * stride + (size_t)(i % components) * es) : (base + (size_t)i * es);
                if (o + 4 > binData.size()) { outFloat.clear(); return false; }
                memcpy(&outFloat[i], &binData[o], 4);
            }
            return true;
        }
        if (compType == 5123) {
            outU16.resize(totalEl);
            for (int i = 0; i < totalEl; i++) {
                size_t o = stride ? (base + (size_t)(i / components) * stride + (size_t)(i % components) * es) : (base + (size_t)i * es);
                if (o + 2 > binData.size()) { outU16.clear(); return false; }
                memcpy(&outU16[i], &binData[o], 2);
            }
            return true;
        }
        if (compType == 5125) {
            outU32.resize(totalEl);
            for (int i = 0; i < totalEl; i++) {
                size_t o = stride ? (base + (size_t)(i / components) * stride + (size_t)(i % components) * es) : (base + (size_t)i * es);
                if (o + 4 > binData.size()) { outU32.clear(); return false; }
                memcpy(&outU32[i], &binData[o], 4);
            }
            return true;
        }
        return false;
    }

    inline bool parseGltfJson(const JVal& root, const std::string& bin, GlbMesh& g) {
        const JVal* accs = root.find("accessors");
        const JVal* bvs = root.find("bufferViews");
        const JVal* meshes = root.find("meshes");
        if (!accs || !meshes || !bvs) return false;

        const JVal* prim = meshes->at(0)->find("primitives")->at(0);
        if (!prim) return false;
        const JVal* attr = prim->find("attributes");
        if (!attr) return false;

        auto loadFloat = [&](const char* key, std::vector<float>& dst, int comp) -> bool {
            const JVal* a = attr->find(key);
            if (!a) return false;
            int i = (int)a->n;
            const JVal* acc = accs->at(i);
            int ct = acc->find("componentType") ? (int)acc->find("componentType")->n : 5126;
            int cnt = (int)acc->find("count")->n;
            std::vector<float> f; std::vector<uint32_t> u32; std::vector<uint16_t> u16;
            if (!readAccessor(acc, bvs, nullptr, bin, ct, cnt, comp, f, u32, u16)) return false;
            dst = f; return true;
            };

        loadFloat("POSITION", g.pos, 3);
        loadFloat("NORMAL", g.norm, 3);
        loadFloat("TEXCOORD_0", g.uv, 2);

        if (const JVal* ia = prim->find("indices")) {
            int i = (int)ia->n;
            const JVal* acc = accs->at(i);
            int ct = acc->find("componentType") ? (int)acc->find("componentType")->n : 5123;
            int cnt = (int)acc->find("count")->n;
            std::vector<float> f; std::vector<uint32_t> u32; std::vector<uint16_t> u16;
            if (readAccessor(acc, bvs, nullptr, bin, ct, cnt, 1, f, u32, u16)) {
                if (ct == 5125) g.idx = u32;
                else if (ct == 5123) g.idx.assign(u16.begin(), u16.end());
            }
        }

        const JVal* skins = root.find("skins");
        if (skins && skins->arr.size() > 0) {
            const JVal* skin = skins->at(0);
            g.hasSkin = true;
            if (const JVal* ibm = skin->find("inverseBindMatrices")) {
                int i = (int)ibm->n;
                const JVal* acc = accs->at(i);
                int cnt = (int)acc->find("count")->n;
                std::vector<float> f; std::vector<uint32_t> u32; std::vector<uint16_t> u16;
                if (readAccessor(acc, bvs, nullptr, bin, 5126, cnt, 16, f, u32, u16))
                    g.invBind = f;
            }
        }
        if (attr->find("JOINTS_0") && attr->find("WEIGHTS_0")) {
            const JVal* ja = accs->at((int)attr->find("JOINTS_0")->n);
            int jct = ja->find("componentType") ? (int)ja->find("componentType")->n : 5123;
            int jcnt = (int)ja->find("count")->n;
            std::vector<float> f; std::vector<uint32_t> u32; std::vector<uint16_t> u16;
            if (readAccessor(ja, bvs, nullptr, bin, jct, jcnt, 4, f, u32, u16)) {
                if (jct == 5125) { g.joints32 = u32; g.joints32Mode = true; }
                else g.joints16 = u16;
            }
            loadFloat("WEIGHTS_0", g.weights, 4);
        }

        const JVal* mats = root.find("materials");
        const JVal* texs = root.find("textures");
        const JVal* imgs = root.find("images");
        if (mats && texs && imgs) {
            int texIndex = -1;
            if (prim->find("material")) {
                const JVal* m = mats->at((int)prim->find("material")->n);
                if (m) {
                    const JVal* pbr = m->find("pbrMetallicRoughness");
                    if (pbr) { const JVal* bct = pbr->find("baseColorTexture"); if (bct && bct->find("index")) texIndex = (int)bct->find("index")->n; }
                }
            }
            if (texIndex < 0 && texs->arr.size() > 0) texIndex = 0;
            const JVal* t = (texIndex >= 0) ? texs->at(texIndex) : nullptr;
            if (t) {
                int src = t->find("source") ? (int)t->find("source")->n : 0;
                const JVal* img = imgs->at(src);
                if (img && img->find("bufferView")) {
                    const JVal* bvJ = bvs->at((int)img->find("bufferView")->n);
                    if (bvJ) {
                        int bOff = bvJ->find("byteOffset") ? (int)bvJ->find("byteOffset")->n : 0;
                        int bLen = bvJ->find("byteLength") ? (int)bvJ->find("byteLength")->n : 0;
                        if (bOff >= 0 && bLen > 0 && (size_t)(bOff + bLen) <= bin.size())
                            g.tex.assign(bin.data() + bOff, (size_t)bLen);
                    }
                }
            }
        }
        return !g.pos.empty();
    }

    inline bool loadGltfFile(const wchar_t* wpath, GlbMesh& g, std::string& err) {
        std::ifstream f(wpath, std::ios::binary);
        if (!f) { err = "cannot open file"; return false; }
        std::string raw((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        f.close();

        if (raw.size() >= 4 && memcmp(raw.data(), "glTF", 4) == 0) {
            if (raw.size() < 20) { err = "truncated glb"; return false; }
            uint32_t jsonLen = *(uint32_t*)(raw.data() + 12);
            uint32_t jsonLenPadded = (jsonLen + 3) & ~3u;
            size_t jsonOff = 20;
            std::string jsonStr(raw.data() + jsonOff, jsonLen);
            size_t binChunkHdr = jsonOff + jsonLenPadded;
            size_t binOff = binChunkHdr + 8;
            std::string binData;
            if (binOff <= raw.size()) {
                uint32_t binLen = *(uint32_t*)(raw.data() + binChunkHdr);
                if (binOff + binLen > raw.size()) binLen = (uint32_t)(raw.size() - binOff);
                binData.assign(raw.data() + binOff, binLen);
            }
            JVal root; JParser jp;
            if (!jp.parse(jsonStr, root)) { err = "json parse failed"; return false; }
            return parseGltfJson(root, binData, g);
        }

        JVal root; JParser jp;
        if (!jp.parse(raw, root)) { err = "json parse failed"; return false; }

        std::string bin;
        if (const JVal* bufs = root.find("buffers")) {
            const JVal* b0 = bufs->at(0);
            if (b0) {
                std::string uri = b0->find("uri") ? b0->find("uri")->s : "";
                if (uri.rfind("data:base64,", 0) == 0) {
                    std::string b64 = uri.substr(13);
                    auto dec = [](const std::string& s) {
                        static int tbl[256]; static bool init = false;
                        if (!init) { for (int i = 0; i < 256; i++) tbl[i] = -1; for (int i = 0; i < 64; i++) tbl["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i; init = true; }
                        std::string out; int val = 0, bits = 0;
                        for (char c : s) { if (c == '=') break; int v = tbl[(unsigned char)c]; if (v < 0) continue; val = (val << 6) | v; bits += 6; if (bits >= 8) { bits -= 8; out.push_back((char)((val >> bits) & 0xFF)); } }
                        return out;
                        };
                    bin = dec(b64);
                }
                else {
                    std::wstring dir(wpath);
                    size_t slash = dir.find_last_of(L"\\/"); if (slash != std::wstring::npos) dir = dir.substr(0, slash + 1); else dir.clear();
                    std::wstring bpath = dir + std::wstring(uri.begin(), uri.end());
                    std::ifstream bf(bpath.c_str(), std::ios::binary);
                    if (bf) { bin.assign((std::istreambuf_iterator<char>(bf)), std::istreambuf_iterator<char>()); bf.close(); }
                }
            }
        }
        return parseGltfJson(root, bin, g);
    }

    inline bool loadObjFile(const wchar_t* wpath, ObjMesh& g, std::string& err) {
        std::ifstream f(wpath, std::ios::binary);
        if (!f) { err = "cannot open file"; return false; }

        struct Vec3 { float x, y, z; };
        struct Vec2 { float u, v; };
        struct Face { int vi[3], ti[3], ni[3]; };

        std::vector<Vec3> tempPos;
        std::vector<Vec3> tempNorm;
        std::vector<Vec2> tempUV;
        std::vector<Face> tempFaces;

        std::string line;
        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            if (line.substr(0, 2) == "v ") {
                Vec3 v;
                if (sscanf_s(line.c_str(), "v %f %f %f", &v.x, &v.y, &v.z) == 3)
                    tempPos.push_back(v);
            }
            else if (line.substr(0, 3) == "vn ") {
                Vec3 n;
                if (sscanf_s(line.c_str(), "vn %f %f %f", &n.x, &n.y, &n.z) == 3)
                    tempNorm.push_back(n);
            }
            else if (line.substr(0, 3) == "vt ") {
                Vec2 t;
                if (sscanf_s(line.c_str(), "vt %f %f", &t.u, &t.v) == 2)
                    tempUV.push_back(t);
            }
            else if (line.substr(0, 2) == "f ") {
                Face fc;
                memset(&fc, 0, sizeof(fc));
                if (sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
                    &fc.vi[0], &fc.ti[0], &fc.ni[0],
                    &fc.vi[1], &fc.ti[1], &fc.ni[1],
                    &fc.vi[2], &fc.ti[2], &fc.ni[2]) == 9) {
                    tempFaces.push_back(fc);
                }
                else if (sscanf_s(line.c_str(), "f %d//%d %d//%d %d//%d",
                    &fc.vi[0], &fc.ni[0], &fc.vi[1], &fc.ni[1], &fc.vi[2], &fc.ni[2]) == 6) {
                    tempFaces.push_back(fc);
                }
                else if (sscanf_s(line.c_str(), "f %d/%d %d/%d %d/%d",
                    &fc.vi[0], &fc.ti[0], &fc.vi[1], &fc.ti[1], &fc.vi[2], &fc.ti[2]) == 6) {
                    tempFaces.push_back(fc);
                }
                else if (sscanf_s(line.c_str(), "f %d %d %d",
                    &fc.vi[0], &fc.vi[1], &fc.vi[2]) == 3) {
                    tempFaces.push_back(fc);
                }
            }
        }
        f.close();

        if (tempFaces.empty()) { err = "no faces found"; return false; }

        for (auto& fc : tempFaces) {
            for (int j = 0; j < 3; j++) {
                int vi = fc.vi[j] - 1;
                if (vi >= 0 && vi < (int)tempPos.size()) {
                    g.pos.push_back(tempPos[vi].x);
                    g.pos.push_back(tempPos[vi].y);
                    g.pos.push_back(tempPos[vi].z);
                }
                int ni = fc.ni[j] - 1;
                if (ni >= 0 && ni < (int)tempNorm.size()) {
                    g.norm.push_back(tempNorm[ni].x);
                    g.norm.push_back(tempNorm[ni].y);
                    g.norm.push_back(tempNorm[ni].z);
                }
                int ti = fc.ti[j] - 1;
                if (ti >= 0 && ti < (int)tempUV.size()) {
                    g.uv.push_back(tempUV[ti].u);
                    g.uv.push_back(tempUV[ti].v);
                }
                g.idx.push_back((uint32_t)(g.idx.size()));
            }
        }
        return !g.pos.empty();
    }

    struct UVector2 { float x, y; };
    struct UVector3 { float x, y, z; };
    struct UBoneWeight { float w0, w1, w2, w3; int b0, b1, b2, b3; };
    struct UMatrix4x4 { float m[16]; };
    struct UColor { float r, g, b, a; };

    inline uintptr_t getMethod(const char* cls, const char* name, int argc = -1, const char* ns = "UnityEngine") {
        auto* klass = Dissector::FindClass(ns, cls);
        if (!klass) klass = Dissector::FindClass("", cls);
        if (!klass) return 0;
        auto* m = Dissector::FindMethod(klass, name, argc);
        return m ? (uintptr_t)m->methodPtr : 0;
    }

    inline uintptr_t getClass(const char* cls, const char* ns = "UnityEngine") {
        return (uintptr_t)Dissector::FindClass(ns, cls);
    }

    inline uintptr_t getClassAC(const char* cls) {
        return (uintptr_t)Dissector::FindClass("Assembly-CSharp", cls);
    }

    inline void ensureThreadAttached() {
        static bool done = false;
        if (done) return;
        auto dom = call<void*>(("il2cpp_domain_get"));
        if (dom) call<uintptr_t, void*>(("il2cpp_thread_attach"), dom);
        done = true;
    }

    inline uintptr_t obj_new(uintptr_t klass) {
        return (uintptr_t)Dissector::IL2CPP::CreateNewObject((Dissector::IL2CPP::IL2CPPClass*)klass);
    }

    inline uintptr_t arr_new(uintptr_t elementKlass, uintptr_t count) {
        return call<uintptr_t, uintptr_t, uintptr_t>(("il2cpp_array_new"), elementKlass, count);
    }

    template<typename T>
    inline void arr_set(uintptr_t arr, uintptr_t idx, const T& v) {
        if (!is_valid(arr)) return;
        *(T*)((uintptr_t)arr + 0x20 + idx * sizeof(T)) = v;
    }

    inline uintptr_t arr_len(uintptr_t arr) {
        if (!is_valid(arr)) return 0;
        return *(uintptr_t*)((uintptr_t)arr + 0x18);
    }

    inline uintptr_t str_new(const char* s) {
        return (uintptr_t)Dissector::IL2CPP::CreateNewString(s);
    }

    inline uintptr_t type_from_name(const char* full) {
        uintptr_t fn = getMethod("Type", "GetType", 1, "System");
        if (!fn) return 0;
        uintptr_t s = str_new(full);
        if (!s) return 0;
        return SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(fn, (uintptr_t)0, s);
    }

    inline uintptr_t fn_Instantiate() { static auto v = getMethod("Object", "Instantiate", 1); return v; }
    inline uintptr_t fn_SetArrayForChannelImpl() { static auto v = getMethod("Mesh", "SetArrayForChannelImpl", 8); return v; }
    inline uintptr_t fn_SetIndicesImpl() { static auto v = getMethod("Mesh", "SetIndicesImpl", 8); return v; }
    inline uintptr_t fn_set_subMeshCount() { static auto v = getMethod("Mesh", "set_subMeshCount", 1); return v; }
    inline uintptr_t fn_set_bindposes() { static auto v = getMethod("Mesh", "set_bindposes", 1); return v; }
    inline uintptr_t fn_SetBoneWeightsImpl() { static auto v = getMethod("Mesh", "SetBoneWeightsImpl", 1); return v; }
    inline uintptr_t fn_RecalculateBoundsImpl() { static auto v = getMethod("Mesh", "RecalculateBoundsImpl", 1); return v; }
    inline uintptr_t fn_RecalculateNormalsImpl() { static auto v = getMethod("Mesh", "RecalculateNormalsImpl", 1); return v; }
    inline uintptr_t fn_RecalculateTangentsImpl() { static auto v = getMethod("Mesh", "RecalculateTangentsImpl", 1); return v; }
    inline uintptr_t fn_UploadMeshDataImpl() { static auto v = getMethod("Mesh", "UploadMeshDataImpl", 1); return v; }
    inline uintptr_t fn_get_vertexCount() { static auto v = getMethod("Mesh", "get_vertexCount", 0); return v; }
    inline uintptr_t fn_Mesh_ctor() { static auto v = getMethod("Mesh", ".ctor", 0); return v; }
    inline uintptr_t fn_Internal_Create() { static auto v = getMethod("Mesh", "Internal_Create", 1); return v; }
    inline uintptr_t fn_Renderer_set_enabled() { static auto v = getMethod("Renderer", "set_enabled", 1); return v; }
    inline uintptr_t fn_Renderer_GetSharedMaterial() { static auto v = getMethod("Renderer", "get_sharedMaterial", 0); return v; }
    inline uintptr_t fn_Renderer_SetMaterial() { static auto v = getMethod("Renderer", "set_material", 1); return v; }
    inline uintptr_t fn_SMR_set_sharedMesh() { static auto v = getMethod("SkinnedMeshRenderer", "set_sharedMesh", 1); return v; }
    inline uintptr_t fn_SMR_get_sharedMesh() { static auto v = getMethod("SkinnedMeshRenderer", "get_sharedMesh", 0); return v; }
    inline uintptr_t fn_Shader_Find() { static auto v = getMethod("Shader", "Find", 1); return v; }
    inline uintptr_t fn_Shader_PropertyToID() { static auto v = getMethod("Shader", "PropertyToID", 1); return v; }
    inline uintptr_t fn_Material_ctor_Shader() { static auto v = getMethod("Material", ".ctor", 1); return v; }
    inline uintptr_t fn_Material_set_color() { static auto v = getMethod("Material", "set_color", 1); return v; }
    inline uintptr_t fn_Material_set_mainTexture() { static auto v = getMethod("Material", "set_mainTexture", 1); return v; }
    inline uintptr_t fn_GameObject_ctor() { static auto v = getMethod("GameObject", ".ctor", 0); return v; }
    inline uintptr_t fn_GameObject_AddComponent() { static auto v = getMethod("GameObject", "AddComponent", 1); return v; }
    inline uintptr_t fn_MeshFilter_set_sharedMesh() { static auto v = getMethod("MeshFilter", "set_sharedMesh", 1); return v; }
    inline uintptr_t fn_Transform_SetParent() { static auto v = getMethod("Transform", "SetParent", 2); return v; }
    inline uintptr_t fn_Transform_set_localPosition() { static auto v = getMethod("Transform", "set_localPosition", 1); return v; }
    inline uintptr_t fn_Transform_set_localScale() { static auto v = getMethod("Transform", "set_localScale", 1); return v; }
    inline uintptr_t fn_GameObject_get_transform() { static auto v = getMethod("GameObject", "get_transform", 0); return v; }
    inline uintptr_t fn_GameObject_set_layer() { static auto v = getMethod("GameObject", "set_layer", 1); return v; }
    inline uintptr_t fn_GameObject_SetActive() { static auto v = getMethod("GameObject", "SetActive", 1); return v; }
    inline uintptr_t fn_Object_Destroy() { static auto v = getMethod("Object", "Destroy", 1); return v; }
    inline uintptr_t fn_GetComponentsInChildren() { static auto v = getMethod("Component", "GetComponentsInChildren", 1); return v; }
    inline uintptr_t fn_Renderer_set_shadowCastingMode() { static auto v = getMethod("Renderer", "set_shadowCastingMode", 1); return v; }
    inline uintptr_t fn_SMM_SetVisible() { static auto v = getClassAC("SkinnedMultiMesh"); if (!v) return 0; auto* m = Dissector::FindMethod((Dissector::IL2CPP::IL2CPPClass*)v, "SetVisible", 1); return m ? (uintptr_t)m->methodPtr : 0; }
    inline uintptr_t fn_SMM_IsCurrentlyVisible() { static auto v = getClassAC("SkinnedMultiMesh"); if (!v) return 0; auto* m = Dissector::FindMethod((Dissector::IL2CPP::IL2CPPClass*)v, "IsCurrentlyVisible", 0); return m ? (uintptr_t)m->methodPtr : 0; }
    inline uintptr_t t_Renderer() {
        static uintptr_t v = 0;
        if (!is_valid(v))
            v = type_from_name("UnityEngine.Renderer, UnityEngine.CoreModule");
        return v;
    }

    inline uintptr_t k_Vector3() { static auto v = getClass("Vector3"); return v; }
    inline uintptr_t k_Vector2() { static auto v = getClass("Vector2"); return v; }
    inline uintptr_t k_Int32() { static auto v = getClass("Int32", "System"); return v; }
    inline uintptr_t k_BoneWeight() { static auto v = getClass("BoneWeight"); return v; }
    inline uintptr_t k_Matrix4x4() { static auto v = getClass("Matrix4x4"); return v; }
    inline uintptr_t k_Mesh() { static auto v = getClass("Mesh"); return v; }
    inline uintptr_t k_Material() { static auto v = getClass("Material"); return v; }
    inline uintptr_t k_Color() { static auto v = getClass("Color"); return v; }
    inline uintptr_t k_GameObject() { static auto v = getClass("GameObject"); return v; }

    inline uintptr_t findShader(const char* name) {
        if (!fn_Shader_Find()) return 0;
        uintptr_t s = str_new(name);
        if (!s) return 0;
        return SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(fn_Shader_Find(), (uintptr_t)0, s);
    }

    inline uintptr_t resolveCullOffShader() {
        static bool tried = false; static uintptr_t sh = 0;
        if (tried) return sh;
        tried = true;
        const char* names[] = { "UI/Default", "Sprites/Default", "Unlit/Texture", "Unlit/Color", "Standard" };
        for (const char* n : names) {
            sh = findShader(n);
            if (is_valid(sh)) break;
        }
        return sh;
    }

    inline std::string g_texPng;
    inline bool g_loadTexture = true;

    inline std::vector<uint8_t> g_texPix;
    inline int g_texW = 0, g_texH = 0;
    inline bool g_texDecoded = false;
    inline bool g_texValid = false;

    inline ULONG_PTR g_gdipToken = 0;
    inline bool ensureGdiplus() {
        if (g_gdipToken) return true;
        Gdiplus::GdiplusStartupInput in;
        return Gdiplus::GdiplusStartup(&g_gdipToken, &in, nullptr) == Gdiplus::Ok;
    }

    inline bool decodeImage(const std::string& data, int& outW, int& outH, std::vector<uint8_t>& rgba) {
        if (data.empty() || !ensureGdiplus()) return false;
        HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, data.size());
        if (!hg) return false;
        void* p = GlobalLock(hg);
        if (!p) { GlobalFree(hg); return false; }
        memcpy(p, data.data(), data.size());
        GlobalUnlock(hg);
        IStream* stream = nullptr;
        if (CreateStreamOnHGlobal(hg, TRUE, &stream) != S_OK || !stream) { GlobalFree(hg); return false; }

        bool ok = false;
        {
            Gdiplus::Bitmap bmp(stream);
            if (bmp.GetLastStatus() == Gdiplus::Ok) {
                UINT w = bmp.GetWidth(), h = bmp.GetHeight();
                if (w && h && w <= 8192 && h <= 8192) {
                    Gdiplus::Rect rc(0, 0, (INT)w, (INT)h);
                    Gdiplus::BitmapData bd;
                    if (bmp.LockBits(&rc, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bd) == Gdiplus::Ok) {
                        rgba.resize((size_t)w * h * 4);
                        for (UINT y = 0; y < h; y++) {
                            const uint8_t* src = (const uint8_t*)bd.Scan0 + (size_t)y * bd.Stride;
                            uint8_t* dst = rgba.data() + (size_t)y * w * 4;
                            for (UINT x = 0; x < w; x++) {
                                dst[x * 4 + 0] = src[x * 4 + 2];
                                dst[x * 4 + 1] = src[x * 4 + 1];
                                dst[x * 4 + 2] = src[x * 4 + 0];
                                dst[x * 4 + 3] = src[x * 4 + 3];
                            }
                        }
                        bmp.UnlockBits(&bd);
                        outW = (int)w; outH = (int)h; ok = true;
                    }
                }
            }
        }
        stream->Release();
        return ok && !rgba.empty();
    }

    inline uintptr_t g_modelMat = 0;
    inline bool g_matTried = false;
    inline uintptr_t g_origMaterial = 0;

    inline UColor hsv2rgb(float h, float s, float v, float a) {
        float c = v * s;
        float hp = h * 6.f;
        float x = c * (1.f - fabsf(fmodf(hp, 2.f) - 1.f));
        float r = 0, g = 0, b = 0;
        if (hp < 1) { r = c; g = x; }
        else if (hp < 2) { r = x; g = c; }
        else if (hp < 3) { g = c; b = x; }
        else if (hp < 4) { g = x; b = c; }
        else if (hp < 5) { r = x; b = c; }
        else { r = c; b = x; }
        float m = v - c;
        return { r + m, g + m, b + m, a };
    }

    inline void ensureModelMaterial(uintptr_t smr) {
        if (g_matTried) return;
        g_matTried = true;
        if (fn_Renderer_GetSharedMaterial())
            g_origMaterial = SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(fn_Renderer_GetSharedMaterial(), (uintptr_t)0, smr);

        uintptr_t sh = resolveCullOffShader();
        if (is_valid(sh) && fn_Material_ctor_Shader()) {
            uintptr_t mat = is_valid(k_Material()) ? obj_new(k_Material()) : 0;
            if (is_valid(mat)) {
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Material_ctor_Shader(), (void*)0, mat, sh);
                g_modelMat = mat;
                return;
            }
        }
        if (is_valid(g_origMaterial) && getMethod("Material", ".ctor", 1)) {
            uintptr_t mat = is_valid(k_Material()) ? obj_new(k_Material()) : 0;
            if (is_valid(mat)) {
                SDK::UnityEngine::SafeExecution::Execute<void*>(getMethod("Material", ".ctor", 1), (void*)0, mat, g_origMaterial);
                g_modelMat = mat;
            }
        }
    }

    inline void applyModelMaterial(uintptr_t smr) {
        ensureModelMaterial(smr);
        if (!is_valid(g_modelMat) || !fn_Renderer_SetMaterial() || !fn_Material_set_color()) return;

        if (setting::visuals::custommodel::rainbow::enabled) {
            float h = fmodf((float)SDK::UnityEngine::Time::GetTime() * 0.3f, 1.f);
            if (h < 0) h += 1.f;
            float a = setting::visuals::custommodel::rainbow::opacity;
            UColor col = hsv2rgb(h, 1.f, 1.f, a);
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Material_set_color(), (void*)0, g_modelMat, col);
        }
        else {
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Material_set_color(), (void*)0, g_modelMat, UColor{ 1.f, 1.f, 1.f, 1.f });
        }
        SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Renderer_SetMaterial(), (void*)0, smr, g_modelMat);
    }

    inline void resetModelMaterial() {
        g_modelMat = 0; g_matTried = false; g_origMaterial = 0;
    }

    inline uintptr_t g_modelGO = 0;
    inline uintptr_t g_modelMR = 0;

    inline void destroyModelObject() {
        if (is_valid(g_modelGO)) {
            uintptr_t got = fn_GameObject_get_transform()
                ? SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(fn_GameObject_get_transform(), (uintptr_t)0, g_modelGO) : 0;
            if (is_valid(got) && fn_Transform_set_localPosition())
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Transform_set_localPosition(), (void*)0, got, UVector3{ 0.f, 0.f, 0.f });

            if (fn_GameObject_SetActive())
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_GameObject_SetActive(), (void*)0, g_modelGO, (uintptr_t)0);
            if (fn_Object_Destroy())
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Object_Destroy(), (void*)0, g_modelGO);
        }
        g_modelGO = 0; g_modelMR = 0;
    }

    inline void sweepRenderersUnder(uintptr_t component, uintptr_t skipMR, bool on) {
        if (!is_valid(component) || !fn_Renderer_set_enabled()) return;
        if (!fn_GetComponentsInChildren()) return;
        uintptr_t ty = t_Renderer();
        if (!is_valid(ty)) return;
        uintptr_t arr = SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(fn_GetComponentsInChildren(), (uintptr_t)0, component, ty);
        if (!is_valid(arr)) return;
        uintptr_t n = arr_len(arr);
        if (n > 512) return;
        for (uintptr_t i = 0; i < n; i++) {
            uintptr_t r = Read<uintptr_t>(arr + 0x20 + i * 0x8);
            if (!is_valid(r) || r == skipMR) continue;
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Renderer_set_enabled(), (void*)0, r, (uintptr_t)(on ? 1 : 0));
            if (!on && fn_Renderer_set_shadowCastingMode())
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Renderer_set_shadowCastingMode(), (void*)0, r, (uintptr_t)0);
        }
    }

    inline void setHierarchyRenderers(SDK::BasePlayer* lp, uintptr_t skipMR, bool on) {
        if (!lp) return;
        sweepRenderersUnder((uintptr_t)lp, skipMR, on);
        auto pm = lp->playerModel();
        if (pm) sweepRenderersUnder((uintptr_t)pm, skipMR, on);
    }

    inline void hideHierarchyRenderers(SDK::BasePlayer* lp, uintptr_t skipMR) {
        setHierarchyRenderers(lp, skipMR, false);
    }

    inline uintptr_t g_lastPlayerModel = 0;
    inline uintptr_t g_lastLocalPlayer = 0;
    inline bool g_wasSleeping = false;

    inline uintptr_t ensureModelObject(uintptr_t mesh, SDK::UnityEngine::Transform* parent, int layer);

    inline uintptr_t ensureModelObject(uintptr_t mesh, SDK::UnityEngine::Transform* parent, int layer) {
        if (is_valid(g_modelMR)) return g_modelMR;
        if (!fn_GameObject_ctor() || !fn_GameObject_AddComponent() || !fn_MeshFilter_set_sharedMesh())
            return 0;

        uintptr_t go = is_valid(k_GameObject()) ? obj_new(k_GameObject()) : 0;
        if (!is_valid(go)) return 0;
        SDK::UnityEngine::SafeExecution::Execute<void*>(fn_GameObject_ctor(), (void*)0, go);
        if (fn_GameObject_SetActive())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_GameObject_SetActive(), (void*)0, go, (uintptr_t)1);
        if (layer >= 0 && fn_GameObject_set_layer())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_GameObject_set_layer(), (void*)0, go, (uintptr_t)layer);

        auto addComp = [&](const char* typeName) -> uintptr_t {
            uintptr_t t = type_from_name(typeName);
            if (!t) return 0;
            return SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(fn_GameObject_AddComponent(), (uintptr_t)0, go, t);
            };

        uintptr_t mf = addComp("UnityEngine.MeshFilter, UnityEngine.CoreModule");
        uintptr_t mr = addComp("UnityEngine.MeshRenderer, UnityEngine.CoreModule");
        if (!is_valid(mf) || !is_valid(mr)) return 0;

        SDK::UnityEngine::SafeExecution::Execute<void*>(fn_MeshFilter_set_sharedMesh(), (void*)0, mf, mesh);

        uintptr_t got = fn_GameObject_get_transform()
            ? SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(fn_GameObject_get_transform(), (uintptr_t)0, go) : 0;
        if (is_valid(got) && is_valid((uintptr_t)parent) && fn_Transform_SetParent()) {
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Transform_SetParent(), (void*)0, got, (uintptr_t)parent, (uintptr_t)0);
            if (fn_Transform_set_localPosition())
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Transform_set_localPosition(), (void*)0, got, UVector3{ 0.f, 0.f, 0.f });
            if (fn_Transform_set_localScale())
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Transform_set_localScale(), (void*)0, got, UVector3{ 1.f, 1.f, 1.f });
        }

        g_modelGO = go;
        g_modelMR = mr;
        return mr;
    }

    inline void apply() {
        if (!enabled() || !loaded() || !is_valid(mesh_cache())) return;

        SDK::BasePlayer* lp = entity_data::local_player;
        if (!lp) return;

        if ((uintptr_t)lp != g_lastLocalPlayer) {
            orig_mesh() = 0;
            g_lastPlayerModel = 0;
            g_lastLocalPlayer = (uintptr_t)lp;
            g_wasSleeping = lp->HasPlayerFlag(SDK::BasePlayer::PlayerFlags::Sleeping);
        }

        bool isSleeping = lp->HasPlayerFlag(SDK::BasePlayer::PlayerFlags::Sleeping);
        if (isSleeping != g_wasSleeping) {
            orig_mesh() = 0;
            g_lastPlayerModel = 0;
            g_wasSleeping = isSleeping;
        }

        SDK::UnityEngine::Transform* rootT = lp->GetTransform();

        auto pm = lp->playerModel();
        if (!pm) return;

        uintptr_t pmRaw = (uintptr_t)pm;
        if (pmRaw != g_lastPlayerModel) {
            orig_mesh() = 0;
            g_lastPlayerModel = pmRaw;
        }

        auto mm = pm->_multiMesh();
        if (!mm) return;

        auto renderers = mm->Renderers();
        if (!renderers) return;
        int size = renderers->GetSize();
        if (size <= 0 || size > 64) return;

        if (size > 0) {
            auto* firstSmr = renderers->GetArray(0);
            if (firstSmr) applyModelMaterial((uintptr_t)firstSmr);
        }

        uintptr_t mr = ensureModelObject(mesh_cache(), rootT, 0);
        if (is_valid(mr)) {
            if (is_valid(g_modelGO) && fn_GameObject_get_transform() && fn_Transform_set_localPosition()) {
                uintptr_t got = SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(fn_GameObject_get_transform(), (uintptr_t)0, g_modelGO);
                if (is_valid(got))
                    SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Transform_set_localPosition(), (void*)0, got, UVector3{ 0.f, 0.f, -0.15f });
            }
            if (is_valid(g_modelMat) && fn_Renderer_SetMaterial())
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Renderer_SetMaterial(), (void*)0, mr, g_modelMat);
            if (fn_Renderer_set_enabled())
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Renderer_set_enabled(), (void*)0, mr, (uintptr_t)1);
        }

        if (fn_SMM_SetVisible())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SMM_SetVisible(), (void*)0, (uintptr_t)mm, (uintptr_t)0);

        if (fn_Renderer_set_enabled()) {
            for (int i = 0; i < size; i++) {
                auto* smr = renderers->GetArray(i);
                if (!smr || (uintptr_t)smr == mr) continue;
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Renderer_set_enabled(), (void*)0, (uintptr_t)smr, (uintptr_t)0);
                if (fn_Renderer_set_shadowCastingMode())
                    SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Renderer_set_shadowCastingMode(), (void*)0, (uintptr_t)smr, (uintptr_t)0);
            }
        }

        hideHierarchyRenderers(lp, mr);
    }

    inline uintptr_t getLocalSharedMesh() {
        SDK::BasePlayer* lp = entity_data::local_player;
        if (!lp) return 0;
        auto pm = lp->playerModel();
        if (!pm) return 0;
        auto mm = pm->_multiMesh();
        if (!mm) return 0;
        auto renderers = mm->Renderers();
        if (!renderers) return 0;
        int size = renderers->GetSize();
        if (size <= 0 || size > 64) return 0;
        for (int i = 0; i < size; i++) {
            auto* smr = renderers->GetArray(i);
            if (!smr || !fn_SMR_get_sharedMesh()) continue;
            uintptr_t cur = SDK::UnityEngine::SafeExecution::Execute<uintptr_t>(fn_SMR_get_sharedMesh(), (uintptr_t)0, (uintptr_t)smr);
            if (is_valid(cur)) return cur;
        }
        return 0;
    }

    inline uintptr_t buildMeshGlb(GlbMesh& g, std::string& err) {
        if (g.pos.empty()) { err = "no positions"; return 0; }
        uintptr_t srcVerts = g.pos.size() / 3;
        if (srcVerts * 2 > 200000) { err = "too heavy (>100k verts)"; return 0; }

        g_texPng = g.tex;

        bool doubleGeo = !is_valid(resolveCullOffShader());
        std::vector<float> P = g.pos;
        std::vector<float> N = g.norm;
        std::vector<float> UV = g.uv;
        std::vector<uint32_t> IDX = g.idx;
        bool haveN = (g.norm.size() == g.pos.size());
        bool haveUV = (g.uv.size() / 2 == srcVerts);
        if (doubleGeo) {
            P.insert(P.end(), g.pos.begin(), g.pos.end());
            if (haveN) { N.reserve(g.norm.size() * 2); for (float v : g.norm) N.push_back(-v); }
            if (haveUV) UV.insert(UV.end(), g.uv.begin(), g.uv.end());
            for (size_t t = 0; t + 2 < g.idx.size(); t += 3) {
                uint32_t a = g.idx[t], b = g.idx[t + 1], c = g.idx[t + 2];
                IDX.push_back((uint32_t)(a + srcVerts));
                IDX.push_back((uint32_t)(c + srcVerts));
                IDX.push_back((uint32_t)(b + srcVerts));
            }
        }
        uintptr_t nVerts = P.size() / 3;

        if (!fn_SetArrayForChannelImpl()) { err = "Mesh vertex API not found"; return 0; }

        uintptr_t mk = k_Mesh();
        uintptr_t newMesh = is_valid(mk) ? obj_new(mk) : 0;
        if (is_valid(newMesh) && fn_Mesh_ctor())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Mesh_ctor(), (void*)0, newMesh);
        if (is_valid(newMesh) && fn_Internal_Create())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Internal_Create(), (void*)0, newMesh);
        if (!is_valid(newMesh)) { err = "mesh creation failed"; return 0; }

        float minY = 1e30f;
        for (size_t i = 0; i < P.size(); i += 3) if (P[i + 1] < minY) minY = P[i + 1];
        float yOff = -minY;

        uintptr_t aPos = arr_new(k_Vector3(), nVerts);
        if (is_valid(aPos)) {
            for (uintptr_t i = 0; i < nVerts; i++)
                arr_set<UVector3>(aPos, i, UVector3{ P[i * 3], P[i * 3 + 1] + yOff, P[i * 3 + 2] });
            if (fn_SetArrayForChannelImpl())
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SetArrayForChannelImpl(), (void*)0, newMesh,
                    (uintptr_t)0, (uintptr_t)0, (uintptr_t)3, (uintptr_t)aPos,
                    (uintptr_t)nVerts, (uintptr_t)0, (uintptr_t)nVerts, (uintptr_t)0);
        }

        if (N.size() == P.size()) {
            uintptr_t aN = arr_new(k_Vector3(), nVerts);
            if (is_valid(aN)) {
                for (uintptr_t i = 0; i < nVerts; i++)
                    arr_set<UVector3>(aN, i, UVector3{ N[i * 3], N[i * 3 + 1], N[i * 3 + 2] });
                if (fn_SetArrayForChannelImpl())
                    SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SetArrayForChannelImpl(), (void*)0, newMesh,
                        (uintptr_t)1, (uintptr_t)0, (uintptr_t)3, (uintptr_t)aN,
                        (uintptr_t)nVerts, (uintptr_t)0, (uintptr_t)nVerts, (uintptr_t)0);
            }
        }

        if (UV.size() / 2 == nVerts) {
            uintptr_t aU = arr_new(k_Vector2(), nVerts);
            if (is_valid(aU)) {
                for (uintptr_t i = 0; i < nVerts; i++)
                    arr_set<UVector2>(aU, i, UVector2{ UV[i * 2], UV[i * 2 + 1] });
                if (fn_SetArrayForChannelImpl())
                    SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SetArrayForChannelImpl(), (void*)0, newMesh,
                        (uintptr_t)4, (uintptr_t)0, (uintptr_t)2, (uintptr_t)aU,
                        (uintptr_t)nVerts, (uintptr_t)0, (uintptr_t)nVerts, (uintptr_t)0);
            }
        }

        if (g_loadTexture && !g_texDecoded && !g_texPng.empty()) {
            g_texDecoded = true;
            g_texValid = decodeImage(g_texPng, g_texW, g_texH, g_texPix);
            if (!g_texValid) { g_texPix.clear(); g_texW = g_texH = 0; }
        }
        const bool haveTex = g_loadTexture && g_texValid && haveUV && g_texW > 0 && g_texH > 0;
        uintptr_t kC = k_Color();
        uintptr_t aCol = is_valid(kC) ? arr_new(kC, nVerts) : 0;
        if (is_valid(aCol)) {
            for (uintptr_t i = 0; i < nVerts; i++) {
                UColor c{ 1.f, 1.f, 1.f, 1.f };
                if (haveTex && UV.size() >= (i + 1) * 2) {
                    float u = UV[i * 2], v = UV[i * 2 + 1];
                    u -= floorf(u); v -= floorf(v);
                    int px = (int)(u * (g_texW - 1) + 0.5f); px = max(0, min(px, g_texW - 1));
                    int py = (int)(v * (g_texH - 1) + 0.5f); py = max(0, min(py, g_texH - 1));
                    const uint8_t* t = &g_texPix[((size_t)py * g_texW + px) * 4];
                    c = UColor{ t[0] / 255.f, t[1] / 255.f, t[2] / 255.f, 1.f };
                }
                arr_set<UColor>(aCol, i, c);
            }
            if (fn_SetArrayForChannelImpl())
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SetArrayForChannelImpl(), (void*)0, newMesh,
                    (uintptr_t)3, (uintptr_t)0, (uintptr_t)4, (uintptr_t)aCol,
                    (uintptr_t)nVerts, (uintptr_t)0, (uintptr_t)nVerts, (uintptr_t)0);
        }

        if (!IDX.empty()) {
            uintptr_t aT = arr_new(k_Int32(), IDX.size());
            if (is_valid(aT)) {
                for (uintptr_t i = 0; i < IDX.size(); i++)
                    arr_set<uint32_t>(aT, i, IDX[i]);
                if (fn_set_subMeshCount())
                    SDK::UnityEngine::SafeExecution::Execute<void*>(fn_set_subMeshCount(), (void*)0, newMesh, (uintptr_t)1);
                if (fn_SetIndicesImpl())
                    SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SetIndicesImpl(), (void*)0, newMesh,
                        (uintptr_t)0, (uintptr_t)0, (uintptr_t)1, (uintptr_t)aT,
                        (uintptr_t)0, (uintptr_t)IDX.size(), (uintptr_t)0, (uintptr_t)0);
            }
        }

        {
            uintptr_t aW = arr_new(k_BoneWeight(), nVerts);
            if (is_valid(aW)) {
                UBoneWeight bw{ 1.f, 0.f, 0.f, 0.f, 0, 0, 0, 0 };
                for (uintptr_t i = 0; i < nVerts; i++) arr_set<UBoneWeight>(aW, i, bw);
                if (fn_SetBoneWeightsImpl())
                    SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SetBoneWeightsImpl(), (void*)0, newMesh, (uintptr_t)aW);
            }
            uintptr_t aB = arr_new(k_Matrix4x4(), 1);
            if (is_valid(aB)) {
                UMatrix4x4 id{}; id.m[0] = 1.f; id.m[5] = 1.f; id.m[10] = 1.f; id.m[15] = 1.f;
                arr_set<UMatrix4x4>(aB, 0, id);
                if (fn_set_bindposes())
                    SDK::UnityEngine::SafeExecution::Execute<void*>(fn_set_bindposes(), (void*)0, newMesh, (uintptr_t)aB);
            }
        }

        if (fn_RecalculateNormalsImpl())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_RecalculateNormalsImpl(), (void*)0, newMesh, (uintptr_t)0);
        if (fn_RecalculateTangentsImpl())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_RecalculateTangentsImpl(), (void*)0, newMesh, (uintptr_t)0);
        if (fn_RecalculateBoundsImpl())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_RecalculateBoundsImpl(), (void*)0, newMesh, (uintptr_t)0);
        if (fn_UploadMeshDataImpl())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_UploadMeshDataImpl(), (void*)0, newMesh, (uintptr_t)0);

        err.clear();
        return newMesh;
    }

    inline uintptr_t buildMeshObj(ObjMesh& g, std::string& err) {
        if (g.pos.empty()) { err = "no positions"; return 0; }
        uintptr_t nVerts = g.pos.size() / 3;
        if (nVerts > 200000) { err = "too heavy"; return 0; }

        g_texPng.clear();

        bool doubleGeo = !is_valid(resolveCullOffShader());
        std::vector<float> P = g.pos;
        std::vector<float> N = g.norm;
        std::vector<float> UV = g.uv;
        std::vector<uint32_t> IDX = g.idx;
        bool haveN = (g.norm.size() == g.pos.size());
        bool haveUV = (g.uv.size() / 2 == nVerts);
        if (doubleGeo) {
            P.insert(P.end(), g.pos.begin(), g.pos.end());
            if (haveN) { N.reserve(g.norm.size() * 2); for (float v : g.norm) N.push_back(-v); }
            if (haveUV) UV.insert(UV.end(), g.uv.begin(), g.uv.end());
            for (size_t t = 0; t + 2 < g.idx.size(); t += 3) {
                uint32_t a = g.idx[t], b = g.idx[t + 1], c = g.idx[t + 2];
                IDX.push_back(a + (uint32_t)nVerts);
                IDX.push_back(c + (uint32_t)nVerts);
                IDX.push_back(b + (uint32_t)nVerts);
            }
        }
        nVerts = P.size() / 3;

        if (!fn_SetArrayForChannelImpl()) { err = "Mesh API not found"; return 0; }

        uintptr_t mk = k_Mesh();
        uintptr_t newMesh = is_valid(mk) ? obj_new(mk) : 0;
        if (is_valid(newMesh) && fn_Mesh_ctor())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Mesh_ctor(), (void*)0, newMesh);
        if (is_valid(newMesh) && fn_Internal_Create())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Internal_Create(), (void*)0, newMesh);
        if (!is_valid(newMesh)) { err = "mesh creation failed"; return 0; }

        float minY = 1e30f;
        for (size_t i = 0; i < P.size(); i += 3) if (P[i + 1] < minY) minY = P[i + 1];
        float yOff = -minY;

        uintptr_t aPos = arr_new(k_Vector3(), nVerts);
        if (is_valid(aPos)) {
            for (uintptr_t i = 0; i < nVerts; i++)
                arr_set<UVector3>(aPos, i, UVector3{ P[i * 3], P[i * 3 + 1] + yOff, P[i * 3 + 2] });
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SetArrayForChannelImpl(), (void*)0, newMesh,
                (uintptr_t)0, (uintptr_t)0, (uintptr_t)3, (uintptr_t)aPos,
                (uintptr_t)nVerts, (uintptr_t)0, (uintptr_t)nVerts, (uintptr_t)0);
        }

        if (N.size() == P.size()) {
            uintptr_t aN = arr_new(k_Vector3(), nVerts);
            if (is_valid(aN)) {
                for (uintptr_t i = 0; i < nVerts; i++)
                    arr_set<UVector3>(aN, i, UVector3{ N[i * 3], N[i * 3 + 1], N[i * 3 + 2] });
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SetArrayForChannelImpl(), (void*)0, newMesh,
                    (uintptr_t)1, (uintptr_t)0, (uintptr_t)3, (uintptr_t)aN,
                    (uintptr_t)nVerts, (uintptr_t)0, (uintptr_t)nVerts, (uintptr_t)0);
            }
        }

        if (UV.size() / 2 == nVerts) {
            uintptr_t aU = arr_new(k_Vector2(), nVerts);
            if (is_valid(aU)) {
                for (uintptr_t i = 0; i < nVerts; i++)
                    arr_set<UVector2>(aU, i, UVector2{ UV[i * 2], UV[i * 2 + 1] });
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SetArrayForChannelImpl(), (void*)0, newMesh,
                    (uintptr_t)4, (uintptr_t)0, (uintptr_t)2, (uintptr_t)aU,
                    (uintptr_t)nVerts, (uintptr_t)0, (uintptr_t)nVerts, (uintptr_t)0);
            }
        }

        {
            uintptr_t kC = k_Color();
            uintptr_t aCol = is_valid(kC) ? arr_new(kC, nVerts) : 0;
            if (is_valid(aCol)) {
                for (uintptr_t i = 0; i < nVerts; i++)
                    arr_set<UColor>(aCol, i, UColor{ 1.f, 1.f, 1.f, 1.f });
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SetArrayForChannelImpl(), (void*)0, newMesh,
                    (uintptr_t)3, (uintptr_t)0, (uintptr_t)4, (uintptr_t)aCol,
                    (uintptr_t)nVerts, (uintptr_t)0, (uintptr_t)nVerts, (uintptr_t)0);
            }
        }

        if (!IDX.empty()) {
            uintptr_t aT = arr_new(k_Int32(), IDX.size());
            if (is_valid(aT)) {
                for (uintptr_t i = 0; i < IDX.size(); i++)
                    arr_set<uint32_t>(aT, i, IDX[i]);
                if (fn_set_subMeshCount())
                    SDK::UnityEngine::SafeExecution::Execute<void*>(fn_set_subMeshCount(), (void*)0, newMesh, (uintptr_t)1);
                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SetIndicesImpl(), (void*)0, newMesh,
                    (uintptr_t)0, (uintptr_t)0, (uintptr_t)1, (uintptr_t)aT,
                    (uintptr_t)0, (uintptr_t)IDX.size(), (uintptr_t)0, (uintptr_t)0);
            }
        }

        {
            uintptr_t aW = arr_new(k_BoneWeight(), nVerts);
            if (is_valid(aW)) {
                UBoneWeight bw{ 1.f, 0.f, 0.f, 0.f, 0, 0, 0, 0 };
                for (uintptr_t i = 0; i < nVerts; i++) arr_set<UBoneWeight>(aW, i, bw);
                if (fn_SetBoneWeightsImpl())
                    SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SetBoneWeightsImpl(), (void*)0, newMesh, (uintptr_t)aW);
            }
            uintptr_t aB = arr_new(k_Matrix4x4(), 1);
            if (is_valid(aB)) {
                UMatrix4x4 id{}; id.m[0] = 1.f; id.m[5] = 1.f; id.m[10] = 1.f; id.m[15] = 1.f;
                arr_set<UMatrix4x4>(aB, 0, id);
                if (fn_set_bindposes())
                    SDK::UnityEngine::SafeExecution::Execute<void*>(fn_set_bindposes(), (void*)0, newMesh, (uintptr_t)aB);
            }
        }

        if (fn_RecalculateNormalsImpl())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_RecalculateNormalsImpl(), (void*)0, newMesh, (uintptr_t)0);
        if (fn_RecalculateTangentsImpl())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_RecalculateTangentsImpl(), (void*)0, newMesh, (uintptr_t)0);
        if (fn_RecalculateBoundsImpl())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_RecalculateBoundsImpl(), (void*)0, newMesh, (uintptr_t)0);
        if (fn_UploadMeshDataImpl())
            SDK::UnityEngine::SafeExecution::Execute<void*>(fn_UploadMeshDataImpl(), (void*)0, newMesh, (uintptr_t)0);

        err.clear();
        return newMesh;
    }

    inline void revert() {
        SDK::BasePlayer* lp = entity_data::local_player;
        if (lp) {
            auto pm = lp->playerModel();
            if (pm) {
                auto mm = pm->_multiMesh();
                if (mm) {
                    if (fn_SMM_SetVisible())
                        SDK::UnityEngine::SafeExecution::Execute<void*>(fn_SMM_SetVisible(), (void*)0, (uintptr_t)mm, (uintptr_t)1);
                }
            }
        }
        if (lp) {
            auto pm = lp->playerModel();
            if (pm) {
                auto mm = pm->_multiMesh();
                if (mm) {
                    auto renderers = mm->Renderers();
                    if (renderers) {
                        int size = renderers->GetSize();
                        if (size > 0 && size <= 64 && fn_Renderer_set_enabled()) {
                            for (int i = 0; i < size; i++) {
                                auto* smr = renderers->GetArray(i);
                                if (!smr) continue;
                                SDK::UnityEngine::SafeExecution::Execute<void*>(fn_Renderer_set_enabled(), (void*)0, (uintptr_t)smr, (uintptr_t)1);
                            }
                        }
                    }
                }
            }
        }
        setHierarchyRenderers(lp, g_modelMR, true);
        orig_mesh() = 0;
        resetModelMaterial();
        destroyModelObject();
    }

    inline char g_statusBuf[128] = "no model";
    inline char g_blendTempPath[MAX_PATH] = {};
    inline GlbMesh g_glbData;
    inline ObjMesh g_objData;
    inline bool g_parsed = false;
    inline bool g_isObj = false;
    inline bool g_blendMode = false;
    inline bool g_buildTried = false;
    inline int g_buildAttempts = 0;
    inline uintptr_t g_lastPlayer = 0;

    inline std::wstring findBlender() {
        const wchar_t* paths[] = {
            L"C:\\Program Files\\Blender Foundation\\Blender 4.2\\blender.exe",
            L"C:\\Program Files\\Blender Foundation\\Blender 4.1\\blender.exe",
            L"C:\\Program Files\\Blender Foundation\\Blender 4.0\\blender.exe",
            L"C:\\Program Files\\Blender Foundation\\Blender 3.6\\blender.exe",
            L"C:\\Program Files\\Blender Foundation\\Blender 3.5\\blender.exe",
            L"C:\\Program Files\\Blender Foundation\\Blender 3.4\\blender.exe",
            L"C:\\Program Files\\Blender Foundation\\Blender 3.3\\blender.exe",
            L"C:\\Program Files\\Blender Foundation\\Blender 3.2\\blender.exe",
            L"C:\\Program Files\\Blender Foundation\\Blender 3.1\\blender.exe",
            L"C:\\Program Files\\Blender Foundation\\Blender 3.0\\blender.exe",
        };
        for (auto& p : paths) {
            if (GetFileAttributesW(p) != INVALID_FILE_ATTRIBUTES)
                return p;
        }
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Blender Foundation\\Blender", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            wchar_t ver[32] = {};
            DWORD verLen = sizeof(ver);
            DWORD idx = 0;
            while (RegEnumKeyExW(hKey, idx, ver, &verLen, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
                wchar_t subKey[256] = {};
                swprintf_s(subKey, L"SOFTWARE\\Blender Foundation\\Blender\\%s", ver);
                HKEY hSub;
                if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hSub) == ERROR_SUCCESS) {
                    wchar_t exePath[MAX_PATH] = {};
                    DWORD exeLen = sizeof(exePath);
                    if (RegQueryValueExW(hSub, L"executablePath", nullptr, nullptr, (LPBYTE)exePath, &exeLen) == ERROR_SUCCESS) {
                        RegCloseKey(hSub);
                        RegCloseKey(hKey);
                        if (GetFileAttributesW(exePath) != INVALID_FILE_ATTRIBUTES)
                            return exePath;
                    }
                    RegCloseKey(hSub);
                }
                verLen = sizeof(ver);
                idx++;
            }
            RegCloseKey(hKey);
        }
        return L"";
    }

    inline bool convertBlendToGlb(const wchar_t* blendPath, wchar_t* outGlb, size_t outSize) {
        std::wstring blender = findBlender();
        if (blender.empty()) return false;

        wchar_t tempDir[MAX_PATH] = {};
        GetTempPathW(MAX_PATH, tempDir);
        wchar_t tempFile[MAX_PATH] = {};
        GetTempFileNameW(tempDir, L"glb", 0, tempFile);
        wcscpy_s(outGlb, outSize, tempFile);

        wchar_t cmd[2048] = {};
        swprintf_s(cmd, L"\"%s\" --background --python-expr \"import bpy; bpy.ops.wm.gltf_export(filepath=r'%s', export_format='GLB')\" -- %s",
            blender.c_str(), tempFile, blendPath);

        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi = {};
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        if (!CreateProcessW(nullptr, cmd, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
            return false;

        WaitForSingleObject(pi.hProcess, 60000);
        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return exitCode == 0 && GetFileAttributesW(outGlb) != INVALID_FILE_ATTRIBUTES;
    }

    inline void browse() {
        wchar_t wpath[MAX_PATH] = { 0 };
        OPENFILENAMEW ofn = { sizeof(ofn) };
        ofn.lpstrFilter = L"3D Models\0*.glb;*.gltf;*.obj;*.blend\0All\0*.*\0";
        ofn.lpstrFile = wpath;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetOpenFileNameW(&ofn)) {
            WideCharToMultiByte(CP_UTF8, 0, wpath, -1, path(), MAX_PATH, NULL, NULL);
            status() = "path set � press Load";
        }
    }

    inline void load() {
        if (!path()[0]) { status() = "no path � Browse first"; return; }
        std::string err;
        wchar_t wpath[MAX_PATH] = { 0 };
        MultiByteToWideChar(CP_UTF8, 0, path(), -1, wpath, MAX_PATH);

        std::string pathStr(path());
        bool isObj = (pathStr.size() >= 4 && pathStr.substr(pathStr.size() - 4) == ".obj");
        bool isBlend = (pathStr.size() >= 6 && pathStr.substr(pathStr.size() - 6) == ".blend");

        if (isBlend) {
            snprintf(g_statusBuf, sizeof(g_statusBuf), "converting .blend...");
            status() = g_statusBuf;

            wchar_t glbPath[MAX_PATH] = {};
            if (!convertBlendToGlb(wpath, glbPath, MAX_PATH)) {
                snprintf(g_statusBuf, sizeof(g_statusBuf), "error: Blender not found or convert failed");
                status() = g_statusBuf;
                return;
            }
            wcscpy_s(wpath, glbPath);
            isObj = false;
        }

        if (isObj) {
            ObjMesh g;
            if (!loadObjFile(wpath, g, err)) {
                snprintf(g_statusBuf, sizeof(g_statusBuf), "error: %s", err.c_str());
                status() = g_statusBuf;
                return;
            }
            if (is_valid(mesh_cache())) { revert(); mesh_cache() = 0; }
            destroyModelObject();
            g_objData = g;
            g_isObj = true;
        }
        else {
            GlbMesh g;
            if (!loadGltfFile(wpath, g, err)) {
                snprintf(g_statusBuf, sizeof(g_statusBuf), "error: %s", err.c_str());
                status() = g_statusBuf;
                return;
            }
            if (is_valid(mesh_cache())) { revert(); mesh_cache() = 0; }
            destroyModelObject();
            g_glbData = g;
            g_isObj = false;
        }

        g_parsed = true;
        g_buildTried = false;
        g_buildAttempts = 0;
        g_texDecoded = false; g_texValid = false; g_texPix.clear(); g_texW = g_texH = 0;
        loaded() = false;

        snprintf(g_statusBuf, sizeof(g_statusBuf), "parsed � enable in game");
        status() = g_statusBuf;
    }

    inline void on_disable() { if (is_valid(mesh_cache())) revert(); }

    inline void tick() {
        if (enabled()) {
            if (!g_parsed) return;
            ensureThreadAttached();
            SDK::BasePlayer* lp = entity_data::local_player;
            if ((uintptr_t)lp != g_lastPlayer) {
                g_lastPlayer = (uintptr_t)lp;
                if (is_valid(mesh_cache())) { revert(); mesh_cache() = 0; }
                resetModelMaterial();
                destroyModelObject();
                g_buildTried = false;
                g_buildAttempts = 0;
            }
            if (!lp) return;

            auto pm = lp->playerModel();
            if (!pm) return;
            auto mm = pm->_multiMesh();
            if (!mm) return;

            if (!is_valid(mesh_cache()) && !g_buildTried) {
                g_buildTried = true;
                std::string err;
                uintptr_t m = 0;
                if (g_isObj)
                    m = buildMeshObj(g_objData, err);
                else
                    m = buildMeshGlb(g_glbData, err);

                if (is_valid(m)) {
                    mesh_cache() = m;
                    loaded() = true;
                    int vc = fn_get_vertexCount() ? SDK::UnityEngine::SafeExecution::Execute<int>(fn_get_vertexCount(), 0, m) : -1;
                    snprintf(g_statusBuf, sizeof(g_statusBuf), "built: %d verts", vc);
                    status() = g_statusBuf;
                }
                else {
                    if (++g_buildAttempts < 3) g_buildTried = false;
                    snprintf(g_statusBuf, sizeof(g_statusBuf), "build failed (%d/3): %s", g_buildAttempts, err.c_str());
                    status() = g_statusBuf;
                    return;
                }
            }

            static uint32_t s_validate = 0;
            if (is_valid(mesh_cache()) && (++s_validate % 30) == 0 && fn_get_vertexCount()) {
                int vc = SDK::UnityEngine::SafeExecution::Execute<int>(fn_get_vertexCount(), 0, mesh_cache());
                if (vc <= 0 && g_buildAttempts < 3) {
                    revert(); mesh_cache() = 0; g_buildTried = false;
                }
            }

            apply();
        }
        else {
            if (is_valid(mesh_cache())) { revert(); mesh_cache() = 0; }
            g_lastPlayer = 0;
        }
    }

}
