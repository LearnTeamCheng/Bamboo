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

    class Scene
    {
    public:
        Scene();    
        // 添加实体
        ~Scene();

        void Update(float deltaTime);

        Entity CreateEntity(const std::string& name = std::string());

        Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());

        Entity FindEntityByName(const std::string_view& name);

        void DestroyEntity(Entity entity);

        entt::registry m_Registry;

    private:
        Scope<ISystem> m_TransformSystem;
        Scope<ISystem> m_SpriteRendererSystem;
        std::vector<Scope<ISystem>> m_Systems;
        std::unordered_map<UUID, Entity> m_EntityMap;
    };
}
