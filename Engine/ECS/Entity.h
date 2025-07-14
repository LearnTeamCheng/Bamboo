#pragma once
#include "../../Core/UUID.h"
namespace Bamboo
{
    class Entity
    {
    public:
        template<typename T, typename... Args>
        T& AddComponent(Args&&... args);

        template<typename T>
        T& GetComponent();

        template<typename T>
        bool HasComponent() const;

        template<typename T>
        void RemoveComponent();


        bool operator ==(const Entity& other) const { }
        
        UUID GetUUID() const { return m_uuid; }

    private:
        UUID m_uuid;
    };
}