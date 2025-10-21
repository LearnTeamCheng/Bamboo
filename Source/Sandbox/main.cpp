/**
 * @file main.cpp
 * @brief 测试文件
 */
#include <iostream>
#include<filesystem>


 #include "../Bamboo/Game/Application.h"
 #include "../Bamboo/Scene/Scene.h"
 #include "../Bamboo/Scene/SceneManager.h"

 #include "../Bamboo/ECS/Entity.h"
 #include "../Bamboo/ECS/Component/Component.h"

 #include "../Bamboo/Math/Color.h"
 #include "../Bamboo/Graphics/Texture.h"

 #include "../Bamboo/Graphics/Camera.h"

 #include "../Bamboo/Assets/ImageAsset.h"

// #include "BreakoutDemo/BreakoutApp.h"



int main(int argc, char** argv) {
    using Bamboo::Vector3;
    Bamboo::Application app{"test"};
    
   
    //auto& component = entity.AddComponent<Bamboo::TriangleComponent>();
    //component.TriangleColor = Bamboo::Color::Blue;
    //


    // auto& quadComponent = entity.AddComponent<Bamboo::QuadComponent>();
    // quadComponent.Color = Bamboo::Color::Blue;
 
    // std::string textList[] = {"container.jpg","awesomeface.png","arrow.png","armor.png"};
    std::string textList[] = {"arrow.png","armor.png","arrow.png","armor.png"};
    size_t length = sizeof(textList)/sizeof(textList[0]);
    Bamboo::Vector3 vectorList[] = {Bamboo::Vector3(1280 / 2, 720 / 2, 0),Bamboo::Vector3(300,300,0),Bamboo::Vector3(100,100,0),Bamboo::Vector3(500,500,0)};

    for(int i = 0;i<length;i++){

        auto entity = app.GetSceneManager()->GetActiveScene()->CreateEntity();
        auto & spriteComponent = entity.AddComponent<Bamboo::SpriteRendererComponent>();
        // auto imageAsset = app.GetAssetManager()->Load<Bamboo::ImageAsset>("container.jpg");
        auto imageAsset = app.GetAssetManager()->Load<Bamboo::ImageAsset>(textList[i]);
        spriteComponent.SpriteTexture = Bamboo::Texture2D::Create(imageAsset);
        spriteComponent.Size = Bamboo::Vector2(imageAsset->GetWidth(), imageAsset->GetHeight()); 
        spriteComponent.ZOrder = i;
        
        // auto & transform = entity.AddComponent<Bamboo::TransformComponent>();
        auto & transform = entity.GetComponent<Bamboo::TransformComponent>();
        // transform.Position = Vector3(1280/2, 720/2, 0);
        transform.Position = vectorList[i];
    }


     auto whiteEntity = app.GetSceneManager()->GetActiveScene()->CreateEntity();
     whiteEntity.AddComponent<Bamboo::SpriteRendererComponent>();
     auto & whiteTransform = whiteEntity.GetComponent<Bamboo::TransformComponent>();
     whiteTransform.Position = Bamboo::Vector3(1000,600,0);


    auto cameraentity = app.GetSceneManager()->GetActiveScene()->CreateEntity();

    //增加摄像机组件
    auto& cameraComponent = cameraentity.AddComponent<Bamboo::CameraComponent>();
    cameraComponent.CurrentCamera = Bamboo::Camera();
    cameraComponent.CurrentCamera.SetOrthographic(10, 1, 100.0f);
    cameraComponent.CurrentCamera.SetViewportSize(1280,720);

    // auto& cameraTransform =  cameraentity.AddComponent<Bamboo::TransformComponent>();
    // cameraTransform.Position = Vector3(0, 0, 0);
    
    app.Run();

    std::cout << std::filesystem::current_path() << std::endl;
    

    //BreakoutApp app;
    //app.Run();
    //return 0;
}