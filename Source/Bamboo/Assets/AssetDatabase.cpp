#include "AssetDatabase.h"
#include <algorithm>
namespace Bamboo
{

    bool AssetDatabase::Contains(AssetHandle handle) const
    {
        return m_Assets.find(handle) != m_Assets.end();
    }

    AssetHandle AssetDatabase::GetHandle(const std::filesystem::path &path) const
    {
        auto it = m_PathToHandle.find(path);
        if (it != m_PathToHandle.end())
        {
            return it->second;
        }
        return AssetHandle();
    }

    const AssetMetadata *AssetDatabase::GetMetadata(AssetHandle handle) const
    {
        auto it = m_Assets.find(handle);
        if (it != m_Assets.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    bool AssetDatabase::Register(AssetMetadata metadata)
    {
        if (!metadata.handle.IsValid())
            return false;

        if (m_Assets.contains(metadata.handle))
            return false;

        if (m_PathToHandle.contains(metadata.path))
            return false;

        m_Assets.emplace(metadata.handle, metadata);
        m_PathToHandle.emplace(metadata.path, metadata.handle);

        return true;
    }

    void AssetDatabase::Remove(AssetHandle handle)
    {
        auto it = m_Assets.find(handle);
        if (it != m_Assets.end())
        {
            const auto path = it->second.path;
            m_Assets.erase(it);
            m_PathToHandle.erase(path);
        }
    }
};