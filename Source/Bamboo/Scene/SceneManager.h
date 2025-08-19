#pragma once

#include"../Bamboo/Core/Ref.h"
#include "Scene.h"

namespace Bamboo
{
    class SceneManager
    {
    public:
        Ref<Scene> GetActiveScene() const { return m_currScene; }
        void SetActiveScene(Ref<Scene> scene) { m_currScene = scene; }

        void LoadScene(const std::string& sceneName);

    private:
        Ref<Scene> m_ActiveScene;
    }
}