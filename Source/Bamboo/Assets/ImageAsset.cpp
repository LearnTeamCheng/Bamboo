#include "ImageAsset.h"
#include <stb_image.h>
#include "Config.h"
#include "../Bamboo/Core/Log.h"
namespace Bamboo
{
    ImageAsset::ImageAsset()
    {
        stbi_set_flip_vertically_on_load(1);
    }

    void ImageAsset::LoadFromFile(const std::string& path)
    {
        std::string fullPath = std::string(BAMBOO_ASSET_ROOT) + "/Texture2d/" + path;
        int width, height, channels;
        unsigned char *data = stbi_load(fullPath.c_str(), &width, &height, &channels, 0);

        if (data)
        {
            m_Width = width;
            m_Height = height;
            m_Channels = channels;
            
            size_t  size =static_cast<size_t> (width) * height * channels;

            m_Data.assign(data,data+ size);
            stbi_image_free(data);
        }
        else
        {
            BAMBOO_CORE_ERROR("Failed to load image: {}", fullPath);
        }
    }

    void ImageAsset::Unload()
    {
        m_Data.clear();
        m_Channels = 0;
        m_Height = 0;
        m_Width = 0;
    }
}