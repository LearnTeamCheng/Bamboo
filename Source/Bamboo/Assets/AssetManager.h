#pragma once
#include <string>
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

        void AsynLoad(const std::string &path, const std::function<void(Ref<Asset>)> &callback);

    private:
        AssetFactory m_AssetFactory;
        std::unordered_map<std::string, Ref<Asset>> m_Assets;
    };

}