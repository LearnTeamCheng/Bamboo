#pragma once

#include <string>
#include<filesystem>
#include "AssetType.h"
namespace Bamboo
{


    class Asset
    {
    public:
        virtual ~Asset() = default;
        virtual void LoadFromFile(const std::filesystem::path& path) = 0;
        virtual void Unload() = 0;

        const std::filesystem::path &GetPath() const { return m_Path; }
        bool IsLoaded() const { return m_IsLoaded; }

        virtual AssetType GetType() = 0;

        static AssetType StaticType() {return AssetType::None;  }

    protected:
        std::filesystem::path m_Path;
        bool m_IsLoaded = false;
    };
}