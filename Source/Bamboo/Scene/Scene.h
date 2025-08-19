#pragma once

#include <string>
#include <vector>
#include <unordered_map>



#include "../Bamboo/ECS/Entity.h"
#include "../Bamboo/ECS/System/ISystem.h"

#include "entt.hpp"
namespace Bamboo
{
    // 八叉树
    class Octree;

    class Scene
    {
    public:
        Scene();
        // 添加实体
        ~Scene();

        void Update(float deltaTime);

        Entity CreateEntity(const std::string& name = std::string());

        Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
        

        void DestroyEntity(Entity entity);


    private:
        entt::registry m_Registry;
        std::vector<std::unique_ptr<ISystem>> m_Systems;
        std::unordered_map<UUID, Entity*> m_EntityMap;
    };
}
