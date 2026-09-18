#pragma once
#include "../Bamboo/Assets/ImageAsset.h"
#include "../Bamboo/Graphics/Texture.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

        virtual void Bind(uint32_t slot = 0) const override;

        virtual bool operator==(const Texture &other) const override
        {
            return m_RendererID == other.GetRendererID();
        }


        /// @brief 用整张纹理的数据更新纹理内容（data 必须覆盖整个纹理，见 refactor_plan.md P3-2）
        virtual void SetData(void* data, uint32_t size) override;
   
    private:
        TextureSpecification m_TextureSpecification;
        uint32_t m_RendererID;
        uint32_t m_Width, m_Height;
        Ref<ImageAsset> m_ImageAsset;
        GLenum m_InternalFormat;
        GLenum m_DataFormat;
        int m_Channels;
    };
}