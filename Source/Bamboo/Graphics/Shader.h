#pragma once
#include <string>

#include "../Bamboo/Core/Ref.h"
#include "../Bamboo/Math/Vector2.h"
#include "../Bamboo/Math/Vector3.h"
#include "../Bamboo/Math/Vector4.h"

#include "../Bamboo/Math/Matrix4.h"
namespace Bamboo
{
    class Shader
    {
    public:
        virtual ~Shader() = default;
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void SetInt(const std::string &name, int value) = 0;
        virtual void SetFloat(const std::string &name, float value) = 0;
        virtual void SetVec2(const std::string &name, const Vector2 &value) = 0;
        virtual void SetVec3(const std::string &name, const Vector3 &value) = 0;

        virtual void SetMat4(const std::string &name, const Matrix4 &value) = 0;

        virtual const std::string &GetName() const = 0;

        /// @brief 根据文件路径创建Shader
        /// @param filepath 文件路径
        static Ref<Shader> Create(const std::string &filepath);
        /// @brief 根据文件路径创建Shader
        /// @param name 名称
        /// @param vertexSrc 顶点着色器源码
        /// @param fragmentSrc 片段着色器源码
        static Ref<Shader> Create(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);

    };
}