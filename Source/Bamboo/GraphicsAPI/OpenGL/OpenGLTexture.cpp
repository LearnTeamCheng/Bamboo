#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include "../Bamboo//Core/Assert.h"
#include "OpenGLTexture.h"
#include <stb_image.h>
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>

#include "../Bamboo/Core/Log.h"
#include "Config.h"
namespace Bamboo
{

    namespace Utils
    {
        static GLenum ImageFormatToGLDataFormat(TextureFormat format)
        {
            switch (format)
            {
            case TextureFormat::RGB8:
                return GL_RGB;
            case TextureFormat::RGBA8:
                return GL_RGBA;
            }
            return 0;
        }

        static GLenum ImageFormatToGLInternalFormat(TextureFormat format){
            switch(format) {
                case TextureFormat::RGB8: return GL_RGB8;
                case TextureFormat::RGBA8: return GL_RGBA8;
            }
            return 0;
        }
    }

        OpenGLTexture2D::OpenGLTexture2D(const std::string &path)
        {
            // 翻转纹理y轴
            stbi_set_flip_vertically_on_load(1);
            int width, height, channels;
            stbi_uc *data = nullptr;

            std::string fullPath = std::string(BAMBOO_ASSET_ROOT) + "/Texture2d/" + path;
            data = stbi_load(fullPath.c_str(), &width, &height, &channels, 0);

            if (data)
            {
                // m_IsLoaded = true;
                m_Width = width;
                m_Height = height;
                GLenum internalFormat = 0, dataFormat = 0;
                // 设置纹理格式
                if (channels == 4)
                {
                    internalFormat = GL_RGBA8;
                    dataFormat = GL_RGBA;
                }
                else if (channels == 3)
                {
                    internalFormat = GL_RGB8;
                    dataFormat = GL_RGB;
                }

                // 生成纹理并绑定
                glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
                // 设置纹理数据
                glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

                // 设置纹理过滤方式
                glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                // 设置纹理环绕方式
                glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

                // 更新纹理数据
                glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

                stbi_image_free(data);
            }
            else
            {
                BAMBOO_CORE_ERROR("Failed to load texture {0}", fullPath);
            }
        }

        OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification &textureSpecification) : m_TextureSpecification(textureSpecification), m_Width(textureSpecification.Width), m_Height(textureSpecification.Height)
        {

            glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
            GLenum internalFormat = 0, dataFormat = 0;

            dataFormat = Utils::ImageFormatToGLDataFormat(textureSpecification.Format);
            internalFormat = Utils::ImageFormatToGLInternalFormat(textureSpecification.Format);
            m_DataFormat = dataFormat;

            //m_Channels = dataFormat;
            m_InternalFormat = internalFormat;

            // 设置纹理数据
            glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

            // 设置纹理过滤方式
            glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // 设置纹理环绕方式
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        OpenGLTexture2D::OpenGLTexture2D(const Ref<ImageAsset> imageAsset)
        {
            m_Width = imageAsset->GetWidth();
            m_Height = imageAsset->GetHeight();
            m_Channels = imageAsset->GetChannels();
            m_ImageAsset = imageAsset;
            GLenum internalFormat = 0, dataFormat = 0;
            // 设置纹理格式
            if (m_Channels == 4)
            {
                internalFormat = GL_RGBA8;
                dataFormat = GL_RGBA;
            }
            else if (m_Channels == 3)
            {
                internalFormat = GL_RGB8;
                dataFormat = GL_RGB;
            }

            m_DataFormat = dataFormat;
            // 生成纹理并绑定
            glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
            // 设置纹理数据
            glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

            // 设置纹理过滤方式
            glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // 设置纹理环绕方式
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

            // 更新纹理数据
            glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, imageAsset->GetData());
        }

        void OpenGLTexture2D::Bind(uint32_t slot) const
        {
            glBindTextureUnit(slot, m_RendererID);
        }


        void OpenGLTexture2D::SetData(void* data, uint32_t size){
            uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
            //BAMBOO_ASSESERT(size == m_Width *m_Height *bpp,"Data must be entire texture!")
            glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
        }

        OpenGLTexture2D::~OpenGLTexture2D()
        {
            glDeleteTextures(1, &m_RendererID);
        }
    };