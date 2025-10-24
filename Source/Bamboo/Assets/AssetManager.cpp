#include "AssetManager.h"

#include <thread>
#include <mutex>
#include <future>

namespace Bamboo
{
    AssetManager::AssetManager()
    {
    }

    AssetManager::~AssetManager()
    {
        m_Assets.clear();
    }

    void AssetManager::UnloadAll()
    {
        for (auto &pair : m_Assets)
        {
            pair.second->Unload();
        }
        m_Assets.clear();
    }

    void AssetManager::AsynLoad(const std::string &path, const std::function<void(Ref<Asset>)> &callback)
    {

    }
}