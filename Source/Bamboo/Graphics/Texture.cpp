#include "Texture.h"
#include "../Bamboo/Graphics/RendererAPI.h"
namespace Bamboo
{
    Ref<Texture> Texture2D::Create(const std::string& path)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::OpenGL:
            {
                // TODO: create OpenGL texture
                return nullptr;
            }
            default:
            {
                return nullptr;
            }
        }
    }

    static Ref<Texture2D> Texture2D::Create(const TextureSettings& settings)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::OpenGL:
            {
                // TODO: create OpenGL texture
                return nullptr;
            }
            default:
            {
                return nullptr;
            }
        }
    }
};