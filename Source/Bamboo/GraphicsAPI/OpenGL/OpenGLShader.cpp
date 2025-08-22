#include "OpenGLShader.h"

namespace Bamboo
{

    OpenGLShader::OpenGLShader(const std::string &filepath):m_filepath(filepath)
    {
      
    }

    OpenGLShader::OpenGLShader(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc):
    m_Name(name)
    {
    }

    void OpenGLShader::Bind() const
    {
    }

    void OpenGLShader::Unbind() const {}

    void OpenGLShader::SetInt(const std::string &name, int value) {}
    void OpenGLShader::SetFloat(const std::string &name, float value) {}
    void OpenGLShader::SetVec2(const std::string &name, const Vector2 &value) {}
    void OpenGLShader::SetVec3(const std::string &name, const Vector3 &value) {}

    void OpenGLShader::SetMat4(const std::string &name, const Matrix4 &value) {}

    const std::string &OpenGLShader::GetName() const {}


    std::string OpenGLShader::ReadFile(const std::string &filepath){
        
    }
}