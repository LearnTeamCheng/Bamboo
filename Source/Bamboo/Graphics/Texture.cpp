#include "Texture.h"
#include "../Bamboo/Graphics/RendererAPI.h"
#include "../Bamboo/GraphicsAPI/OpenGL/OpenGLTexture.h"
#include "../Bamboo/Game/Application.h"
namespace Bamboo
{
    Ref<Texture2D> Texture2D::Create(const std::string& path)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::OpenGL:
            {
                // TODO: create OpenGL texture
                std::filesystem::path localPath{ path };
                return CreateRef<OpenGLTexture2D>(Application::GetInstance()->GetAssetManager()->Load<ImageAsset>(localPath));
            }
            default:
            {
                return nullptr;
            }
        }
    }

     Ref<Texture2D> Texture2D::Create(const TextureSpecification& textureSpecification)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::OpenGL:
            {
                // TODO: create OpenGL texture
                return CreateRef<OpenGLTexture2D>(textureSpecification);
            }
            default:
            {
                return nullptr;
            }
        }
    }

      Ref<Texture2D> Texture2D::Create(const Ref<ImageAsset>& imageAsset)
      {
          return  CreateRef<OpenGLTexture2D>(imageAsset);
      }
};