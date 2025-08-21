#pragma once

#include"../Bamboo/Core/Ref.h"
#include "Scene.h"

namespace Bamboo
{
    class SceneManager
    {
    public:
        SceneManager();
        Ref<Scene> GetActiveScene() const { return m_ActiveScene; }
        void SetActiveScene(Ref<Scene> scene) { m_ActiveScene = scene; }

        void LoadScene(const std::string& sceneName);

    private:
        Ref<Scene> m_ActiveScene;
    };
}