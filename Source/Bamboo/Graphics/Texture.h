#pragma once
#include "../Bamboo/Core/Ref.h"
#include <cstdint> 
#include <string>

#include "../Bamboo/Assets/ImageAsset.h"
namespace Bamboo
{

    enum class TextureFormat {
        NONE = 0,
        R8,
        RGB8,
        RGBA8,
        RGBA32F,
    };

    /// @brief 纹理设置
    struct TextureSpecification
    {
        TextureFormat Format = TextureFormat::RGBA8;
        uint32_t Width = 0;
        uint32_t Height = 0;
        /// @brief 是否需要mipmap
        bool Mipmap = false;
    };
    

    class Texture
    {
    public:
   
        virtual ~Texture() = default;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetRendererID() const = 0;

        /// @brief 获取路径
        // virtual const std::string &GetPath()const = 0;
        virtual void SetData(void* data, uint32_t size) const = 0;

        /// @brief 绑定纹理到指定槽位
        virtual void Bind(uint32_t slot = 0)const = 0;

        // virtual bool IsLoaded() const = 0;

        virtual bool operator==(const Texture& other) const = 0;
    };


    /// @brief 2D纹理
    class Texture2D :public Texture
    {
    public:
        static Ref<Texture2D> Create(const std::string& path);
        static Ref<Texture2D> Create(const TextureSpecification& textureSpecification);
        static Ref<Texture2D> Create(const Ref<ImageAsset>& imageAsset);
    };

}

