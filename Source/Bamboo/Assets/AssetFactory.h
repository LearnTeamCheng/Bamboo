#pragma once
#include <filesystem>
#include<functional>
#include <string>

#include "AssetMetadata.h"
#include "../Bamboo/core/Ref.h"
#include "../Bamboo/Assets/ImageAsset.h"

namespace Bamboo
{


    class AssetFactory
    {
    public:
        AssetFactory();

        Ref<Asset> Create(AssetType type,  const AssetMetadata& metadata);

    private:
        using AssetCreator = std::function<Ref<Asset>(const AssetMetadata& metadata)>;
        std::unordered_map<AssetType, AssetCreator> m_FactoryMap;
    };

}