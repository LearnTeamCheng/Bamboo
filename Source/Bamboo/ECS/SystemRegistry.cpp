#include "SystemRegistry.h"
namespace Bamboo
{
    void SystemRegistry::UpdateLogic(entt::registry &registry, float deltaTime)
    {
        for (auto &system : m_LogicSystems)
        {
            system.get()->Update(registry, deltaTime);
        }
    }

    void SystemRegistry::UpdatePhysics(entt::registry &registry, float deltaTime)
    {
        for (auto &system : m_PhysicsSystems)
        {
            system.get()->Update(registry, deltaTime);
        }
    }

    void SystemRegistry::UpdateRender(entt::registry &registry, float deltaTime)
    {
        for (auto &system : m_RenderSystems)
        {
            system.get()->Update(registry, deltaTime);
        }
    }

    void SystemRegistry::UpdateTransform(entt::registry &registry, float deltaTime){
        for (auto &system : m_TransformSystems)
        {
            system.get()->Update(registry, deltaTime);
        }
    }
};