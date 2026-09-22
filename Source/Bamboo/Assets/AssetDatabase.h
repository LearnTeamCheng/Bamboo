#pragma once

#include "../Core/UUID.h"
namespace Bamboo
{
    class AssetDatabase
    {
    public:
        UUID GetAssetID(const std::string &assetPath);
        void AddAsset(const std::string &assetPath, const UUID &assetID);
        void RemoveAsset(const std::string &assetPath);
        bool AssetExists(const std::string &assetPath) const;
    };
};
