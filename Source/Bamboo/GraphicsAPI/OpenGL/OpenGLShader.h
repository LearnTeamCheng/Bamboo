#pragma once
#include "../Bamboo/Graphics/Shader.h"

namespace Bamboo
{

    class OpenGLShader : public Shader
    {

    public:
        OpenGLShader(const std::string &filepath);
        OpenGLShader(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);

        virtual ~OpenGLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void SetInt(const std::string &name, int value) override;
        virtual void SetFloat(const std::string &name, float value) override;
        virtual void SetVec2(const std::string &name, const Vector2 &value) override;
        virtual void SetVec3(const std::string &name, const Vector3 &value) override;

        virtual void SetMat4(const std::string &name, const Matrix4 &value) override;

        virtual const std::string &GetName() const override;
    protected:
        std::string ReadFile(const std::string &filepath);
    private:
        uint32_t m_RendererID;
        std::string m_Name;
        std::string m_FilePath;
    }
}