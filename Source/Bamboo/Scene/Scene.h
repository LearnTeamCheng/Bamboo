#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "../Bamboo/Core/Ref.h"
#include "../Bamboo/Core/UUID.h"
#include "../Bamboo/ECS/System/ISystem.h"

#include "entt.hpp"

namespace Bamboo
{
    // 八叉树
    class Octree;
    class Entity;
    class Camera;

    // class PhysicsSystem;

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
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            m_LogicSystems.emplace_back(std::move(system)); 
        }

        void DestroyEntity(Entity entity);

        entt::registry m_Registry;

    private:

        std::vector<Scope<ISystem>> m_Systems;
        std::vector<Scope<ISystem>> m_LogicSystems;
        std::unordered_map<UUID, Entity> m_EntityMap;
    };
}
