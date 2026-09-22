#pragma once
namespace Bamboo
{
    enum class SystemPhase // 系统阶段
    {
        None,

        Logic,
        Physics,
        Transform,
        PreRender,
        Render,
        UI,
    };
};