#pragma once
#include <filesystem>
#include <string>
#include <mutex>

#include <unordered_map>
#include "../Bamboo/Core/Ref.h"
#include "../Bamboo/Core/Log.h"
#include "Assets/Asset.h"
#include "AssetFactory.h"
#include "AssetHandle.h"
#include "AssetDatabase.h"
namespace Bamboo
{
    class AssetManager
    {

    public:
        AssetManager();
        ~AssetManager();

        template <typename T>
        Ref<T> Load(std::filesystem::path &path)
        {
            AssetHandle handle = m_Database.GetHandle(path);
            if (!handle.IsValid())
            {
                handle = AssetHandle(UUID::Generate());

                AssetMetadata metadata {
                    .handle = handle,
                    .type = T::StaticType(),
                    .path = path,
                };
                m_Database.Register(metadata);
            }

            return Load<T>(handle);
        }

        template <typename T>
        Ref<T> Load(AssetHandle &handle)
        {
            auto metadata =  m_Database.GetMetadata(handle);

            if(!metadata){
                BAMBOO_CORE_ERROR("Asset not found: {}",handle.GetUUID().Value());
                return nullptr;
            }

            auto it = m_Assets.find(handle);    
            if (it != m_Assets.end())
            {
                return std::dynamic_pointer_cast<T>(it->second);
            }

            auto asset = std::dynamic_pointer_cast<T>(
                m_AssetFactory.Create(metadata->type, *metadata));

            if(asset == nullptr){
                return nullptr;  
            }

            m_Assets.emplace(handle, asset);  

            return asset;
        }

        template <typename T>
        void Unload(AssetHandle &handle)
        {
            m_Assets.erase(handle);  
        }

        void UnloadAll() ;

        template <typename T>
        void AsyncLoad(const std::string &path, const std::function<void(Ref<Asset>)> &callback)
        {
        }

    private:
        AssetFactory m_AssetFactory;
        std::unordered_map<AssetHandle, Ref<Asset>> m_Assets;

        AssetDatabase m_Database;
    };

}