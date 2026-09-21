#pragma once
// Stubs minimos para checar sintaxe e logica FORA do jogo (nao e o CommonLibF4 de verdade).
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <format>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace REX {
    template <class... A> void INFO(std::format_string<A...> f, A&&... a) { std::printf("[I] %s\n", std::format(f, std::forward<A>(a)...).c_str()); }
    template <class... A> void WARN(std::format_string<A...> f, A&&... a) { std::printf("[W] %s\n", std::format(f, std::forward<A>(a)...).c_str()); }
    template <class... A> void ERROR(std::format_string<A...> f, A&&... a) { std::printf("[E] %s\n", std::format(f, std::forward<A>(a)...).c_str()); }
}
namespace RE {
    struct NiPoint3 { float x{}, y{}, z{}; constexpr NiPoint3() = default; constexpr NiPoint3(float a, float b, float c) : x(a), y(b), z(c) {} };
    struct NiPoint3A : NiPoint3 {};
    struct hkVector4f { float x{}, y{}, z{}, w{}; };
    enum class COL_LAYER : std::int32_t { kCharController = 30 };
    template <class T> struct hkPadSpu { T storage{}; void operator=(T v) { storage = v; } };
    struct FilterData { hkPadSpu<std::uint32_t> m_collisionFilterInfo; };
    struct CastQuery { FilterData m_filterData; };
    struct CastResult { hkVector4f normal; };

    struct Box { float x0, y0, z0, x1, y1, z1; };
    inline std::vector<Box> g_world;
    inline int g_rayCount = 0;

    class bhkPickData {
    public:
        CastQuery castQuery; CastResult result; NiPoint3 s, e; bool hit = false; float frac = 1.0F;
        void SetStartEnd(const NiPoint3& a, const NiPoint3& b) { s = a; e = b; hit = false; frac = 1.0F; }
        bool HasHit() const { return hit; }
        float GetHitFraction() const { return frac; }
    };
    class TESObjectCELL {
    public:
        unsigned GetFormID() const { return 0xDEC8; }
        void* Pick(bhkPickData& pd) {
            ++g_rayCount;
            const float d[3] = { pd.e.x - pd.s.x, pd.e.y - pd.s.y, pd.e.z - pd.s.z };
            const float o[3] = { pd.s.x, pd.s.y, pd.s.z };
            float best = 2.0F; float bn[3] = { 0, 0, 0 };
            for (const auto& b : g_world) {
                const float lo[3] = { b.x0, b.y0, b.z0 }, hi[3] = { b.x1, b.y1, b.z1 };
                bool inside = true; for (int i = 0; i < 3; ++i) if (o[i] < lo[i] || o[i] > hi[i]) inside = false;
                if (inside) continue;  // raio que comeca dentro do solido nao acerta
                float tmin = 0.0F, tmax = 1.0F; int axis = -1; float sign = 0; bool ok = true;
                for (int i = 0; i < 3 && ok; ++i) {
                    if (std::fabs(d[i]) < 1e-9F) { if (o[i] < lo[i] || o[i] > hi[i]) ok = false; continue; }
                    float t1 = (lo[i] - o[i]) / d[i], t2 = (hi[i] - o[i]) / d[i]; float sg = -1.0F;
                    if (t1 > t2) { std::swap(t1, t2); sg = 1.0F; }
                    if (t1 > tmin) { tmin = t1; axis = i; sign = sg; }
                    tmax = std::min(tmax, t2); if (tmin > tmax) ok = false;
                }
                if (ok && axis >= 0 && tmin < best) { best = tmin; bn[0] = bn[1] = bn[2] = 0; bn[axis] = sign; }
            }
            if (best <= 1.0F) { pd.hit = true; pd.frac = best; pd.result.normal = { bn[0], bn[1], bn[2], 0 }; }
            return nullptr;
        }
    };
    struct LOADED_REF_DATA {};
    struct PlayerData { NiPoint3A angle; };
    class PlayerCharacter {
    public:
        static PlayerCharacter* GetSingleton() { static PlayerCharacter p; return &p; }
        NiPoint3 GetPosition() const { return pos; }
        TESObjectCELL* GetParentCell() const { return parentCell; }
        void SetPosition(const NiPoint3& p, bool) { pos = p; }
        NiPoint3 pos; PlayerData data; TESObjectCELL* parentCell = nullptr; LOADED_REF_DATA* loadedData = nullptr;
    };
    namespace SendHUDMessage { inline void ShowHUDMessage(const char* m, const char*, bool, bool) { std::printf("[HUD] %s\n", m); } }
    class MainMenu {}; class LoadingMenu {}; class PauseMenu {}; class PipboyMenu {}; class Console {};
    class UI { public: static UI* GetSingleton() { static UI u; return &u; } template <class T> bool GetMenuOpen() const { return false; } };
}
