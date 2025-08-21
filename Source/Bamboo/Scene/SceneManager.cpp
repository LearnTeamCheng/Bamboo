#include "SceneManager.h"


namespace Bamboo 
{
    void SceneManager::LoadScene(const std::string& sceneName)
    {
        //目前测试就直接创建一个场景
        m_ActiveScene = CreateRef<Scene>();
    }

    SceneManager::SceneManager() 
    {
        LoadScene("测试场景");
    }
}