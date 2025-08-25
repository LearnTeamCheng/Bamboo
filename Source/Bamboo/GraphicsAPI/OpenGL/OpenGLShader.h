#pragma once
#include <unordered_map>

#include "../Bamboo/Graphics/Shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>


namespace Bamboo
{

    class OpenGLShader : public Shader
    {

    public:
        OpenGLShader(const std::string& filepath);
        OpenGLShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);

        virtual ~OpenGLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void SetInt(const std::string& name, int value) override;
        virtual void SetFloat(const std::string& name, float value) override;
        virtual void SetVec2(const std::string& name, const Vector2& value) override;
        virtual void SetVec3(const std::string& name, const Vector3& value) override;

        virtual void SetMat4(const std::string& name, const Matrix4& value) override;

        virtual const std::string& GetName() const override {return m_Name;}

    protected:
        /// @brief 处理shader代码 拆分成   顶点着色器代码  片段着色器代码
        std::unordered_map<GLenum, std::string> Preprocess(const std::string& source);
        std::string ReadFile(const std::string& filepath);
        void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);

    private:
        uint32_t m_RendererID;
        std::string m_Name;
        std::string m_FilePath;

        std::unordered_map<GLenum, std::string> m_ShaderSources;
    };
}