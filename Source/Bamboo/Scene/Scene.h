#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "../Bamboo/Core/Ref.h"
#include "../Bamboo/Core/UUID.h"
#include "../Bamboo/ECS/System/ISystem.h"
#include "../Bamboo/ECS/SystemRegistry.h"
#include "../ECS/World.h"
#include "entt.hpp"

namespace Bamboo
{
    // 前向声明（避免把完整定义拉进头文件）
    class Octree;
    class Entity;
    class Camera;

    class Scene
    {
    public:
        Scene();
        ~Scene();

        void Update(float deltaTime);

        Entity CreateEntity(const std::string &name = std::string());

        Entity CreateEntityWithUUID(UUID uuid, const std::string &name = std::string());

        Entity FindEntityByName(const std::string_view &name);

        Camera *GetMainCamera();
        /// @brief 添加系统 只能是逻辑系统
        template <typename T, typename... Args>
        void AddSystem(Args &&...args)
        {
            m_SystemRegistry.Register<T>(std::forward<Args>(args)...); // 逻辑系统
        }

        void DestroyEntity(Entity entity);

        SystemRegistry &GetSystemRegistry() { return m_SystemRegistry; }


    private:
        World m_World;
        std::unordered_map<UUID, Entity> m_EntityMap;
        SystemRegistry m_SystemRegistry;
    };
}
