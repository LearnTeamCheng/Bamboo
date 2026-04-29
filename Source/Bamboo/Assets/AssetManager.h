#pragma once
#include <string>
#include <mutex>
#include <unordered_map>
#include "../Bamboo/Core/Ref.h"
#include "../Bamboo/Assets/Asset.h"

#include "../Assets/AssetFactory.h"
namespace Bamboo
{
    class AssetManager
    {

    public:
        AssetManager();
        ~AssetManager();

        template <typename T>
        Ref<T> Load(const std::string &path)
        {
            auto it = m_Assets.find(path);
            if (it != m_Assets.end())
            {
                return std::dynamic_pointer_cast<T>(it->second);
            }

            auto asset = std::dynamic_pointer_cast<T>(
                m_AssetFactory.Create(T::StaticType(), path));

            m_Assets[path] = asset;
            return asset;
        }

        template <typename T>
        void Unload(const std::string &path)
        {
            auto it = m_Assets.find(path);
            if (it != m_Assets.end())
            {
                m_Assets.erase(it);
            }
        }

        void UnloadAll();

        template <typename T>
        void AsyncLoad(const std::string &path, const std::function<void(Ref<Asset>)> &callback)
        {
            {

                std::lock_guard<std::mutex> lock(m_Mutex);

                auto it = m_Assets.find(path);
                if (it != m_Assets.end())
                {
                    if (callback)
                    {
                        callback(it->second);
                    }
                    return;
                }
            }

            std::thread th{[this, path, callback]()
                           {
                               auto asset = std::dynamic_pointer_cast<T>(
                                   m_AssetFactory.Create(T::StaticType(), path));

                               {
                                   std::lock_guard<std::mutex> lock(m_Mutex);
                                   if (m_Assets.find(asset) != m_Assets.end())
                                   {
                                       m_Assets[path] = asset;
                                   }
                               }

                               if (callback)
                               {
                                   callback(asset);
                               }
                           },
                           path};

            th.detach();
        }

    private:
        AssetFactory m_AssetFactory;
        std::unordered_map<std::string, Ref<Asset>> m_Assets;
        std::mutex m_Mutex;
    };

}