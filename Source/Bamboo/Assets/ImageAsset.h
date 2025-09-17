#pragma once
#include<vector>
#include "../Bamboo/Assets/Asset.h"
namespace Bamboo
{
    class ImageAsset : public Asset
    {
    public:
        ImageAsset();
        ~ImageAsset() override;
        void LoadFromFile(const std::string& filePath) override;
        void Unload() override;

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        int GetChannels() const { return m_Channels; }
        const unsigned char* GetData() const { return m_Data.data(); }


        static AssetType StaticType() { return AssetType::Image; }

        AssetType GetType() override { return StaticType(); } 

    private:
        int m_Width, m_Height, m_Channels;
        std::vector<unsigned char> m_Data;
    };
}