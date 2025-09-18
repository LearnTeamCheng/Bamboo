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


int main(int argc, char** argv) {
    using Bamboo::Vector3;
    Bamboo::Application app;
    
    auto entity = app.GetSceneManager()->GetActiveScene()->CreateEntity();
    //auto& component = entity.AddComponent<Bamboo::TriangleComponent>();
    //component.TriangleColor = Bamboo::Color::Blue;
    //


    // auto& quadComponent = entity.AddComponent<Bamboo::QuadComponent>();
    // quadComponent.Color = Bamboo::Color::Blue;

    auto & spriteComponent = entity.AddComponent<Bamboo::SpriteRendererComponent>();
    //spriteComponent.SpriteTexture = Bamboo::Texture2D::Create("container.jpg");
    auto imageAsset = app.GetAssetManager()->Load<Bamboo::ImageAsset>("container.jpg");
    spriteComponent.SpriteTexture = Bamboo::Texture2D::Create(imageAsset);


    auto & transform = entity.AddComponent<Bamboo::TransformComponent>();
    transform.Position = Vector3(100, 100, 0);

    //增加摄像机组件
    auto& cameraComponent = entity.AddComponent<Bamboo::CameraComponent>();
    cameraComponent.CurrentCamera = Bamboo::Camera();
    cameraComponent.CurrentCamera.SetOrthographic(10, 0.1f, 1000.0f);
    cameraComponent.CurrentCamera.SetViewportSize(1280,720);
    
    app.Run();

    std::cout << std::filesystem::current_path() << std::endl;

    return 0;
}