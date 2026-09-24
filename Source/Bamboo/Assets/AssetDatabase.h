#pragma once
#include <filesystem>
#include <unordered_map> // unordered_map
#include <string>        // string
#include "../Core/UUID.h"
#include "AssetHandle.h"
#include "AssetMetadata.h"
namespace Bamboo
{
    class AssetDatabase
    {
    public:
        bool Contains(AssetHandle handle) const;
        AssetHandle GetHandle(const std::filesystem::path &path) const;
        const AssetMetadata *GetMetadata(AssetHandle handle) const;
        bool Register(AssetMetadata metadata);
        void Remove(AssetHandle handle);

    private:
        std::unordered_map<AssetHandle, AssetMetadata> m_Assets;
        std::unordered_map<std::filesystem::path,AssetHandle>m_PathToHandle;
    };
};
