#pragma once
#include "entt.hpp"
namespace Bamboo
{
    class Entity;

    class World
    {
    public:
        Entity CreateEntity();
        void DestroyEntity(Entity entity);

        entt::registry &GetRegistry() { return m_Registry; }

    private:
        entt::registry m_Registry;
    };
};