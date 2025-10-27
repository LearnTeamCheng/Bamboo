#pragma once

namespace Bamboo
{
    class Random
    {
        public:
            static int RandomInt(int min, int max);
            static float RandomFloat(float min, float max);

            static void Seed(unsigned int seed);
            
    };
}