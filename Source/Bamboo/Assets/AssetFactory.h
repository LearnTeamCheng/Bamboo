#pragma once
#include <string>
#include "../Bamboo/core/Ref.h"

#include "../Bamboo/Assets/ImageAsset.h"

namespace Bamboo {
    class AssetFactory
    {

    public:

   
        template<typename T>
        Ref<T> Create(const std::string& path) {
            nullptr;
        }
    };



    template<>
    Ref<ImageAsset> AssetFactory::Create<ImageAsset>(const std::string& path) {
        auto image =  CreateRef<ImageAsset>();
        image->LoadFromFile(path);

        return image;
    }
    
}