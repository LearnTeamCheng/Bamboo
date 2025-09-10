#pragma once
#include "OpenGLTexture.h"
#include <stb_image.h>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include "../Bamboo/Core/Log.h"
namespace Bamboo {

    OpenGLTexture2D::OpenGLTexture2D(const std::string& path):m_Path(path) 
    {
        //翻转纹理y轴
        stbi_set_flip_vertically_on_load(1);
        int width, height, channels;
        stbi_uc* data = nullptr;
        data = stbi_load(path.c_str(), &width, &height, &channels, 0);

        if(data)
        {
            m_IsLoaded = true;
            m_Width = width;
            m_Height = height;


            stbi_image_free(data);
        }else {
            BAMBOO_CORE_ERROR("Failed to load texture {0}", path);
        }
    }


    OpenGLTexture2D::OpenGLTexture2D(const TextureSettings& settings):m_Settings(settings),m_Width(settings.Height),m_Height(settings.Width)
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