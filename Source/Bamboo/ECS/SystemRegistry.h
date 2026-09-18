#pragma once
#include <vector>
#include <utility>
#include "../Bamboo/Core/Ref.h"
#include "../Bamboo/ECS/System/ISystem.h"

namespace Bamboo
{
    class SystemRegistry
    {
    public:
        template <typename T, typename... Args>
        void Register(Args &&...args);

        void UpdateLogic(entt::registry &registry, float deltaTime);
        void UpdatePhysics(entt::registry &registry, float deltaTime);
        void UpdateRender(entt::registry &registry, float deltaTime);
        void UpdateTransform(entt::registry &registry, float deltaTime);

    private:
        std::vector<Scope<ISystem>> m_LogicSystems;
        std::vector<Scope<ISystem>> m_PhysicsSystems;
        std::vector<Scope<ISystem>> m_RenderSystems;
        std::vector<Scope<ISystem>> m_TransformSystems; // Add scene systems here if needed
    };

    template <typename T, typename... Args>
    void SystemRegistry::Register(Args &&...args)
    {
        auto system = CreateScope<T>(std::forward<Args>(args)...);
        system->Init();

        switch (system->GetPhase())
        {
        case SystemPhase::Logic:
            m_LogicSystems.push_back(std::move(system));
            break;
        case SystemPhase::Physics:
            m_PhysicsSystems.push_back(std::move(system));
            break;
        case SystemPhase::Render:
            m_RenderSystems.push_back(std::move(system));
            break;
        case SystemPhase::Transform:
            m_TransformSystems.push_back(std::move(system));
            break; 
        default:
            break;
        }
    }
};