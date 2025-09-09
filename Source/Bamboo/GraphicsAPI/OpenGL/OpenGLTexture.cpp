#pragma once
#include "OpenGLTexture.h"

#include <stb_image.h>
namespace Bamboo {

    OpenGLTexture2D::OpenGLTexture2D(const std::string& path):m_Path(path) {

    }

    OpenGLTexture2D::OpenGLTexture2D(const TextureSettings& settings):m_Settings(settings),m_Width(settings.Height),m_Height(settings.Width)
    {
   
    }


    void OpenGLTexture2D::Bind(uint32_t slot) const
    {


    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {


    }

        
};