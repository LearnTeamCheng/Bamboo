#include "Random.h"

#include <random>

namespace Bamboo
{

    std::random_device rd;
    std::mt19937 gen(rd());

    int Random::RandomInt(int min, int max)
    {
        std::uniform_int_distribution<int> dis(min, max);
        int result = dis(gen);
        int value = result % (max - min + 1);
        return value+min;
    }


    float Random::RandomFloat(float min, float max)
    {
        std::uniform_real_distribution<float> dis(min, max);
        float result = dis(gen);
        float value = result % (max - min);
        return value+min;
    }

}