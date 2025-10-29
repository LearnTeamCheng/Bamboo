// Random.cpp
#include "Random.h"
#include <algorithm>

namespace Bamboo {

    void Random::Seed(uint64_t seed) noexcept {
        if (seed == 0) {
            std::random_device rd;
            seed = ((uint64_t)rd() << 32) | rd();
        }
        gen_.seed(seed);
        gen_.discard(100);
    }

    int Random::Int(int min, int max) noexcept {
        if (min > max) std::swap(min, max);
        if (min == max) return min;
        std::uniform_int_distribution<int> dist(min, max);
        return dist(gen_);
    }

    float Random::Float(float min, float max) noexcept {
        if (min > max) std::swap(min, max);
        if (min == max) return min;
        std::uniform_real_distribution<float> dist(min, max);
        return dist(gen_);
    }

    double Random::Double(double min, double max) noexcept {
        if (min > max) std::swap(min, max);
        if (min == max) return min;
        std::uniform_real_distribution<double> dist(min, max);
        return dist(gen_);
    }

    bool Random::Bool(float p) noexcept {
        if (p <= 0.0f) return false;
        if (p >= 1.0f) return true;
        std::bernoulli_distribution dist(p);
        return dist(gen_);
    }

    template<> int   Random::Range<int>(int min, int max) noexcept { return Int(min, max); }
    template<> float Random::Range<float>(float min, float max) noexcept { return Float(min, max); }
    template<> double Random::Range<double>(double min, double max) noexcept { return Double(min, max); }

    template<typename T>
    T Random::Choice(const std::vector<T>& items) {
        if (items.empty()) {
            assert(false && "Random::Choice: empty container");
            static T default_val{};
            return default_val;
        }
        std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
        return items[dist(gen_)];
    }

    template<typename T>
    T Random::Choice(std::initializer_list<T> items) {
        return Choice(std::vector<T>(items.begin(), items.end()));
    }

    template<typename T>
    std::optional<T> Random::WeightedChoice(const std::vector<std::pair<T, float>>& items) noexcept {
        if (items.empty()) return std::nullopt;
        float total = 0.0f;
        for (const auto& p : items) {
            if (p.second < 0.0f) return std::nullopt;
            total += p.second;
        }
        if (total <= 0.0f) return std::nullopt;
        float r = Float(0.0f, total);
        float sum = 0.0f;
        for (const auto& p : items) {
            sum += p.second;
            if (r < sum) return p.first;
        }
        return items.back().first;
    }

    Vec2<float> Random::InUnitCircle() noexcept {
        float x, y, s;
        do {
            x = Float(-1.0f, 1.0f);
            y = Float(-1.0f, 1.0f);
            s = x * x + y * y;
        } while (s >= 1.0f || s == 0.0f);
        return Vec2<float>(x, y);
    }

    Vec3<float> Random::InUnitSphere() noexcept {
        float x, y, z, s;
        do {
            x = Float(-1.0f, 1.0f);
            y = Float(-1.0f, 1.0f);
            z = Float(-1.0f, 1.0f);
            s = x * x + y * y + z * z;
        } while (s >= 1.0f || s == 0.0f);
        return Vec3<float>(x, y, z);
    }

    Vec2<float> Random::OnUnitCircle() noexcept {
        float a = Float(0.0f, 6.28318530718f);
        return Vec2<float>(std::cos(a), std::sin(a));
    }

    Vec3<float> Random::OnUnitSphere() noexcept {
        float z = Float(-1.0f, 1.0f);
        float a = Float(0.0f, 6.28318530718f);
        float r = std::sqrt(1.0f - z * z);
        return Vec3<float>(r * std::cos(a), r * std::sin(a), z);
    }

    void Random::FillFloat01(float* out, size_t count) noexcept {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        for (size_t i = 0; i < count; ++i) out[i] = dist(gen_);
    }

    void Random::FillInt(int* out, size_t count, int min, int max) noexcept {
        if (min > max) std::swap(min, max);
        std::uniform_int_distribution<int> dist(min, max);
        for (size_t i = 0; i < count; ++i) out[i] = dist(gen_);
    }

    template<typename It>
    void Random::Shuffle(It first, It last) noexcept {
        std::shuffle(first, last, gen_);
    }

    // ==================== 显式实例化 ====================
    template int   Random::Range<int>(int, int) noexcept;
    template float Random::Range<float>(float, float) noexcept;

    template int   Random::Choice<int>(const std::vector<int>&);
    template float Random::Choice<float>(const std::vector<float>&);

    template std::optional<int> Random::WeightedChoice<int>(const std::vector<std::pair<int, float>>&) noexcept;

    template void Random::Shuffle(std::vector<int>::iterator, std::vector<int>::iterator);

} // namespace Bamboo