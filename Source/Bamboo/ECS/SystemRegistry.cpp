#include "SystemRegistry.h"
#include "SystemContext.h"
#include "World.h"
namespace Bamboo
{

    SystemRegistry::SystemRegistry()
    {
        AddPhase(SystemPhase::Logic, 10);
        AddPhase(SystemPhase::Physics, 20);
        AddPhase(SystemPhase::Transform, 30);
        AddPhase(SystemPhase::Render, 40);
        AddPhase(SystemPhase::UI, 100);
    }

    void SystemRegistry::Update(SystemContext &context, float deltaTime)
    {
        if (m_Dirty)
        {
            std::sort(m_Phases.begin(), m_Phases.end(), [](const Phase &a, const Phase &b)
                      { return a.Order < b.Order; });
            m_Dirty = false; // 更新完成，清空标记
        }

        for (auto &phase : m_Phases)
        {
            for (auto &system : phase.m_Systems)
            {
                system->Update(context, deltaTime);
            }
        }
    }

    void SystemRegistry::AddPhase(SystemPhase phase, int order)
    {
        m_PhaseOrder[phase] = order;
    }
    
};