#pragma once
#include "../Bamboo/Assets/ImageAsset.h"
#include "../Bamboo/Graphics/Texture.h"

namespace Bamboo
{

    class OpenGLTexture2D : public Texture2D
    {
    public:
        OpenGLTexture2D(const Ref<ImageAsset> imageAsset);
        OpenGLTexture2D(const std::string &path);
        OpenGLTexture2D(const TextureSpecification & textureSpecification);
        virtual ~OpenGLTexture2D();

        virtual uint32_t GetWidth() const override {return m_Width;};
        virtual uint32_t GetHeight() const override {return m_Height;};
        virtual uint32_t GetRendererID() const override {return m_RendererID;};

        virtual const std::string &GetPath() const override {return m_Path;};

        virtual void Bind(uint32_t slot = 0) const override;
        virtual bool IsLoaded()const override { return m_IsLoaded; }
        virtual bool operator==(const Texture &other) const override
        {
            return m_RendererID == other.GetRendererID();
        }


        virtual void SetData(void* data, uint32_t size) const override 
        {
        
        }
    private:
        TextureSpecification m_TextureSpecification;
        bool m_IsLoaded = false;
        std::string m_Path;
        uint32_t m_RendererID;
        uint32_t m_Width, m_Height;
        int m_Channels;
    };
}