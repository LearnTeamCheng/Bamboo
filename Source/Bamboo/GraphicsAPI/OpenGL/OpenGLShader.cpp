#include <sstream>
#include <fstream>
#include <iostream>
#include "../Bamboo/Core/Log.h"
#include "Config.h"
#include "OpenGLShader.h"

namespace Bamboo
{

    OpenGLShader::OpenGLShader(const std::string &filepath) : m_FilePath(filepath)
    {
        auto source = ReadFile(filepath);
        auto sources = Preprocess(source);
        Compile(sources);
    }

    OpenGLShader::OpenGLShader(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath) : m_Name(name)
    {
        std::unordered_map<GLenum, std::string> sources;
        sources[GL_VERTEX_SHADER] = ReadFile(vertexPath);
        sources[GL_FRAGMENT_SHADER] = ReadFile(fragmentPath);

        Compile(sources);
    }

    OpenGLShader::~OpenGLShader()
    {
        glDeleteProgram(m_RendererID);
    }

    void OpenGLShader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void OpenGLShader::Unbind() const
    {
        glUseProgram(0);
    }

    void OpenGLShader::SetInt(const std::string &name, int value)
    {
    
    }

    void OpenGLShader::SetFloat(const std::string &name, float value) {}
    void OpenGLShader::SetVec2(const std::string &name, const Vector2 &value) {}
    void OpenGLShader::SetVec3(const std::string &name, const Vector3 &value) {}

    void OpenGLShader::SetMat4(const std::string &name, const Matrix4 &value) {}

 

    void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string> &shaderSources)
    {
        m_RendererID = glCreateProgram();
        for (auto &kv : shaderSources)
        {
            GLenum type = kv.first;
            const std::string &source = kv.second;

            unsigned int shader = glCreateShader(type);
            const char* shaderSource = source.c_str();
            glShaderSource(shader, 1, &shaderSource, 0);
            glCompileShader(shader);

            int success;
            char infoLog[512];
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 512, NULL, infoLog);
                BAMBOO_CORE_ERROR("Shader compilation failed: {0}", infoLog);
                glDeleteShader(shader);
                return;
            }
            glAttachShader(m_RendererID, shader);
        }
        glLinkProgram(m_RendererID);

        int success;
        char infoLog[512];
        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(m_RendererID, 512, NULL, infoLog);
            BAMBOO_CORE_ERROR("Shader linking failed: {0}", infoLog);
            return;
        }

        for (auto &kv : shaderSources)
        {
            glDeleteShader(kv.first);
        }
    }
    
    std::string OpenGLShader::ReadFile(const std::string &filePath)
    {
        //获取到文件名
        std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
        m_Name = fileName.substr(0, fileName.find_last_of("."));
        std::string fullPath = std::string(BAMBOO_ASSET_ROOT)+"/"+"Shaders/"  + fileName;
      
        std::string result;
        std::ifstream in(fullPath, std::ios::in | std::ios::binary);
        if (in)
        {
            in.seekg(0, std::ios::end);
            std::streamsize size = in.tellg();
            if (size != -1)
            {
                in.seekg(0, std::ios::beg);
                result.resize(size);
                in.read(&result[0], size);
            }
            else
            {
                BAMBOO_CORE_ERROR("Could not read file: {0}", fullPath);
            }
        }
        else
        {
            BAMBOO_CORE_ERROR("Could not open file: {0}", fullPath);
        }

        return result;
    }

    std::unordered_map<GLenum, std::string> OpenGLShader::Preprocess(const std::string &source) 
    {
        std::unordered_map<GLenum, std::string> sources;
        const char *typeToken = "#type";
        size_t pos = source.find(typeToken, 0);
        GLenum shaderType = 0;
        while (pos != std::string::npos)
        {   
            //找到第一个换行符
            size_t eol = source.find_first_of("\r\n", pos);
            //获取到shader类型
            std::string type = source.substr(pos + strlen(typeToken) +1, eol - pos - strlen(typeToken));

            if(type == "vertex") {
                shaderType = GL_VERTEX_SHADER;
            }else if(type == "fragment") {
                shaderType = GL_FRAGMENT_SHADER;
            }

            std::string::size_type nextLinePos = source.find_first_not_of("\r\n", eol);
            pos = source.find(typeToken, nextLinePos);

            sources[shaderType] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
        
        }

        return sources;
        
    }
}