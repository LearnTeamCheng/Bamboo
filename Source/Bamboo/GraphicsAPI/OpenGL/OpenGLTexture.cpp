#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "OpenGLTexture.h"
#include <stb_image.h>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include "../Bamboo/Core/Log.h"
#include "Config.h"
namespace Bamboo {

    OpenGLTexture2D::OpenGLTexture2D(const std::string& path):m_Path(path) 
    {
        //翻转纹理y轴
        stbi_set_flip_vertically_on_load(1);
        int width, height, channels;
        stbi_uc* data = nullptr;

    
        std::string fullPath = std::string(BAMBOO_ASSET_ROOT) +"/"+path;
        data = stbi_load(fullPath.c_str(), &width, &height, &channels, 0);

        if(data)
        {
            m_IsLoaded = true;
            m_Width = width;
            m_Height = height;
            GLenum internalFormat = 0, dataFormat = 0;
            //设置纹理格式
            if(channels == 4)
            {
                internalFormat = GL_RGBA8;
                dataFormat = GL_RGBA;
            }
            else if(channels == 3)
            {
                internalFormat = GL_RGB8;
                dataFormat = GL_RGB;
            }
            
            //生成纹理并绑定
            glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
            //设置纹理数据
            glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);


            //设置纹理过滤方式
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            //设置纹理环绕方式
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            //更新纹理数据
            glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        }else {
            BAMBOO_CORE_ERROR("Failed to load texture {0}", fullPath);
        }
    }


    OpenGLTexture2D::OpenGLTexture2D(const TextureSettings& settings):m_Settings(settings)
    ,m_Width(settings.Height),m_Height(settings.Width)
    {
   
    }


    void OpenGLTexture2D::Bind(uint32_t slot) const
    {
        glBindTextureUnit(slot, m_RendererID);
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1, &m_RendererID);
    }

        
};