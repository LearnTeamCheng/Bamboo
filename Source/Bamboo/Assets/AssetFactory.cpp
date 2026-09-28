#include "../Bamboo/Core/Ref.h"

#include "AssetFactory.h"
#include "ImageAsset.h"
#include "../Bamboo/Core/Log.h"

namespace Bamboo
{

    AssetFactory::AssetFactory()
    {

        m_FactoryMap[AssetType::Image] = [](const AssetMetadata& metadata ) -> Ref<Asset>
            {
                auto image = CreateRef<ImageAsset>();
                image->LoadFromFile(metadata.path);
                return image;
            };
    }

   Ref<Asset> AssetFactory::Create(AssetType type, const AssetMetadata& metadata)
    {
        auto it = m_FactoryMap.find(type);
        if (it != m_FactoryMap.end())
        {
            return it->second(metadata);
        }
        BAMBOO_CORE_WARN("AssetFactory::CreateAsset failed, type:{}, path:{}", static_cast<int>(type),metadata.path.string());
        return nullptr;
    }
}