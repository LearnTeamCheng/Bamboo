// Random.h
#pragma once

#include <random>
#include <cstdint>
#include <vector>
#include <string>
#include <initializer_list>
#include <algorithm>
#include <optional>
#include <cmath>
#include <cassert>

namespace Bamboo {

    using RNG = std::mt19937_64;

    // 轻量向量
    template<typename T>
    struct Vec2 { T x, y; Vec2(T xx = T(0), T yy = T(0)) : x(xx), y(yy) {} };

    template<typename T>
    struct Vec3 { T x, y, z; Vec3(T xx = T(0), T yy = T(0), T zz = T(0)) : x(xx), y(yy), z(zz) {} };

    // ==================== 前向声明 detail ====================
    namespace detail {
        struct GlobalRandomAccessor;
    }

    // ==================== Random 类 ====================
    class Random
    {
        // 友元：允许 detail::GlobalRandomAccessor 访问 private
        friend struct detail::GlobalRandomAccessor;

    public:
        // ==================== 静态全局接口（避免与成员冲突）================
        static int      GlobalInt(int min, int max) noexcept;
        static float    GlobalFloat(float min, float max) noexcept;
        static float    GlobalFloat01() noexcept;
        static bool     GlobalBool(float p = 0.5f) noexcept;
        static void     SetGlobalSeed(uint64_t seed) noexcept;
        static uint64_t GetGlobalSeed() noexcept;

        // ==================== 实例接口 ====================
        explicit Random(uint64_t seed = 0) noexcept { Seed(seed); }
        void Seed(uint64_t seed) noexcept;

        // 成员函数（不与 static 冲突）
        int      Int(int min, int max) noexcept;
        float    Float(float min, float max) noexcept;
        double   Double(double min, double max) noexcept;
        float    Float01() noexcept { return Float(0.0f, 1.0f); }
        bool     Bool(float p = 0.5f) noexcept;

        template<typename T> T Range(T min, T max) noexcept;
        template<typename T> T Choice(const std::vector<T>& items);
        template<typename T> T Choice(std::initializer_list<T> items);

        template<typename T>
        std::optional<T> WeightedChoice(const std::vector<std::pair<T, float>>& items) noexcept;

        Vec2<float> InUnitCircle() noexcept;
        Vec3<float> InUnitSphere() noexcept;
        Vec2<float> OnUnitCircle() noexcept;
        Vec3<float> OnUnitSphere() noexcept;

        void FillFloat01(float* out, size_t count) noexcept;
        void FillInt(int* out, size_t count, int min, int max) noexcept;

        template<typename It> void Shuffle(It first, It last) noexcept;

    private:
        RNG gen_;  // 私有，detail 可访问
    };

    // ==================== detail 实现 ====================
    namespace detail {

        // 线程局部 RNG（预生成）
        inline thread_local RNG g_threadLocalGen([] {
            std::random_device rd;
            uint64_t seed = ((uint64_t)rd() << 32) | rd();
            RNG gen;
            gen.seed(seed);
            gen.discard(100);
            return gen;
            }());

        // 线程局部 Random 实例
        inline thread_local Random g_threadLocalRandom(0);

        // 访问器：延迟初始化 gen_
        struct GlobalRandomAccessor {
            static void Init() noexcept {
                static thread_local bool initialized = false;
                if (!initialized) {
                    g_threadLocalRandom.gen_ = g_threadLocalGen;  // 友元可访问！
                    initialized = true;
                }
            }

            static Random& Get() noexcept {
                Init();
                return g_threadLocalRandom;
            }
        };

    } // namespace detail

    // ==================== 静态函数实现（使用 Global 前缀）===================
    inline int Random::GlobalInt(int min, int max) noexcept {
        return detail::GlobalRandomAccessor::Get().Int(min, max);
    }

    inline float Random::GlobalFloat(float min, float max) noexcept {
        return detail::GlobalRandomAccessor::Get().Float(min, max);
    }

    inline float Random::GlobalFloat01() noexcept {
        return detail::GlobalRandomAccessor::Get().Float01();
    }

    inline bool Random::GlobalBool(float p) noexcept {
        return detail::GlobalRandomAccessor::Get().Bool(p);
    }

    inline void Random::SetGlobalSeed(uint64_t seed) noexcept {
        detail::GlobalRandomAccessor::Get().Seed(seed);
    }

    inline uint64_t Random::GetGlobalSeed() noexcept {
        return detail::GlobalRandomAccessor::Get().gen_();
    }

} // namespace Bamboo