#pragma once

#include <string>

namespace Bamboo
{

    class Asset
    {
    public:
        virtual ~Asset() = default;
        virtual void LoadFromFile(const std::string &path) = 0;
        virtual void Unload() = 0;

        const std::string &GetPath() const { return m_Path; }
        bool IsLoaded() const { return m_IsLoaded; }

    protected:
        std::string m_Path;
        bool m_IsLoaded = false;
    };
}