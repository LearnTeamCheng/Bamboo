#pragma once
#include <algorithm>
#include <vector>
#include <utility>
#include <unordered_map>
#include "../Bamboo/Core/Ref.h"
#include "SystemPhase.h"
#include "../Bamboo/ECS/System/ISystem.h"

namespace Bamboo
{
    class SystemRegistry
    {
    public:
        SystemRegistry();
        template <typename T, typename... Args>
        T& Register(Args &&...args);

        void Update(entt::registry &registry, float deltaTime);

    private:
        void AddPhase(SystemPhase phase, int order);
        int GetPhaseOrder(SystemPhase phase) { return m_PhaseOrder[phase]; }
        struct Phase
        {
            SystemPhase Type{SystemPhase::None};
            int Order{0};
            std::vector<Scope<ISystem>> m_Systems;
        };
        std::vector<Phase> m_Phases;
        std::unordered_map<SystemPhase, int> m_PhaseOrder; // 系统阶段顺序

        bool m_Dirty = false; // 是否需要更新
    };

    template <typename T, typename... Args>
    T& SystemRegistry::Register(Args &&...args)
    {
        auto system = CreateScope<T>(std::forward<Args>(args)...);

        SystemPhase phase = system->GetPhase();
        auto it = std::find_if(m_Phases.begin(), m_Phases.end(), [&](const auto &p)
                               { return p.Type == phase; });
        T &resule = *system;
        if (it == m_Phases.end())
        {
            m_Phases.push_back({phase, GetPhaseOrder(phase)});
            m_Phases.back().m_Systems.push_back(std::move(system));
        }
        else
        {
            it->m_Systems.push_back(std::move(system));
        }

        m_Dirty = true;

        return resule;
    }
};