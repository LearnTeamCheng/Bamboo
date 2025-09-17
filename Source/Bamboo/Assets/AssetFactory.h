#pragma once
#include<functional>
#include <string>

#include "../Bamboo/core/Ref.h"
#include "../Bamboo/Assets/ImageAsset.h"

namespace Bamboo
{


    class AssetFactory
    {
    public:
        AssetFactory();

        Ref<Asset> Create(AssetType type, const std::string& path);

    private:
        using AssetCreator = std::function<Ref<Asset>(const std::string &)>;
        std::unordered_map<AssetType, AssetCreator> m_FactoryMap;
    };

}