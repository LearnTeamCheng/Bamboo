#pragma once

#include <string>

namespace Bamboo
{
    enum class AssetType
    {
        Image,
        Font,
        Audio,
        Model,
        Shader,
    };

    class Asset
    {
    public:
        virtual ~Asset() = default;
        virtual void LoadFromFile(const std::string &path) = 0;
        virtual void Unload() = 0;

        const std::string &GetPath() const { return m_Path; }
        bool IsLoaded() const { return m_IsLoaded; }

         virtual AssetType GetType() = 0;

    protected:
        std::string m_Path;
        bool m_IsLoaded = false;
    };
}