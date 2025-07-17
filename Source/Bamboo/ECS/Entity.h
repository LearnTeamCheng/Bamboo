#pragma once
#include "../../Core/UUID.h"
#include "entt.hpp"
namespace Bamboo
{
    class Entity
    {
    public:
        template <typename T, typename... Args>
        T &AddComponent(Args &&...args);

        template <typename T>
        T &GetComponent();

        template <typename T>
        bool HasComponent() const;

        template <typename T>
        void RemoveComponent();

        bool operator==(const Entity &other) const {}

        UUID GetUUID() const { return m_uuid; }

    private:
        entt::entity m_EntityHandle{entt::null}; 
        UUID m_uuid;
    };
}