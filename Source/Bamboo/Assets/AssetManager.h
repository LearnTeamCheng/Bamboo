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
        AssetManager(/* args */);
        ~AssetManager();

        template<typename T>
        
        Ref<T> Load(const std::string& path) {
           return m_AssetFactory.Create<T>(path);
        }


    private:
        AssetFactory m_AssetFactory;
        std::unordered_map<std::string, Ref<Asset>> m_Assets;
    };

}