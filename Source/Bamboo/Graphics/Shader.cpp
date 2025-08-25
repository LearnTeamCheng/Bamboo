#include "../Bamboo/Graphics/RendererAPI.h"
#include "Shader.h"
#include "../Bamboo/GraphicsAPI/OpenGL/OpenGLShader.h"

namespace Bamboo
{

    /// @brief 根据文件路径创建Shader
    /// @param filepath 文件路径
     Ref<Shader> Shader::Create(const std::string &filepath)
    {

        return CreateRef<OpenGLShader>(filepath);

    }
    /// @brief 根据文件路径创建Shader
     Ref<Shader> Shader::Create(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath)
    {
         return  CreateRef<OpenGLShader>(name, vertexPath, fragmentPath);
    }

    void ShaderLibrary::Add(const std::string &name, const Ref<Shader> &shader)
    {
        m_Shaders[name] = shader;
    }

    void ShaderLibrary::Add(const Ref<Shader> &shader)
    {
        m_Shaders[shader->GetName()] = shader;
    }

    Ref<Shader> ShaderLibrary::Load(const std::string &filepath)
    {
        Ref<Shader> shader = Shader::Create(filepath);
        return shader;
    }

    Ref<Shader> ShaderLibrary::Load(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath)
    {
        Ref<Shader> shader = Shader::Create(name, vertexPath, fragmentPath);
        Add(name, shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::Get(const std::string &name)
    {
        return m_Shaders[name];
    }

    bool ShaderLibrary::Exists(const std::string &name) const
    {
        return m_Shaders.find(name) != m_Shaders.end();
    }

}