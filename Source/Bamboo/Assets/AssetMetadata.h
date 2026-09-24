#pragma once
#include <filesystem>
#include "AssetHandle.h" // Include the AssetHandle class
#include "AssetType.h"
namespace Bamboo
{
    struct AssetMetadata
    {
        AssetHandle handle;
        AssetType type{AssetType::None};
        std::filesystem::path path;
    };
};