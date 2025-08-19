#pragma once
#include <string>

#include "../Bamboo/Core/Ref.h"

#include "Scene.h"

namespace Bamboo
{
    class SceneSerializer
    {
        public:
            SceneSerializer(Ref<Scene>& scene);

            /// @brief 序列化场景
            /// @param filename 
            void Serialize(const std::string& filename);
            
            /// @brief 反序列化场景
            /// @param filename 
            void Deserialize(const std::string& filename);
    };
}