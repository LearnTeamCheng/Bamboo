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



int main(int argc, char** argv) {
    using Bamboo::Vector3;
    Bamboo::Application app;
    
    auto entity = app.GetSceneManager()->GetActiveScene()->CreateEntity();
    //auto& component = entity.AddComponent<Bamboo::TriangleComponent>();
    //component.TriangleColor = Bamboo::Color::Blue;
    //


    auto& quadComponent = entity.AddComponent<Bamboo::QuadComponent>();
    quadComponent.Color = Bamboo::Color::Blue;


    auto & transform = entity.AddComponent<Bamboo::TransformComponent>();
    transform.Position = Vector3(100, 100, 0);
    
    app.Run();

    std::cout << std::filesystem::current_path() << std::endl;

    return 0;
}